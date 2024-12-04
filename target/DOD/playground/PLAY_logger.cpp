#include <random>
#include <chrono>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_log_logger_printer.h"


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

}