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

struct tag_list {};                                                            ///< operations that use some sort of container class in stl  
struct tag_memory {};                                                          ///< logic around memory
struct tag_pair {};                                                            ///< tag dispatcher used to select working with pair items instead of vector
struct tag_parse {};                                                           ///< methods that parse
struct tag_parse_type{};                                                       ///< tag to try to parse type of value
struct tag_align {};                                                           ///< align related methods
struct tag_section {};                                                         ///< section related methods, section in arguments is a named value with multiple non named values after


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
   index_edit( const std::string_view& stringName, uint32_t uSecondIndex ): m_stringName(stringName), m_uType(eTypeString), m_uSecondIndex(uSecondIndex) {}
   index_edit( uint64_t uIndex ): m_uIndex(uIndex), m_uType(eTypeIndex) {}

   //operator std::string_view() const { assert( m_uType == eTypeString ); return m_stringName; }
   //operator uint64_t() const { assert( m_uType == eTypeIndex ); return m_uIndex; }

// ## methods -----------------------------------------------------------------
   bool is_string() const noexcept { return m_uType == eTypeString; }
   bool is_index() const noexcept { return m_uType == eTypeIndex; }
   bool is_second_index() const noexcept { return m_uSecondIndex != 0; }

   /// return value for second index, second index = sub index, named value with un-named values that follow
   uint32_t get_second_index() const noexcept { return m_uSecondIndex; }
   
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