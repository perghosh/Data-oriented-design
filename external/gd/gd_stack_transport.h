// @FILE [tag: stack, transport] [description: template stack with optional borrowed storage, grows to the heap only when the borrowed buffer is full] [name: gd_stack_transport.h] [type: header] 

#pragma once

#include <cassert>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

#include "gd_vector_transport.h"

/*
 * # stack_transport
 *
 * Jump board
 *
 * stack_transport   Stack that works in a borrowed buffer and only allocates when the buffer is full
 * stack_local       Stack that owns a local (inline) buffer of uCapacity elements, no heap until it overflows
 */

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

/** @CLASS [tag: stack, transport] [summary: stack used for transport and minimal memory allocation]
 * @brief LIFO stack that can use caller-provided storage.
 *
 * Replacement for std::stack where the normal case is a few elements. std::stack uses
 * std::deque by default and that allocates on the heap, also for one element. stack_transport
 * is built on vector_transport so it has the same 16 byte footprint and the same storage rules.
 *
 * @par Borrowed storage
 * Construct from a writable T pointer and capacity or a std::span<T>. The buffer must have room
 * for between 1 and 65535 elements and be aligned for T. The buffer is raw storage, elements are
 * created in it with placement new and destroyed when popped, the buffer itself is never freed.
 * The caller owns the buffer and it must stay valid and at the same address while the stack uses it.
 *
 * When the stack grows past the borrowed capacity the elements are moved to heap storage and the
 * stack stays owned from then on. A default constructed stack has no buffer and allocates on first push.
 *
 * @par Ownership
 * Copying creates an independent owned copy. Moving transfers the storage; moving a borrowed
 * stack does not extend the lifetime of the external buffer.
 * Any push that allocates invalidates references returned from top().
 *
 * @par Example: local buffer
 * @code{.cpp}
 * gd::stack_local<token, 32> stackOperator;   // 32 tokens on the stack frame, no heap
 * stackOperator.push(token_);
 * auto& tokenTop = stackOperator.top();
 * stackOperator.pop();
 * @endcode
 */
template<typename TYPE>
class stack_transport
{
public:
   using value_type = TYPE;
   using size_type = std::size_t;
   using reference = TYPE&;
   using const_reference = const TYPE&;
   using container_type = vector_transport<TYPE>;

public:
   stack_transport() = default;

   // Borrow an external writable buffer (no allocation until it is full).
   stack_transport(TYPE* pBuffer, size_type uBufferCapacity) : m_vectorValue(pBuffer, uBufferCapacity) {}
   explicit stack_transport(std::span<TYPE> spanBuffer) : m_vectorValue(spanBuffer) {}
   explicit stack_transport(std::span<std::byte> spanBuffer) requires (std::is_same_v<TYPE, std::byte> == false) : m_vectorValue(spanBuffer) {}

   stack_transport(const stack_transport&) = default;
   stack_transport(stack_transport&&) noexcept = default;
   stack_transport& operator=(const stack_transport&) = default;
   stack_transport& operator=(stack_transport&&) noexcept = default;
   ~stack_transport() = default;

   // ## access -------------------------------------------------------------------
   reference top() noexcept { assert(empty() == false); return m_vectorValue.back(); }
   const_reference top() const noexcept { assert(empty() == false); return m_vectorValue.back(); }

   /// element counted from the bottom of the stack, 0 is the oldest element
   reference operator[](size_type uIndex) noexcept { return m_vectorValue[uIndex]; }
   const_reference operator[](size_type uIndex) const noexcept { return m_vectorValue[uIndex]; }

   // ## capacity -----------------------------------------------------------------
   bool empty() const noexcept { return m_vectorValue.empty(); }
   size_type size() const noexcept { return m_vectorValue.size(); }
   size_type capacity() const noexcept { return m_vectorValue.capacity(); }
   bool is_borrowed() const noexcept { return m_vectorValue.is_borrowed(); }
   bool is_owned() const noexcept { return m_vectorValue.is_owned(); }
   void reserve(size_type uCapacity) { m_vectorValue.reserve(uCapacity); }

   // ## modify -------------------------------------------------------------------
   void push(const TYPE& valueElement) { m_vectorValue.push_back(valueElement); }
   void push(TYPE&& valueElement) { m_vectorValue.push_back(std::move(valueElement)); }
   template<typename... Args>
   reference emplace(Args&&... arguments) { return m_vectorValue.emplace_back(std::forward<Args>(arguments)...); }
   void pop() noexcept { assert(empty() == false); m_vectorValue.pop_back(); }
   void clear() noexcept { m_vectorValue.clear(); }               ///< destroys the elements, the capacity (borrowed or owned) is kept
   void swap(stack_transport& o) noexcept { m_vectorValue.swap(o.m_vectorValue); }

   friend void swap(stack_transport& stackLeft, stack_transport& stackRight) noexcept { stackLeft.swap(stackRight); }

   // ## attributes ---------------------------------------------------------------
   container_type m_vectorValue;      ///< storage, borrowed buffer or owned heap memory
};

static_assert(sizeof(stack_transport<char>) == 16, "stack_transport must be sizeof 16 bytes");
static_assert(sizeof(stack_transport<double>) == 16, "stack_transport must be sizeof 16 bytes");


/// Raw storage for stack_local, a separate base so it exists before stack_transport is constructed.
template<typename TYPE, std::size_t uCapacity>
struct stack_storage
{
   alignas(TYPE) std::byte m_byteBuffer[sizeof(TYPE) * uCapacity];
};

/** @CLASS [tag: stack, transport] [summary: stack_transport with its own inline buffer]
 * @brief stack_transport that carries its buffer inside the object.
 *
 * Use it as a local variable: the buffer lives on the stack frame, nothing is allocated until more
 * than uCapacity elements are pushed. The buffer is raw bytes so TYPE does not need a default
 * constructor and no element is constructed until it is pushed.
 *
 * The object borrows its own address, so it can not be copied or moved. Pass it by reference
 * or as stack_transport<TYPE>&.
 */
template<typename TYPE, std::size_t uCapacity>
class stack_local : private stack_storage<TYPE, uCapacity>, public stack_transport<TYPE>
{
   static_assert(uCapacity > 0 && uCapacity <= 0xFFFFu, "borrowed capacity must be 1..65535");
public:
   stack_local() : stack_transport<TYPE>(reinterpret_cast<TYPE*>(this->m_byteBuffer), uCapacity) {}
   stack_local(const stack_local&) = delete;
   stack_local& operator=(const stack_local&) = delete;
   ~stack_local() = default;
};

_GD_END
