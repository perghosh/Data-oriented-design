#include <random>
#include <chrono>
#include <memory>
#include <ranges>
#include <filesystem>


#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_file.h"
#include "gd/gd_file_rotate.h"
#include "gd/gd_utf8.h"
#include "gd/gd_database_sqlite.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_printer2.h"

/// Return folder for application as string.
std::string GetApplicationFolder()
{
   std::string stringFilePath = mainarguments_g.m_ppbszArgumentValue[0];
   auto position_ = stringFilePath.find_last_of("\\/");
   if( position_ != std::string::npos ) { stringFilePath = stringFilePath.substr( 0, position_ + 1 ); }

   return stringFilePath;
}

TEST_CASE( "[sqlite] create", "[sqlite]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->clear();

   std::string stringSql = R"SQL(CREATE TABLE TUser (
      UserK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FName VARCHAR(50),
      FSurname VARCHAR(50),
      FAge INTEGER,
      FGender INTEGER
   );)SQL";

   std::string stringDbName = GetApplicationFolder();
   stringDbName += "db01.sqlite";
   if( std::filesystem::exists( stringDbName ) == true ) { std::filesystem::remove( stringDbName ); }

   gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i("db01");
   auto result_ = pdatabase->open( { {"file", stringDbName }, {"create", true } } );               REQUIRE( result_.first == true );
   pdatabase->execute( stringSql );

   pdatabase->close();
   pdatabase->release();

   if( std::filesystem::exists(stringDbName) == true ) { std::filesystem::remove(stringDbName); }

   std::cout << GetApplicationFolder() << "\n";

}

TEST_CASE( "[sqlite] arguments table", "[sqlite]" ) {
   //gd::table::arguments::table table(10);
   {
      gd::table::arguments::table table_(  (unsigned)gd::table::arguments::table::eTableFlagAll,{ { "int64", 0, "FInteger"} }, gd::table::tag_prepare{} );

      auto uArgumentsSize = sizeof( gd::argument::shared::arguments );

      auto uRow = table_.get_row_count();
      table_.row_add();
      table_.cell_set( uRow, "FInteger", int64_t(10) );
      auto* parguments_ = table_.row_create_arguments(uRow);
      parguments_->set("ten", uint32_t(10));

      parguments_ = table_.row_get_arguments_pointer(uRow);                                        REQUIRE(parguments_->size() == 1);
      parguments_->set("eleven", uint32_t(11));                                                    REQUIRE(parguments_->size() == 2);

      //char* pbszData = new char[100];

      table_.cell_set( uRow, "new", uint32_t(10) );
      auto arguments_ = table_.row_get_arguments( 0 );

      auto string_d = gd::argument::debug::print(arguments_);
      std::cout << string_d << "\n";

      // int i = table_.cell_get_variant_view(0, "FInteger").get<int>();
   }
}

/*

#include "malloc.h"

extern "C"
{
   #define SUCCESS  1
   #define FAILURE  0

   typedef struct MarkovNode {
      char* data; 
      int list_length; 
      struct MarkovNodeFrequency *frequency_list; 
   } MarkovNode;

   typedef struct MarkovNodeFrequency {
      struct MarkovNode* markov_node; 
      int frequency; 
   } MarkovNodeFrequency;

   int add_node_to_frequency_list(MarkovNode* pmarkovnodeFirst, MarkovNode* pmarkovnodeSocond)
   {
      // ## compare if the same pointer, if it is increase frequency
      for( int i = 0; i < pmarkovnodeFirst->list_length; ++i )
      {
         MarkovNodeFrequency* pmarkovnodefrequency = &pmarkovnodeFirst->frequency_list[i];
         if( pmarkovnodefrequency->markov_node == pmarkovnodeSocond )
         {
            pmarkovnodefrequency->frequency++;
            return SUCCESS;
         }
      }

      // ## increase frequency list size by 1 and add the new MarkovNode

      pmarkovnodeFirst->list_length++; // increase list length by 1 to store the new MarkovNode
      size_t uReallocSize = pmarkovnodeFirst->list_length * sizeof(MarkovNodeFrequency);// calculate the new size
      MarkovNodeFrequency* pmarkovnodefrequency = (MarkovNodeFrequency*)realloc( pmarkovnodeFirst->frequency_list, uReallocSize );
      if(pmarkovnodefrequency == NULL) { return FAILURE; }

      // ### Add new node to frequency list
      pmarkovnodefrequency[pmarkovnodeFirst->list_length - 1].markov_node = pmarkovnodeSocond;
      pmarkovnodefrequency[pmarkovnodeFirst->list_length - 1].frequency = 1;
      pmarkovnodeFirst->frequency_list = pmarkovnodefrequency;
      return SUCCESS;
   }

   /// ------------------------------------------------------------------------
   /// Original code
   int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode *second_node)
   {
      // compare if the same pointer, if it is increase frequency
      for (int i = 0; i < first_node->list_length; ++i)
      {
         if (first_node->frequency_list[i].markov_node == second_node)
         {
            first_node->frequency_list[i].frequency++;
            return SUCCESS;
         }
      }
      // increase frequency list size by 1 and add the new MarkovNode
      // HERE!!
      MarkovNodeFrequency *fl = realloc(first_node->frequency_list,
         (++first_node->list_length)*sizeof(*fl));
      if (fl == NULL)
      {
         return FAILURE;
      }
      first_node->frequency_list = fl;
      first_node->frequency_list[first_node->list_length - 1].markov_node =
         second_node;
      first_node->frequency_list[first_node->list_length - 1].frequency = 1;
      return SUCCESS;
   }
}
*/
