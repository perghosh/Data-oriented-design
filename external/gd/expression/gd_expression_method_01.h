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

std::pair<bool, std::string> tolower_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> toupper_g(const std::vector< value >& vectorArgument, value* pvalueResult);

std::pair<bool, std::string> count_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> has_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> missing_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> starts_with_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> ends_with_g(const std::vector< value >& vectorArgument, value* pvalueResult);


// Array of MethodInfo definitions
const method pmethodDefault_g[] = {
   { (void*)&average_g, "average", 2, 1},
   { (void*)&max_g, "max", 2, 1 },
   { (void*)&min_g, "min", 2, 1 },
   { (void*)&sum_g, "sum", 2, 1 }
};

constexpr size_t uMethodDefaultSize_g = sizeof(pmethodDefault_g) / sizeof(method);

const method pmethodString_g[] = {
   { (void*)&count_g, "count", 2, 1 },
   { (void*)&ends_with_g, "ends_with", 2, 1 },
   { (void*)&has_g, "has", 2, 1 },
   { (void*)&length_g, "length", 1, 1 },
   { (void*)&missing_g, "missing", 2, 1 },
   { (void*)&starts_with_g, "starts_with", 2, 1 },
   { (void*)&tolower_g, "tolower", 1, 1},
   { (void*)&toupper_g, "toupper", 1, 1 }
};

constexpr size_t uMethodStringSize_g = sizeof(pmethodString_g) / sizeof(method);



_GD_EXPRESSION_END