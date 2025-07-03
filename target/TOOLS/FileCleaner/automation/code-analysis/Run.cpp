#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "Expression.h"

#include "Run.h"

using namespace gd::expression;
using namespace AUTOMATION;

std::pair<bool, std::string> RunExpression_g(const std::string_view& stringExpression, const gd::argument::shared::arguments& argumentsCode, const gd::table::dto::table* ptableLineList, gd::table::dto::table* ptableSnippet)
{
   // ## convert string to tokens
   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed
   std::vector<gd::expression::token> vectorPostfix;
   result = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   gd::expression::runtime runtime_;

   runtime_.add( { uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   std::string stringFileName;
   for( uint64_t uRow = 0; uRow < ptableLineList->size(); ++uRow )
   {
      
   }


   
   return { true, "" }; // Placeholder for actual implementation
}
