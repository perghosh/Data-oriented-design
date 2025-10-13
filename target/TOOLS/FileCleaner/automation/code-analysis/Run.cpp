/**                                                                            @CODE [tag: expression, where] [description: run expressions in file cleaner]
 * \file Run.cpp
 * \brief Contains the implementation of expression methods used for cleaner
 * 
 [description: "## Expession methods
  Methods converts expressions from strings to logic and perform operations on table data.
  What they do is to iterate through table data and perform operations on each line."]
 [tags: "gd::expression, gd::table"]
 */

// ## convert string to tokens
#include "gd/expression/gd_expression_value.h"
// ## convert string to tokens
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "gd/expression/gd_expression_glue_to_gd.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"

#include "Expression.h"

#include "Run.h"

using namespace gd::expression;
using namespace AUTOMATION;


/** 
 * @brief Executes an expression given as a string, using the provided arguments and table data.
 * 
 * - configure runtime with methods for default operations
 * - iterate through the table lines to process the expression
 * 
 * This function parses the expression string into tokens, compiles them into postfix notation,
 * and then evaluates the expression using the provided arguments and table data.
 * 
 * @param stringExpression The expression to be evaluated.
 * @param argumentsCode The arguments to be used in the expression.
 * @param ptableLineList Pointer to a table containing line data.
 * @param ptableSnippet Pointer to a table for storing snippets or results.
 * 
 * @return A pair containing a boolean indicating success or failure, and a string with the result or error message.
 */
std::pair<bool, std::string> RunExpression_g(const std::string_view& stringExpression, const gd::argument::shared::arguments& argumentsCode, const gd::table::dto::table* ptableLineList, gd::table::dto::table* ptableSnippet)
{
#ifndef NDEBUG
   std::string stringTableList_d = gd::table::debug::print(ptableLineList, gd::table::tag_columns{});
   std::string stringTableSnippet_d = gd::table::debug::print(ptableSnippet, gd::table::tag_columns{});
#endif 

   // ## convert string to tokens

   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result_ = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
   if( result_.first == false ) { throw std::invalid_argument(result_.second); }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed

   std::vector<gd::expression::token> vectorPostfix;
   result_ = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result_.first == false ) { throw std::invalid_argument(result_.second); }

   // ## create runtime and add methods for operations

   gd::expression::runtime runtime_;
   runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { (unsigned)uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   ExpressionSource source_;
   runtime_.set_variable( "source", std::pair<const char*, void*>("source", &source_)); // set source variable to the expression source

   // ## add variables from arguments into the runtime for use in expession
   for( auto it = argumentsCode.begin(), itEnd = argumentsCode.end(); it != itEnd; it++ )
   {
      if( it.is_name() == true )
      {
         const auto vv_ = it.get_argument().as_variant_view();
         runtime_.set_variable(it.name(), gd::expression::to_value_g( vv_ ) ); // set each argument as a variable in the runtime
      }
   }

   std::string stringFileName;
   for( uint64_t uRow = 0; uRow < ptableLineList->size(); ++uRow )
   {
      auto filename_ = ptableLineList->cell_get_variant_view(uRow, "filename").as_string_view();
      if( stringFileName != filename_ )
      {
         stringFileName = std::string(filename_);
         runtime_.set_variable("filename", stringFileName);                   // set filename variable to the current file
         source_.set_file(stringFileName);                                    // set the source file for the expression
         source_.Reset();
         result_ = source_.OpenFile();                                        // Open internal file and prepare for reading
         if( result_.first == false ) { return result_; }
      }
      else
      {
         // reset file to start
         source_.Seek();                                                      // reset the file pointer to the start of the file
         source_.Reset();
      }

      auto line_ = ptableLineList->cell_get_variant_view(uRow, "line").as_string_view();
      runtime_.set_variable("line", std::string( line_ ) ); // set line variable to the current line
      int64_t row_ = ptableLineList->cell_get_variant_view(uRow, "row").as_int64();
      runtime_.set_variable("row", row_ );                                    // set line variable to the current line
      source_.set_goto_line(row_ + 1); // set the current line number in the source, we add 1 because the source code is 1-based index, while the table is 0-based index  

      gd::expression::value valueResult;
      std::vector<gd::expression::value> vectorReturn;
      result_ = gd::expression::token::calculate_s(vectorPostfix, &vectorReturn, runtime_);
      if( result_.first == false ) { return result_; }

      

      for( const auto& value_ : vectorReturn )
      {
         auto uAddSnippetRow = ptableSnippet->row_add_one();                  // add row
			ptableSnippet->cell_set(uAddSnippetRow, "key", uAddSnippetRow + 1);  // set key to row + 1
         uint64_t uFileKey = ptableLineList->cell_get_variant_view(uRow, "key");
         ptableSnippet->cell_set(uAddSnippetRow, "file-key", uFileKey );
         ptableSnippet->cell_set(uAddSnippetRow, "filename", stringFileName);
         ptableSnippet->cell_set(uAddSnippetRow, "row", (uint64_t)row_);
         ptableSnippet->cell_set(uAddSnippetRow, "snippet", value_.get_string()); // use value_ instead of valueResult
      }
   }

   return { true, "" }; // Placeholder for actual implementation
}



