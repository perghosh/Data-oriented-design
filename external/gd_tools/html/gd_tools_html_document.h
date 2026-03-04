// @FILE [tag: binary] [description: Handle xml and html documents] [type: header] [name: gd_tools_html_document.h]


#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"


#ifndef _GD_TOOLS_HTML_BEGIN
#define _GD_TOOLS_HTML_BEGIN namespace gd { namespace tools { namespace html {
#define _GD_TOOLS_HTML_END } } }
#endif

_GD_TOOLS_HTML_BEGIN

/**
 * \brief 
 *
 *
 */
struct element {
   element() = default;
   explicit element(std::string_view stringName) : m_stringName(stringName) {}
   
   // copy
   element( const element& o ) { common_construct( o ); }
   element( element&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   element& operator=( const element& o ) { common_construct( o ); return *this; }
   
   ~element() {}
   
   void common_construct( const element& o );

// ## operators ---------------------------------------------------------------
   
   element* operator[]( size_t uIndex ) { return m_vectorElement[uIndex].get(); }
   const element* operator[]( size_t uIndex ) const { return m_vectorElement[uIndex].get(); }
   
   
   
// ## getters/setter  ---------------------------------------------------------

   element* parent() const { return m_pelementParent; }
   void set_parent( element* pelementParent ) { m_pelementParent = pelementParent; }
   
// ## methods -----------------------------------------------------------------

   element* add(std::unique_ptr<element> pelementChild);
   void add_attribute( std::string_view stringName, std::string_view stringContent );


   element* at( size_t uIndex ) { return m_vectorElement[uIndex].get(); }
   const element* at( size_t uIndex ) const { return m_vectorElement[uIndex].get(); }
   element* front() { return m_vectorElement.front().get(); }
   const element* front() const { return m_vectorElement.front().get(); }
   element* back() { return m_vectorElement.back().get(); }
   const element* back() const { return m_vectorElement.back().get(); }
   size_t size() const noexcept { return m_vectorElement.size(); }
   

/** \name DEBUG
 *///@{
   
//@}

// ## attributes ------Name-----------------------------------------------------
   std::string m_stringName;
   std::string m_stringContent;
   element*    m_pelementParent = nullptr;
   std::vector<std::unique_ptr<element>> m_vectorElement;
   gd::argument::shared::arguments m_argumentsAttribute;
   
// ## free functions ----------------------------------------------------------

};


_GD_TOOLS_HTML_END
