/**
 * @file gd_expression.h
 * @TAG #gd::expression
 * 
 * @brief Provides utilities for handling expressions, including operator precedence, 
 *        character code checks, and a general variant type for expression values.
 * 
 * This header defines several utility functions and types used in the context of 
 * expression parsing and evaluation. It includes support for operator precedence 
 * determination, character code validation, and a flexible variant type for storing 
 * expression values. Additionally, it defines tags for categorizing operations or 
 * states, which can be conditionally included based on the presence of `GD_TYPES_VERSION`.
 * 
 * @note If `GD_TYPES_VERSION` is defined, the tag types are imported from `gd::types`. 
 *       Otherwise, they are defined locally within this file.
 * 
 * @namespace gd::expression
 * Encapsulates all types and functions related to expression handling.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>
#include <variant>

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 


#ifdef GD_TYPES_VERSION
   using tag_error = gd::types::tag_error;
   using tag_optimize = gd::types::tag_optimize;
   using tag_single = gd::types::tag_single;
   using tag_double = gd::types::tag_double;
   using tag_multiple = gd::types::tag_multiple;
   using tag_namespace = gd::types::tag_namespace;
#else
   struct tag_error {};          ///< error is used in some form
   struct tag_optimize {};       ///< optimize is used in some form;
   struct tag_single {};         ///< single is used in some form;
   struct tag_double {};         ///< double is used in some form;
   struct tag_multiple {};       ///< multiple is used in some form;
   struct tag_namespace {};      ///< multiple is used in some form;
#endif

/// get the precedence of the operator
int to_precedence_g(const char* piOperator);
int to_precedence_g(const char iOperator);
/// get the precedence of the operator
int to_precedence_g(const char* piOperator, tag_optimize );
int to_precedence_g(char iOperator, tag_optimize );
/// check if the character is a code
int is_code_g( char iCharacter );
inline int is_code_g(const uint8_t iCharacter) { return is_code_g(char(iCharacter)); } ///< check if the character is a code

/// @brief general variant type with the basic types used in the expressions
using variant_t = std::variant<int64_t, double, std::string_view, bool,std::pair<const char*, void*> >; ///< value type



_GD_EXPRESSION_END