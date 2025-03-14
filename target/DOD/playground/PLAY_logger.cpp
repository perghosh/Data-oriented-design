#include <random>
#include <chrono>
#include <memory>
#include <ranges>

#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_file.h"
#include "gd/gd_file_rotate.h"
#include "gd/gd_utf8.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_printer2.h"


using namespace gd::log;
gd::log::logger<0>* plogger = gd::log::get_s();                                // get pointer to logger 0


TEST_CASE( "[logging] colors", "[logging]" ) {
   plogger->append( std::make_unique<gd::log::printer_console>( std::string_view( "CONSOLE" ) ));
   plogger->append( std::make_unique<gd::log::printer_console>( std::string_view( "CONSOLE2" ) ));
   plogger->set_severity(eSeverityNumberVerbose);

   gd::log::printer_console* pprinterconsole = (printer_console*)plogger->get( "CONSOLE" );
   pprinterconsole->set_margin( 10 );
   pprinterconsole->set_color( gd::log::printer_console::m_arrayColorDeGrey_s );

   auto callback_ = [](auto& message_, auto* plogger) -> void {
      const char* pbszMessage = message_.get_text();
      const char* pbszCpp = std::strstr( pbszMessage, ".cpp" );
      if( pbszCpp != nullptr )
      {
         while(pbszCpp > pbszMessage && *pbszCpp != '/' && *pbszCpp != '\\') { pbszCpp--; }
         if(pbszCpp != pbszMessage)
         {
            pbszCpp++;
            message_.set_text( pbszCpp );
         }
      }
      auto i = 0;
   };

   // plogger->callback_add( callback_ );


   LOG_FATAL("LOG_FATAL");
   LOG_ERROR("LOG_ERROR");
   LOG_WARNING("LOG_WARNING");
   LOG_INFORMATION("LOG_INFORMATION");
   LOG_DEBUG("LOG_DEBUG");
   LOG_VERBOSE("LOG_VERBOSE");
   LOG_NONE("LOG_NONE");

   LOG_FATAL("LOG_FATAL");
   LOG_ERROR("LOG_ERROR");
   LOG_WARNING("LOG_WARNING");
   LOG_INFORMATION("LOG_INFORMATION");
   LOG_DEBUG("LOG_DEBUG");
   LOG_VERBOSE("LOG_VERBOSE");
   LOG_NONE("LOG_NONE");

   {
      const char* ppbsz_[] = { "Hello", "World", "C++" };

      auto pair_ = std::pair<int,const char**>( 3, ppbsz_ );
      gd::log::ascii ascii_("1234567890");
      ascii_ += pair_;
      //message_.append( p_ );
      LOG_NONE( ascii_ );
      ascii_.clear();
      ascii_ += std::make_tuple( 3, ppbsz_, " " );
      LOG_NONE( ascii_ );
      LOG_NONE( gd::log::ascii( std::make_tuple( 3, ppbsz_, " " ) ) );

      LOG_ERROR( gd::log::make_ascii_g( "1", " ", "3", " ", "2", " ", true, 1, 3.5 ) );
      LOG_FATAL( gd::log::make_ascii_g( std::make_tuple( 3, ppbsz_, " " ) ) );
      LOG_FATAL( gd::log::make_ascii_g( "\n", std::make_pair( 100, '=' ), "\n") );
      LOG_FATAL( gd::log::make_ascii_g( std::string("test") ) );
      LOG_DEBUG( gd::log::ascii("1 2 3 4 5 6 7 8 9 0").keep( gd::log::ascii::eGroupDigit ) );
      LOG_DEBUG( gd::log::ascii().line( "=\n", 100 ) );
   }

   plogger->erase( std::string_view( "CONSOLE2" ) );
}

TEST_CASE( "[logging] hash tag", "[logging]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->tag_add("sql");
   LOG_FATAL( gd::log::tag("sql") << "SELECT * FROM Table");
   LOG_FATAL(gd::log::tag("xml") << "<document></document>");
   LOG_DEBUG2(gd::log::tag("sql"), "<document></document>");
   LOG_DEBUG_RAW2(gd::log::tag("sql"), "SELECT * FROM Table");

   plogger->tag_add("json");
   LOG_FATAL(gd::log::tag("json") << R"({
    "name": "John Doe",
    "age": 30,
    "city": "New York",
    "hobbies": ["reading", "coding", "gaming"],
    "address": {
        "street": "123 Main St",
        "city": "New York",
        "zip": "10001"
    },
    "isMarried": false,
    "pets": [
        {
            "name": "Buddy",
            "type": "dog",
            "age": 5
        },
        {
            "name": "Whiskers",
            "type": "cat",
            "age": 2
        }
    ]
})");

   plogger->set_flags( gd::log::eLoggerFlagOnlyTag, 0 );
   LOG_DEBUG("1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0");
   LOG_DEBUG("1" & "2" & "3" & "4" & "5" & "6" & "7" & "8" & "9" & "0");
   LOG_DEBUG("1" << "2" << "3" );
   LOG_DEBUG("1" << 100 );
   LOG_DEBUG("1" & 100 & 200 & 300 & 400 & "6" & "7" & "8" & "9" & "0");
   LOG_DEBUG("���01234567890");
}

