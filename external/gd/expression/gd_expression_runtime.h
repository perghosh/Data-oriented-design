/**
 * @file gd_expression_runtime.h
 * 
 * @brief 
 * 
 */



#pragma once

#include <cassert>
#include <functional>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd_expression.h"


#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

/**
 * \brief
 *
 *
 */
struct runtime
{
// ## construction ------------------------------------------------------------
   runtime() {}
   // copy
   runtime(const runtime& o) { common_construct(o); }
   runtime(runtime&& o) noexcept { common_construct(std::move(o)); }
   // assign
   runtime& operator=(const runtime& o) { common_construct(o); return *this; }
   runtime& operator=(runtime&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~runtime() {}
   // common copy
   void common_construct(const runtime& o) { m_stringError = o.m_stringError; }
   void common_construct(runtime&& o) noexcept { m_stringError = std::move( o.m_stringError ); }

// ## methods -----------------------------------------------------------------
   void add(const std::string& stringError, tag_error ) { m_stringError.push_back(stringError); }

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   std::vector<std::string> m_stringError;

// ## free functions ----------------------------------------------------------

};

_GD_EXPRESSION_END