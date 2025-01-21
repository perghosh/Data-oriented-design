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
gd::log::logger<0>* plogger = gd::log::get_s();

TEST_CASE("[Logging] Log", "[Logging]")
{
   plogger->append(std::make_unique<gd::log::printer_console>(std::string_view("CONSOLE")));
   plogger->set_severity(eSeverityNumberVerbose);

   gd::log::printer_console* pprinterconsole = (printer_console*)plogger->get("CONSOLE");
   pprinterconsole->set_margin(8);
   pprinterconsole->set_color(gd::log::printer_console::m_arrayColorDeGrey_s);

   //REQUIRE(false);
   LOG_DEBUG(1 & 2 & 3);
   LOG_INFORMATION("test");
   LOG_FATAL("101010101010101" << 100 << 200 << 300 );
   bool bError = true;
   LOG_FATAL_RAW_IF(bError == false, "101010101010101" << 100 << 200 << 300);
}

TEST_CASE("[Logging] Log2", "[Logging]")
{
#ifdef GD_LOG_SIMPLE
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->tag_add("kevin");
   plogger->tag_add("per");
   //plogger->set_flags(gd::log::enumLoggerFlag::eLoggerFlagNoTagFilter, 0);
#endif
   LOG_INFORMATION(gd::log::tag("kevin") << "Hej, jag heter kevin");
   LOG_INFORMATION(gd::log::tag("per") << "Nu har jag hejjat pa dig");
   LOG_INFORMATION("XXXXXXXXXXXXXXXXXXXXX");

}
