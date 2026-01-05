#pragma once
#include <cassert>
#include <string>
#include <string_view>

#include "gd_uuid.h"

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

namespace com {
   struct guid { uint32_t u1; uint16_t u2; uint16_t u3; uint8_t  pu4[8]; };

   constexpr int32_t S_Ok                 = 0x00000000;

   constexpr int32_t E_Fail               = 0x80004005;

   constexpr int32_t E_Pointer            = 0x80004003;
   constexpr int32_t E_InvalidArgument    = 0x80070057;
   constexpr int32_t E_Abort              = 0x80004004;
   constexpr int32_t E_Handle             = 0x80070006;
   constexpr int32_t E_NoInterface        = 0x80004002;
   
   /** ---------------------------------------------------------------------------
    * @brief base interface for other com interfaces, this solutions mimics IUnknown in Microsoft COM
   */
   struct unknown_i 
   {
      virtual int32_t query_interface( const guid& guidId, void** ppObject ) = 0;
      virtual unsigned add_reference() = 0;
      virtual unsigned release() = 0;
      
   protected:
      virtual ~unknown_i() = default;                                             // protected destructor prevents direct deletion
   };

   /// compare operator for guid, uses logic from gd::uuid to compare
   inline bool operator==( const guid& guid1, const guid& guid2 )
   {
      return *(const gd::uuid*)&guid1 == *(const gd::uuid*)&guid2;
   }

   /** ---------------------------------------------------------------------------
    * @brief template smart pointer to com interfaces
    */
   template <typename POINTER>
   struct pointer
   {
      pointer(): m_ppointer( nullptr ) {}
      pointer( POINTER* p_ ): m_ppointer( p_ ) { if( m_ppointer ) m_ppointer->add_reference(); }
      pointer( const pointer& o ) { m_ppointer = o.m_ppointer; m_ppointer->add_reference(); }
      pointer( pointer&& o ) { m_ppointer = o.m_ppointer; o.m_ppointer = nullptr; }
      ~pointer() { if( m_ppointer != nullptr ) m_ppointer->release(); }

      pointer& operator=( POINTER* p_ ) { 
         if( m_ppointer ) { m_ppointer->release(); }
         m_ppointer = p_;
         m_ppointer->add_reference();
         return *this;
      }

      pointer& operator=( const pointer& p_ ) { 
         if( m_ppointer ) { m_ppointer->release(); }
         m_ppointer = p_.m_ppointer;
         if( m_ppointer != nullptr ) m_ppointer->add_reference();
         return *this;
      }
      
      pointer& operator=( pointer&& o ) noexcept
      {
         if( this != &o )
         {
            if( m_ppointer ) m_ppointer->release();
            m_ppointer = o.m_ppointer;
            o.m_ppointer = nullptr;
         }
         return *this;
      }      

      POINTER* operator->() { return m_ppointer; }
      POINTER* operator->() const { return m_ppointer; }
      POINTER& operator*() { return *m_ppointer; }
      POINTER& operator*() const { return *m_ppointer; }
      POINTER** operator&() { return &m_ppointer; }

      operator POINTER*() const { return m_ppointer; }

      POINTER* get() const { return m_ppointer; }
      void reset() { if( m_ppointer ) { m_ppointer->release(); m_ppointer = nullptr; } }
      POINTER* detach() { POINTER* p_ = m_ppointer; m_ppointer = nullptr; return p_; }      

      POINTER* m_ppointer;
   };

   /** -------------------------------------------------------------------------
    * @brief Template implementation to wrap custom pointers with unknown_i interface
    * 
    * This template class allows any custom pointer to be used with the COM-style
    * interface system by providing the required reference counting and query interface
    * functionality through customizable callbacks or template arguments.
    * 
    * @tparam TYPE The underlying type to wrap
    * @tparam DELETE_FUNC Type of the delete function (default: std::default_delete<TYPE>)
    * @tparam QUERY_FUNC Type of the query interface function (default: nullptr)
    */
   template <typename TYPE, typename DELETE_FUNC = std::default_delete<TYPE>, typename QUERY_FUNC = nullptr_t>
   class pointer_impl : public unknown_i
   {
   public:
      /// Type definition for the query interface function
      using query_interface_func = std::function<int32_t(const guid&, void**)>;
      
