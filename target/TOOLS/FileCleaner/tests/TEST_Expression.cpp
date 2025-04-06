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



TEST_CASE( "[expression] create and read", "[expression]" ) {
   CApplication application;
   application.Initialize();

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "10 + 20 + 30 + 40 + 50";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::convert_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::evaluate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }


   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "1000 - 900 - 50 - 25";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::convert_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::evaluate_s(vectorCalculate, &valueResult);
      std::cout << "Result: " << valueResult.as_string() << std::endl;
      for( const auto& it : vectorCalculate ) { std::cout << "[ \"" << it.get_name() << "\" (" << it.get_type() << ") ] "; }
      std::cout << "\n\n";
   }

   {
      std::vector<gd::expression::token> vectorToken;
      std::string stringExpression = "'1000' + 50 + 70 + 20000";
      // tokenize the expression
      auto result_ = gd::expression::token::parse_s(stringExpression.c_str(), stringExpression.c_str() + stringExpression.length(), vectorToken, gd::expression::tag_formula{}); REQUIRE(result_.first == true);
      // prepare the expression
      std::vector<gd::expression::token> vectorCalculate;
      gd::expression::token::convert_s(vectorToken, vectorCalculate, gd::expression::tag_postfix{});

      gd::expression::value valueResult;
      gd::expression::token::evaluate_s(vectorCalculate, &valueResult);
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
