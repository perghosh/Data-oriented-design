#include <filesystem>
#include <fuctional>

#include "gd/gd_variant_view.h"
#include "gd/gd_table_aggregate.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"


NAMESPACE_AUTOMATION_BEGIN

#include "Expression.h"

static std::pair<bool, std::string> CountLines_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}


static std::pair<bool, std::string> SelectLines_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}

static std::pair<bool, std::string> SelectBetween_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}


// Array of MethodInfo for visual studio operations
const method pmethodSelect_g[] = {
   { (void*)&CountLines_s, "count_lines", 1, 0, method::eFlagRuntime },
   { (void*)&SelectBetween_s, "select_between", 2, 0, method::eFlagRuntime },
   { (void*)&SelectLines_s, "select_lines", 1, 0, method::eFlagRuntime },
};

constexpr size_t uMethodSelectSize_g = sizeof(pmethodSelect_g) / sizeof(method);


// executes expression and takes callback to inject values to runtime
std::pair<bool, std::string> ExecuteExpression_g(const std::string_view& stringExpression, 
                                                 const std::vector<gd::expression::value>& vectorVariable
   )   
{
   // ## convert string to tokens
   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed
   std::vector<gd::expression::token> vectorPostfix;
   result = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## calculate the result
   // NOTE: vectorVariable is not defined in this scope. Assuming member variable or needs to be passed in.
   gd::expression::runtime runtime_(vectorVariable);

   runtime_.add( { uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   gd::expression::value valueResult;
   result = gd::expression::token::calculate_s(vectorPostfix, &valueResult, runtime_);
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   return { true, "" };
}


NAMESPACE_AUTOMATION_END