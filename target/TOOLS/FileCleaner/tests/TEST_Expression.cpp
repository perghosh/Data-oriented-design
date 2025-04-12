#include <chrono>
#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"
#include "gd/io/gd_io_repository_stream.h"
#include "gd/expression/gd_expression_token.h"

#include "main.h"

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

/// Generate path to data folder where files are located for tests
std::string GetDataFolder()
{
   return FOLDER_GetRoot_g("target/TOOLS/FileCleaner/tests/data");
}

/*
auto valueResult = gd::expression::token::calculate_s("length( text )", { {"text", "0123456789012345"} });
auto valueResult = gd::expression::token::calculate_s( "10 - -10" );
auto valueResult = gd::expression::token::calculate_s("min( 100, 200 ) + 999 + max( 10, 30 )");
auto valueResult = gd::expression::token::calculate_s( "10 >= x", {{"x", 10}} );
*/


TEST_CASE( "[expression] create and read", "[expression]" ) {
   CApplication application;
   application.Initialize();

   {
      auto valueResult = gd::expression::token::calculate_s( "x = 10; x" );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      //valueResult = gd::expression::token::calculate_s( "x = 10; x" );
      //std::cout << "Result: " << valueResult.as_string() << std::endl;
   }


   {
      auto valueResult = gd::expression::token::calculate_s("length( text )", { {"text", "0123456789012345"} });
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s("min( 100, 200 ) + 999 + max( 10, 30 )", { {"text", "0123456789012345"} });
      std::cout << "Result: " << valueResult.as_string() << std::endl;
   }


   {
      auto valueResult = gd::expression::token::calculate_s( "10 - -10" );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s( "1 * -1" );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s( "-1" );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      std::cout << "\n\n";
   }


   {
      auto valueResult = gd::expression::token::calculate_s( "10 >= x", {{"x", 10}} );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s( "10 > x", {{"x", 10}} );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s( "10 < x", {{"x", 10}} );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      std::cout << "\n\n";
   }

   {
      gd::expression::runtime runtime( []( auto string_, auto* pvalue_ ) -> bool {  
         if( string_ == "x" ) { *pvalue_ = 10; return true; }
         return false;
      });

      auto valueResult = gd::expression::token::calculate_s( "10 >= x", runtime );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      valueResult = gd::expression::token::calculate_s( "10 * 10 * x", runtime );
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      std::cout << "\n\n";
   }



   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "10 + x";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::runtime runtime( {{"x", 10}} );
      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult, runtime);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }


   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "(10 + 20) * (2 - 1)";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "10 + 5 * 5";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "5.0 / 3.0 + 7";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "5.0 / 3.0 + 7.0 * 5 / 2";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "5 == 5";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::compile_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::calculate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }


   /*
   std::string stringDataFolder = GetDataFolder();
   std::string stringFile = stringDataFolder + "/expression.txt";
   gd::file::path pathFile(stringFile);
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);
   */
   //std::string stringExpression = "if (a > 0) { b = 1; } else { b = 2; }";


   

   /*
   gd::expression::value valueResult;
   gd::expression::token::evaluate_s(vectorCalculate, &valueResult);
   std::cout << "Result: " << valueResult.as_string() << std::endl;
   */
}
