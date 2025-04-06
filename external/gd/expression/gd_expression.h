/**
 * @file gd_expression.h
 * 
 * @brief 
 * 
 */

#pragma once

#include <cassert>

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 


#ifdef GD_TYPES_VERSION
   using tag_error = gd::types::tag_error;
   using tag_optimize = gd::types::tag_optimize;
#else
   struct tag_error {};          ///< error is used in some form
   struct tag_optimize {};       ///< optimize is used in some form;
#endif

/// get the precedence of the operator
int to_precedence_g(const char iOperator);
/// get the precedence of the operator
int to_precedence_g(const char iOperator, tag_optimize );



_GD_EXPRESSION_END