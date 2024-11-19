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

}