TEST_CASE( "[logging] cvs logger", "[logging]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->clear();

   std::string stringFilePath = mainarguments_g.m_ppbszArgumentValue[0];
   auto position_ = stringFilePath.find_last_of("\\/");
   if( position_ != std::string::npos ) { stringFilePath = stringFilePath.substr( 0, position_ + 1 ); }

   // ## add date and time
   std::string stringDate = gd::file::rotate::backup_history::date_now_s();    // date value
   stringFilePath += stringDate;
   stringFilePath += "_";
   stringDate = gd::file::rotate::backup_history::time_now_s( gd::file::rotate::tag_filename{} );// time value
   stringFilePath += stringDate;
   stringFilePath += ".csv";

   plogger->append( std::make_unique<gd::log::printer_csvfile>( std::string_view( "CSV" ), stringFilePath ));
   gd::log::printer_csvfile* pprinter = (gd::log::printer_csvfile*)plogger->get( "CSV" );
   pprinter->set_flags( printer_csvfile::flags_s("+benchmark +benchmark-text") );

   LOG_DEBUG_RAW("DEBUG");
   LOG_INFORMATION_RAW("INFORMATION");

   std::vector<int> vectorInteger = {1000, 2000, 3000, 4000, 5000, 6000, 7000};
   for( auto it : vectorInteger ) { LOG_NONE_RAW( "Number =" << it ); }

   LOG_DEBUG_RAW("LOG_DEBUG with time");
   std::this_thread::sleep_for(std::chrono::seconds(1));
   LOG_DEBUG_RAW("LOG_DEBUG one seconds later");
}

TEST_CASE( "[logging] hashtag logging", "[logging]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->clear();
   plogger->append( std::make_unique<gd::log::printer_console>( std::string_view( "CONSOLE" ) ));

   std::vector<std::string> vectorTag = { "one", "two", "three", "four", "five", "six", "seven", "eight" };

   LOG_VERBOSE_RAW( gd::log::ascii().line( "=\n", 100 ) << "Print log message for added tag");
   for( auto it : vectorTag )
   {
      plogger->tag_add( it );
      LOG_DEBUG2( gd::log::tag("one"), "Print 1!");
      LOG_DEBUG2( gd::log::tag("two"), "Print 2!");
      LOG_DEBUG2( gd::log::tag("three"), "Print 3!");
      LOG_DEBUG2( gd::log::tag("four"), "Print 4!");
      LOG_DEBUG2( gd::log::tag("five"), "Print 5!");
      LOG_DEBUG2( gd::log::tag("six"), "Print 6!");
      LOG_DEBUG2( gd::log::tag("seven"), "Print 7!");
      LOG_DEBUG2( gd::log::tag("eight"), "Print 8!");
      plogger->tag_erase( it );
   }
   
   auto vectorTag3 = vectorTag | std::views::take(3);
   LOG_VERBOSE_RAW( gd::log::ascii().line( "=\n", 100 ) << "Print log messages for tags added, prints 6 message");
   for( auto it : vectorTag3 )
   {
      plogger->tag_add( it );
      LOG_DEBUG2( gd::log::tag("one"), "Print 1!");
      LOG_DEBUG2( gd::log::tag("two"), "Print 2!");
      LOG_DEBUG2( gd::log::tag("three"), "Print 3!");
   }
}

TEST_CASE( "[logging] extra columns", "[logging]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->clear();

   std::string stringFilePath = mainarguments_g.m_ppbszArgumentValue[0];
   auto position_ = stringFilePath.find_last_of("\\/");
   if( position_ != std::string::npos ) { stringFilePath = stringFilePath.substr( 0, position_ + 1 ); }

   // ## add date and time
   std::string stringDate = gd::file::rotate::backup_history::date_now_s();    // date value
   stringFilePath += stringDate;
   stringFilePath += "_2";
   stringDate = gd::file::rotate::backup_history::time_now_s( gd::file::rotate::tag_filename{} );// time value
   stringFilePath += stringDate;
   stringFilePath += ".csv";

   plogger->append( std::make_unique<gd::log::printer_csvfile>( std::string_view( "CSV" ), stringFilePath ));
   gd::log::printer_csvfile* pprinter = (gd::log::printer_csvfile*)plogger->get( "CSV" );
   pprinter->create([](auto& table_) {
      table_.column_add("uint64", 0, "rows" );
   });

   LOG_DEBUG_RAW("DEBUG, testing writing number to column?rows=1");
}
