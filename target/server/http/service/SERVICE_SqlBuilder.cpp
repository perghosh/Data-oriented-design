// @FILE [tag: sql, build] [description: SqlBuilder service is used to generate SQL queries] [name: SERVICE_SqlBuilder.cpp] [type: source]


#include "gd/gd_sql_value.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "gd/expression/gd_expression_glue_to_gd.h"

#include "SERVICE_SqlBuilder.h"

NAMESPACE_SERVICE_BEGIN

std::pair<bool, std::string> CSqlBuilder::Initialize( gd::argument::shared::arguments arguments_ )
{
   m_argumentsValues = arguments_;
   return { true, "" };
}

std::pair<bool, std::string> CSqlBuilder::Initialize( gd::argument::shared::arguments arguments_, std::string_view stringSql )
{
   m_argumentsValues = arguments_;
   m_stringSql = stringSql;

   return { true, "" };
}

static std::pair<bool, std::string> EXPRESSION_GetArgument_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto object_ = vectorArgument[1];                                                               assert(object_.is_pointer() == true);
   gd::argument::shared::arguments* parguments_ = (gd::argument::shared::arguments*)object_.get_pointer();

   auto& name_ = vectorArgument[0];
   if( name_.is_string() == true )
   {
      std::string stringName( name_.as_string() );
      if( stringName.empty() == true ) { return { false, "Argument name cannot be empty." }; }
      auto variantview_ = ( *parguments_ )[stringName].as_variant_view();
      *pvalueReturn = gd::expression::to_value_g( variantview_ );

      return { true, "" };
   }

   return { false, "Invalid argument name type, expected string." };
}

static std::pair<bool, std::string> EXPRESSION_Exists_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto object_ = vectorArgument[1];                                                               assert(object_.is_pointer() == true);
   gd::argument::shared::arguments* parguments_ = (gd::argument::shared::arguments*)object_.get_pointer();

   auto& name_ = vectorArgument[0];
   if( name_.is_string() == true )
   {
      std::string stringName( name_.as_string() );
      if( stringName.empty() == true ) { return { false, "Argument name cannot be empty." }; }
      auto variantview_ = ( *parguments_ )[stringName].as_variant_view();

      if( variantview_.is_null() == true ) { *pvalueReturn = false; }         // if argument not found, return false
      else { *pvalueReturn = true; }

      return { true, "" };
   }

   return { false, "Invalid argument name type, expected string." };
}


// Array of MethodInfo for visual studio operations
const gd::expression::method pmethodSource_g[] = {
   { (void*)&EXPRESSION_Exists_s, "exists", 2, 1 },
   { (void*)&EXPRESSION_GetArgument_s, "get_argument", 2, 1 },
};

const size_t uMethodSourceSize_g = sizeof(pmethodSource_g) / sizeof(gd::expression::method);




/**  -------------------------------------------------------------------------- Build
 * @CRITICAL [tag: build, sql] [description: main method to build SQL query from template and arguments]
 * @brief Build SQL query by replacing placeholders with values from arguments
 * 
 * Processes SQL template string (`m_stringSql`) by:
 * 1. Detecting preprocessing tags (`{??...??}`) for expression evaluation
 * 2. If found, evaluates embedded expressions using runtime with custom methods (`exists`, `get_argument`)
 * 3. Replaces argument placeholders (`{...}`) with values from `m_argumentsValues`
 * 
 * @param stringSqlReady Output parameter receiving the fully processed SQL string
 * @return std::pair<bool, std::string> Success flag and error message (empty on success)
 * 
 * @code
 * CSqlBuilder builder;
 * builder.Initialize(arguments_, "SELECT * FROM users WHERE id = {id} AND {??exists('name')}");
 * std::string stringSql;
 * auto [bOk, stringError] = builder.Build(stringSql);
 * @endcode
 */
std::pair<bool, std::string> CSqlBuilder::Build( std::string& stringSqlReady )
{
   std::string stringNew;

   // ## Test for preparsing, find "{??" to check if there are tags to process
   auto position_ = m_stringSql.find( "{??" );
   if( position_ != std::string::npos ) 
   {
      stringNew = m_stringSql;
      
      // ## prepare runtime for expression evaluation, add methods and variables
      using namespace gd::expression;
      gd::expression::runtime runtime_;
      runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""}); // global scode
      runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string( "str" ) } ); // str scope for string operations
      runtime_.add( { (unsigned)uMethodSourceSize_g, pmethodSource_g, std::string("args")});
      runtime_.set_variable( "args", std::pair<const char*, void*>("args", &m_argumentsValues)); // set source variable to the expression source

      // ## callback for expression evaluation, this will be called for each expression found in the string, and will return the result of the expression to replace in the string
      auto callback_ = [&]( const std::string_view& stringExpression, bool* pbError ) -> std::string
      {
         std::vector<gd::expression::token> vectorToken;
         std::pair<bool, std::string> result_ = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::vector<token> vectorPostfix;
         vectorPostfix.reserve( vectorToken.size() );
         result_ = gd::expression::token::compile_s(vectorToken, vectorPostfix, tag_postfix{});
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::vector<gd::expression::value> vectorReturn;
         result_ = gd::expression::token::calculate_s(vectorPostfix, &vectorReturn, runtime_);
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::string stringResult;
         if( vectorReturn.size() > 0 ) { stringResult = vectorReturn[0].as_string(); }

         return stringResult; // return empty variant if not found
      };


      bool bError = false;
      auto stringResult = gd::sql::replace_g( stringNew, m_argumentsValues, callback_, &bError, gd::sql::tag_preprocess{} ); 
      if( bError == true ) { return { false, "Error during SQL preprocessing, sql: " + m_stringSql }; }

      stringNew.clear();
      auto result_ = gd::sql::replace_g( stringResult, m_argumentsValues, stringNew, gd::sql::tag_brace{} ); 
      if( result_.first == false ) { return result_; }
      stringSqlReady = std::move(stringNew);
   }
   else
   {
      stringNew.reserve( m_stringSql.size() );
      auto result_ = gd::sql::replace_g( m_stringSql, m_argumentsValues, stringNew, gd::sql::tag_brace{} ); 
      if( result_.first == false ) { return result_; }
      stringSqlReady = std::move(stringNew);
   }

   return { true, "" };
}

NAMESPACE_SERVICE_END