/** ---------------------------------------------------------------------------
 * @brief Executes a "where" expression on a table, filtering rows based on the expression.
 * 
 * - configure runtime with methods for default operations
 * - iterate through the table lines to process the expression
 * 
 * This function parses the expression string into tokens, compiles them into postfix notation,
 * and then evaluates the expression for each row in the provided table. Rows that do not satisfy
 * the expression are marked for deletion and removed from the table.
 * 
 * @param stringExpression The "where" expression to be evaluated.
 * @param ptableKeyValue Pointer to a table containing key-value data to be filtered.
 * 
 * @return A pair containing a boolean indicating success or failure, and a string with the result or error message.
 * 
 * @code
 * // Example usage: Filtering a table based on a "where" expression
   std::string stringExpression("(str::toupper( source::get_argument(args,'owner') ) == 'PER') || (source::get_argument(args,'owner') == 'kevin')");
   auto result_ = RunExpression_Where_g(stringExpression, &tableKeyValue);
   @endcode
 */
std::pair<bool, std::string> RunExpression_Where_g(const std::string_view& stringExpression, gd::table::arguments::table* ptableKeyValue)
{
   // ## convert string to tokens

   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result_ = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
   if( result_.first == false ) { return result_; }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed

   std::vector<gd::expression::token> vectorPostfix;
   result_ = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result_.first == false ) { return result_; }

   // ## create runtime and add methods for operations

   gd::expression::runtime runtime_;
   runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { (unsigned)uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   std::vector<uint64_t> vectorDeleteRow; // rows to delete

   runtime_.set_variable( "argstable", std::pair<const char*, void*>( "argstable", ptableKeyValue ) ); // set table used to calculate on

   for( uint64_t uRow = 0; uRow < ptableKeyValue->size(); uRow++ )
   {
      auto* parguments_ = ptableKeyValue->row_get_arguments_pointer(uRow);
      if( parguments_ == nullptr ) { continue; }                              // if there are no arguments, we cannot evaluate the expression

      runtime_.set_variable( "args", std::pair<const char*, void*>("args", parguments_)); // set source variable to the expression source

      gd::expression::value valueResult;
      std::vector<gd::expression::value> vectorReturn;
      result_ = gd::expression::token::calculate_s(vectorPostfix, &vectorReturn, runtime_); // calculate the expression

      bool bWhere = false;
      for( const auto& value_ : vectorReturn )
      {
         if( value_.is_bool() == true )
         {
            bool bResult = value_.get_bool();
            if( bResult == true ) { bWhere = true; break; }
         }
      }

      if( bWhere == false ) { vectorDeleteRow.push_back(uRow); } // mark row for deletion

      if( result_.first == false ) { return result_; }
   }
                                                                                                   LOG_VERBOSE_RAW("== Keep Rows: " & (ptableKeyValue->size() - vectorDeleteRow.size()));
   if( vectorDeleteRow.empty() == false ) { ptableKeyValue->erase(vectorDeleteRow); }  // erase rows that did not match the where condition

   return { true, "" };
}



/** ---------------------------------------------------------------------------
 * @brief Executes a "where" expression on a table, filtering rows based on the expression.
 * 
 * - configure runtime with methods for default operations
 * - iterate through the table lines to process the expression
 * 
 * This function parses the expression string into tokens, compiles them into postfix notation,
 * and then evaluates the expression for each row in the provided table. Rows that do not satisfy
 * the expression are marked for deletion and removed from the table.
 * 
 * @param stringExpression The "where" expression to be evaluated.
 * @param ptable_ Pointer to a table containing data to be filtered.
 * 
 * @return A pair containing a boolean indicating success or failure, and a string with the result or error message.
 * 
 * @code
 * // Example usage: Filtering a table based on a "where" expression
   @endcode
 */
std::pair<bool, std::string> RunExpression_Where_g(const std::string_view& stringExpression, gd::table::dto::table* ptable_)
{
   // ## convert string to tokens

   std::vector<gd::expression::token> vectorPostfix;
   std::pair<bool, std::string> result_ = gd::expression::token::parse_s(stringExpression, vectorPostfix, gd::expression::tag_postfix{});
   if( result_.first == false ) { return result_; }

   // ## create runtime and add methods for operations .......................

   gd::expression::runtime runtime_;
   runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { (unsigned)uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   std::vector<uint64_t> vectorDeleteRow; // rows to delete

   runtime_.set_variable("dtotable", std::pair<const char*, void*>("dtotable", ptable_)); // set table used to calculate on

   for( uint64_t uRow = 0; uRow < ptable_->size(); uRow++ )
   {
      runtime_.set_variable("row", (int64_t)uRow);                            // set line variable to the current line

      gd::expression::value valueResult;
      std::vector<gd::expression::value> vectorReturn;
      result_ = gd::expression::token::calculate_s(vectorPostfix, &vectorReturn, runtime_); // calculate the expression
      if( result_.first == false ) { return result_; }

      bool bWhere = false;
      for( const auto& value_ : vectorReturn )
      {
         if( value_.is_bool() == true )
         {
            bool bResult = value_.get_bool();
            if( bResult == true ) { bWhere = true; break; }
         }
      }

      if( bWhere == false ) { vectorDeleteRow.push_back(uRow); } // mark row for deletion

      if( result_.first == false ) { return result_; }
   }
                                                                                                   LOG_VERBOSE_RAW("== Keep Rows: " & (ptable_->size() - vectorDeleteRow.size()));
   if( vectorDeleteRow.empty() == false ) { ptable_->erase(vectorDeleteRow); }  // erase rows that did not match the where condition

   return { true, "" };
}

