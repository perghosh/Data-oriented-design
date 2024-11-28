/**
 * \file gd_arguments_helper.h
 * 
 * \brief general code used for arguments objects
 * 
 */

#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_variant_view.h"

#ifndef _GD_ARGUMENT_BEGIN
#define _GD_ARGUMENT_BEGIN namespace gd::argument {
#define _GD_ARGUMENT_END }
#endif

_GD_ARGUMENT_BEGIN

/**
 * \brief
 *
 *
 */
struct index_edit
{
   enum enumType { eTypeUnknown, eTypeString, eTypePair, eTypeIndex };
// ## construction ------------------------------------------------------------
   index_edit() {}
   index_edit( const std::string_view& stringName ): m_stringName(stringName), m_uType(eTypeString) {}
   index_edit( const std::string_view& stringName, uint32_t ): m_stringName(stringName), m_uType(eTypeString) {}
   index_edit( uint64_t uIndex ): m_uIndex(uIndex), m_uType(eTypeIndex) {}

   //operator std::string_view() const { assert( m_uType == eTypeString ); return m_stringName; }
   //operator uint64_t() const { assert( m_uType == eTypeIndex ); return m_uIndex; }

// ## methods -----------------------------------------------------------------
   //void set( const gd::variant_view& )
   bool is_string() const noexcept { return m_uType == eTypeString; }
   bool is_index() const noexcept { return m_uType == eTypeIndex; }
   bool is_second_index() const noexcept { return m_uSecondIndex != 0; }
   
   const std::string_view& get_string() const { assert( m_uType == eTypeString ); return m_stringName; }
   uint64_t get_index() const { assert( m_uType == eTypeIndex ); return m_uIndex; }

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   unsigned m_uType = eTypeUnknown;
   union
   {
      const std::string_view m_stringName;
      const std::pair< uint8_t*, uint8_t* > m_pairValue;
      const uint64_t m_uIndex;
   };

   uint32_t m_uSecondIndex = 0; ///< index for sub item, used for named ranges

// ## free functions ----------------------------------------------------------

};

inline index_edit operator ""_edit(const char* pbsz, size_t uSize) {
   return index_edit( std::string_view{ pbsz, uSize } );
}

inline index_edit operator ""_edit(unsigned long long int uIndex) {
   return index_edit( uIndex );
}



_GD_ARGUMENT_END