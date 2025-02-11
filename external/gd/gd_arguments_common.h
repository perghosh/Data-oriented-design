/**
 * \file gd_arguments_common.h
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
#define _GD_ARGUMENT_BEGIN namespace gd { namespace argument {
#define _GD_ARGUMENT_END } }
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
   index_edit( const std::string_view& stringName ): m_uType(eTypeString), m_stringName(stringName) {}
   index_edit( const std::string_view& stringName, uint32_t uSecondIndex ): m_uType(eTypeString), m_stringName(stringName), m_uSecondIndex(uSecondIndex) {}
   index_edit( uint64_t uIndex ): m_uType(eTypeIndex), m_uIndex(uIndex) {}

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

/**
* \brief simplify to work with values arguments
*
* `arguments_value` is a helper object to simplify work with values in command object. It is used 
* to set and get values and enables easier access to values using some compiler featuers. 
* Never use `arguments_value` directly, use `command` object instead.
*/
template< typename ARGUMENTS >
struct arguments_value
{
   using pointer = typename ARGUMENTS::pointer;
   arguments_value() : m_parguments{ nullptr }, m_pPosition( nullptr ) {};
   arguments_value(const std::string_view& stringName) : m_stringName{ stringName }, m_parguments{ nullptr }, m_pPosition( nullptr ) {}
   arguments_value(ARGUMENTS* parguments) : m_parguments{ parguments }, m_pPosition( parguments->buffer_data() ) {}
   arguments_value(ARGUMENTS* parguments, typename ARGUMENTS::pointer pPosition) : m_parguments{ parguments }, m_pPosition( pPosition ) {}
   arguments_value(ARGUMENTS* parguments, const std::string_view& stringName) : m_parguments{ parguments }, m_stringName{ stringName }, m_pPosition( nullptr ) {}

   arguments_value(const arguments_value& o) : m_stringName{ o.m_stringName },  m_parguments{ o.m_parguments }, m_pPosition( o.m_pPosition ) {}

   operator ARGUMENTS&() { return *m_parguments; }
   operator const ARGUMENTS&() const { return *m_parguments; }
   operator gd::variant_view();

   const ARGUMENTS* get_arguments() const { return m_parguments; }
   const typename ARGUMENTS::pointer get_position() const { return m_pPosition; }

   arguments_value& operator[](const std::string_view& stringName) { m_stringName = stringName; m_pPosition = nullptr; return *this; }

   arguments_value& operator=(const arguments_value& o) { m_stringName = o.m_stringName; m_parguments = o.m_parguments; return *this; }
   arguments_value& operator=(const gd::variant_view& variantviewValue);
   arguments_value& operator+=(const gd::variant_view& variantviewValue) { m_parguments->append_argument(m_stringName, variantviewValue, gd::types::tag_view{}); return *this; }
   arguments_value& operator<<(const gd::variant_view& variantviewValue) { m_parguments->append_argument(m_stringName, variantviewValue, gd::types::tag_view{}); return *this; }
   arguments_value& operator=(std::pair<std::string_view, gd::variant_view> pair_) { m_parguments->set(pair_.first, pair_.second); return *this; }  
   arguments_value& operator+=(std::pair<std::string_view, gd::variant_view> pair_) { m_parguments->append_argument(pair_.first, pair_.second); return *this; }  
   arguments_value& operator<<(std::pair<std::string_view, gd::variant_view> pair_) { m_parguments->append_argument(pair_.first, pair_.second); return *this; }  
   arguments_value& operator>>(gd::variant_view& v__);
   /// stream value into variable
   template< typename VARIABLE > arguments_value& operator>>(VARIABLE& v_);

   std::string_view m_stringName; ///< name of value that value represents
   typename ARGUMENTS::pointer m_pPosition;
   ARGUMENTS* m_parguments;       ///< pointer to internal arguments object found in command
};

/// return variant value 
template<typename ARGUMENTS>
arguments_value<ARGUMENTS>::operator gd::variant_view() 
{
   if(m_pPosition != nullptr) {
      return m_parguments->get_argument( m_pPosition ).as_variant_view();
   } 
   else {
      m_pPosition = m_parguments->find(m_stringName);
      if( m_pPosition != nullptr ) return m_parguments->get_argument( m_pPosition ).as_variant_view();
   }
   return gd::variant_view();
}

/// set value in arguments object
/// updates position when after value is set
template< typename ARGUMENTS >
arguments_value<ARGUMENTS>& arguments_value<ARGUMENTS>::operator=(const gd::variant_view& variantviewValue) {
   if( m_pPosition != nullptr ) { assert( m_parguments->verify_d( m_pPosition ) ); m_parguments->set(m_pPosition, variantviewValue, &m_pPosition); }
   else {
      m_parguments->set(m_stringName, variantviewValue); 
      m_pPosition = m_parguments->find(m_stringName);                                              assert(m_pPosition != nullptr);
   }                                                                                               assert( m_parguments->verify_d( m_pPosition ) );
   return *this; 
}

/// get value from arguments object at current position and move to next position
/// @code
/// @endcode
template< typename ARGUMENTS >
arguments_value<ARGUMENTS>& arguments_value<ARGUMENTS>::operator>>(gd::variant_view& vv_ ){ 
   if( m_pPosition != nullptr ) {
      vv_ = m_parguments->get_argument(m_pPosition).as_variant_view();
      m_pPosition = m_parguments->next(m_pPosition);
   }
   else if( m_stringName.empty() == false ) { vv_ = m_parguments->get_argument(m_stringName).as_variant_view(); }
   else vv_ = gd::variant_view();
   return *this; 
}

/// get value from arguments object at current position and move to next position
/// @code
/// gd::argument::shared::arguments arguments_;
/// gd::argument::arguments_value AV_( &arguments_ );
/// AV_ << 1 << 2 << 3 << 4 << 5;
/// int i1, i2, i3, i4, i5;
/// gd::argument::arguments_value AVRead( &arguments_ );
/// AVRead >> i1 >> i2 >> i3 >> i4 >> i5;
/// @endcode
template< typename ARGUMENTS > 
   template< typename VARIABLE > 
arguments_value<ARGUMENTS>& arguments_value<ARGUMENTS>::operator>>(VARIABLE& v_) { 
   auto argument_ = m_parguments->get_argument(m_pPosition);
   v_ = argument_.as_variant_view().template as<VARIABLE>();                   // convert to value 
   m_pPosition = m_parguments->next(m_pPosition); 
   return *this; 
}

_GD_ARGUMENT_END