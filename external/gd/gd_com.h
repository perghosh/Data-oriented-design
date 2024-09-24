#pragma once
#include <cassert>
#include <string>
#include <string_view>

#include "gd_uuid.h"

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd { 
#define _GD_END } 
_GD_BEGIN
#else
_GD_BEGIN
#endif


namespace com {
   struct guid { uint32_t u1; uint16_t u2; uint16_t u3; uint8_t  pu4[8]; };

   constexpr int32_t S_Ok                 = 0x00000000;

   constexpr int32_t E_Fail               = 0x80004005;

   constexpr int32_t E_Pointer            = 0x80004003;
   constexpr int32_t E_InvalidArgument    = 0x80070057;
   constexpr int32_t E_Abort              = 0x80004004;
   constexpr int32_t E_Handle             = 0x80070006;
   constexpr int32_t E_NoInterface        = 0x80004002;


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
      pointer( POINTER* p_ ): m_ppointer( p_ ) {}
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

      POINTER* operator->() { return m_ppointer; }
      POINTER* operator->() const { return m_ppointer; }
      POINTER& operator*() { return *m_ppointer; }
      POINTER& operator*() const { return *m_ppointer; }
      POINTER** operator&() { return &m_ppointer; }

      operator POINTER*() const { return m_ppointer; }

      POINTER* get() const { return m_ppointer; }

      POINTER* m_ppointer;
   };

};

using COMPONENT_GUID = com::guid;


/** ---------------------------------------------------------------------------
 * @brief base interface for other com interfaces, this solutions mimics IUnknown in Microsoft COM
*/
struct unknown_i {
  virtual int32_t query_interface(const com::guid& guidId, void** ppObject) = 0;
  virtual unsigned add_reference() = 0;
  virtual unsigned release() = 0;
};


_GD_END