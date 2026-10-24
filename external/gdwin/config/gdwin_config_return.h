// @FILE [tag: return] [summary: Defines objects used as return values in gdwin framework] [name: gdwin_config_return.h] [type: header]

#pragma once

/**
 * @file gdwin_window.h
 * @brief Window class for portable window handling on Windows.
 *
 *    lresult__               Result object
 *    event_result_traits__   Wrapper for mapping portable EVENT_RESULT to native Win32 LRESULT
 */


#include <expected>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <cstdint>
#include <limits>


#ifndef _GDWIN_BEGIN
#  define _GDWIN_BEGIN namespace gdwin {
#  define _GDWIN_END }
#endif


_GDWIN_BEGIN

/** @API [tag: result] [summary: The lresult struct represents the result of an event handling operation.] [jump: lresult__]
 * @brief The lresult struct represents the result of an event handling operation in a windowing framework. It encapsulates the outcome of an event processing function, allowing for clear communication of whether the event was handled, not handled, or resulted in an error.
 */
struct lresult
{
   /// Three outcomes an OnEvent override can produce.
   enum class enumState
   {
      eHandled,     ///< we processed it, value is the LRESULT to return to Windows
      eNotHandled, ///< we decline it, framework should call DefWindowProc
      eError,       ///< we failed, framework may call OnGetError for details
   };

   // ---------------------------------------------------------------------
   // Constructors / factories
   // ---------------------------------------------------------------------

// @API [tag: construction] [summary: constructors and factories] [jump: construct__]

   lresult() = default;
   lresult(int64_t iValue) : m_eState(enumState::eHandled), m_iValue(iValue) {}
   lresult(enumState eState, int64_t iValue = 0) : m_eState(eState), m_iValue(iValue) {}

   lresult& operator=(int64_t iValue) { m_eState = enumState::eHandled; m_iValue = iValue; return *this; }

   // @API [tag: getter, setter] [summary: Getter and setter functions for lresult] [jump: getter_setter__]

      /// "I handled this, and the LRESULT to return is `value`."
   lresult handled(int64_t iValue = 0) noexcept { return lresult{ enumState::eHandled, iValue }; }

   /// "I didn't handle this — run DefWindowProc."
   lresult not_handled() const noexcept { return lresult{ enumState::eNotHandled, 0 }; }

   /// "I failed — the framework may call OnGetError for details."
   lresult error() const noexcept { return lresult{ enumState::eError, 0 }; }

   [[nodiscard]] constexpr bool is_handled()     const noexcept { return m_eState == enumState::eHandled; }
   [[nodiscard]] constexpr bool is_not_handled() const noexcept { return m_eState == enumState::eNotHandled; }
   [[nodiscard]] constexpr bool is_error()       const noexcept { return m_eState == enumState::eError; }

   /// The LRESULT to return to Windows. Only meaningful when is_handled().
   [[nodiscard]] constexpr int64_t value() const noexcept { return m_iValue; }

   // @API [tag: conversion] [summary: Conversion to/from the raw int64_t OnEvent return] [jump: conversion__]

      /// Map this result to the raw int64_t the framework expects.
   [[nodiscard]] constexpr int64_t as_int64() const noexcept;

   /// Decode a raw int64_t returned by OnEvent into an lresult.
   [[nodiscard]] constexpr lresult from_int64(int64_t iRaw) noexcept;

   // @API [tag: attributes] [summary: Attributes of the lresult struct] [jump: attributes__]
public:
   enumState m_eState = enumState::eNotHandled; ///< default to not handled, so uninitialized lresult is safe
   int64_t m_iValue = 0; ///< meaningful only when m_state == enumState::eHandled

   static constexpr int64_t m_iNotHandled_s = INT64_MIN; ///< sentinel value for not handled
   static constexpr int64_t m_iError_s = INT64_MIN + 1; ///< sentinel value for error

};

/// @brief Map this result to the raw int64_t the framework expects -----------
constexpr int64_t lresult::as_int64() const noexcept
{
   switch(m_eState)
   {
   case enumState::eHandled:     return m_iValue;
   case enumState::eNotHandled:  return m_iNotHandled_s;
   case enumState::eError:       return m_iError_s;
   }
   return m_iNotHandled_s; // unreachable, silences warnings
}


/// @brief Decode a raw int64_t returned by OnEvent into an lresult.-----------
constexpr lresult lresult::from_int64(int64_t iRaw) noexcept
{
   if(iRaw == m_iNotHandled_s) return not_handled();
   if(iRaw == m_iError_s)      return error();
   return handled(iRaw);
}


// ----------------------------------------------------------------------------
/// @API [tag: return] [summary: The event_result_traits struct provides a mapping between portable EVENT_RESULT types and native Win32 LRESULT values.] [jump: event_result_traits__]
/// Trait that maps a portable EVENT_RESULT type to/from the native Win32 LRESULT.
/// Specialize this for custom result wrappers.
template<typename EVENT_RESULT>
struct event_result_traits
{
   /// @brief Check if the event result indicates that the event was handled.
   static bool          is_handled(const EVENT_RESULT& result_) { return result_.is_handled(); }
   /// @brief Convert a native Win32 LRESULT to the portable EVENT_RESULT type.
   static EVENT_RESULT  from_native(LRESULT lresult_) { return EVENT_RESULT{ static_cast<int64_t>(lresult_) }; }
   /// @brief Convert a portable EVENT_RESULT type to the native Win32 LRESULT.
   static LRESULT       to_native(const EVENT_RESULT& result_) { return static_cast<LRESULT>(result_.value()); }
};

/// Specialization for lresult, which is the default EVENT_RESULT type.
template<>
struct event_result_traits<lresult>
{
   static bool          is_handled(const lresult& result_) { return result_.is_handled(); }
   static lresult       from_native(LRESULT lresult_) { return lresult(static_cast<int64_t>(lresult_)); }
   static LRESULT       to_native(const lresult& result_) { return static_cast<LRESULT>(result_.value()); }
};

_GDWIN_END
