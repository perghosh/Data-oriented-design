/**
* @file gd_expression_method_01.h
* 
* @brief 
* 
*/

#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "gd_expression.h"
#include "gd_expression_value.h"
#include "gd_expression_runtime.h"

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

std::pair<bool, std::string> average_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> length_g( const std::vector< value >& vectorArgument, value* pvalueResult );
std::pair<bool, std::string> max_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> min_g(const std::vector< value >& vectorArgument, value* pvalueResult);  
std::pair<bool, std::string> sum_g(const std::vector< value >& vectorArgument, value* pvalueResult);


// Array of MethodInfo definitions
const method pmethodDefault_g[] = {
   { (void*)&average_g, "average", 2, 1},
   { (void*)&length_g, "length", 1, 1 },
   { (void*)&max_g, "max", 2, 1 },
   { (void*)&min_g, "min", 2, 1 },
   { (void*)&sum_g, "sum", 2, 1 }
};


_GD_EXPRESSION_END