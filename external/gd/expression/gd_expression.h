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
#else
   struct tag_error {};          ///< error is used in some form
#endif




_GD_EXPRESSION_END