      /// Type definition for the delete function
      using delete_func = std::function<void(TYPE*)>;

   // ## construction -------------------------------------------------------------
   public:
      /// Constructor taking ownership of a pointer
      pointer_impl(TYPE* pPointer, query_interface_func pQueryFunc = nullptr, delete_func pDeleteFunc = nullptr)
         : m_pPointer(pPointer), m_uReference(1), m_pQueryFunc(pQueryFunc ? pQueryFunc : default_query_interface_s), m_pDeleteFunc(pDeleteFunc ? pDeleteFunc : default_delete_s) {}

      /// Destructor - protected to prevent direct deletion
   protected:
      ~pointer_impl()
      {
         if( m_pPointer )
         {
            m_pDeleteFunc(m_pPointer);
            m_pPointer = nullptr;
         }
      }

   // ## unknown_i interface implementation ------------------------------------
   public:
      /** -----------------------------------------------------------------------
       * @brief Query for a specific interface
       * 
       * @param guidId The GUID of the requested interface
       * @param ppObject Output parameter for the interface pointer
       * @return int32_t S_OK on success, E_NOINTERFACE on failure
       */
      int32_t query_interface(const guid& guidId, void** ppObject) override
      {
         if( not ppObject )
            return E_Pointer;

         return m_pQueryFunc(guidId, ppObject);
      }

      /// @brief Increment reference count 
      unsigned add_reference() override { return ++m_uReference; }

      /// @brief Decrement reference count and delete if zero
      unsigned release() override
      {
         unsigned uCount = --m_uReference;
         if( uCount == 0 )
         {
            delete this;
         }
         return uCount;
      }

   // ## helper methods --------------------------------------------------------
   public:
      /// @brief Get the wrapped pointer
      TYPE* get() const {  return m_pPointer; }

      /** -----------------------------------------------------------------------
       * @brief Default query interface implementation
       * 
       * @param guidId The GUID of the requested interface
       * @param ppObject Output parameter for the interface pointer
       * @return int32_t E_NOINTERFACE (default implementation)
       */
      static int32_t default_query_interface_s(const guid& guidId, void** ppObject)
      {
         (void)guidId;  // Unused parameter
         (void)ppObject; // Unused parameter
         return E_NoInterface;
      }

      /// @brief Default delete implementation
      static void default_delete_s(TYPE* pPointer)
      {
         DELETE_FUNC deleteFunc;
         deleteFunc(pPointer);
      }

   // ## attributes -------------------------------------------------------------
   private:
      TYPE* m_pPointer;                     ///< Wrapped pointer
      unsigned m_uReference;                ///< Reference count
      query_interface_func m_pQueryFunc;    ///< Function to handle query_interface
      delete_func m_pDeleteFunc;            ///< Function to handle deletion
   };

   /** ---------------------------------------------------------------------------
    * @brief Convenience function to create pointer_impl instances
    * 
    * This function template creates a pointer_impl instance with the specified
    * pointer and optional custom query interface and delete functions.
    * 
    * @tparam TYPE The type of pointer to wrap
    * @param pPointer The pointer to wrap
    * @param pQueryFunc Optional custom query interface function
    * @param pDeleteFunc Optional custom delete function
    * @return pointer_impl<TYPE>* Pointer to the created instance
    */
   template <typename TYPE>
   pointer_impl<TYPE>* make_pointer_impl(
      TYPE* pPointer, 
      typename pointer_impl<TYPE>::query_interface_func pQueryFunc = nullptr,
      typename pointer_impl<TYPE>::delete_func pDeleteFunc = nullptr
   )
   {
      return new pointer_impl<TYPE>(pPointer, pQueryFunc, pDeleteFunc);
   }

} // namespace com
 
 using COMPONENT_GUID = com::guid;




_GD_END
