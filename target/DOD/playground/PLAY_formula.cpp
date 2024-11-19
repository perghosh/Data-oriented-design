#include <random>
#include <chrono>

#include "gd/gd_types.h"
#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_log_logger_printer.h"

enum enumTokenType {
   eKeyword,
   eIdentifier,
   eLiteral,
   eOperator,
   eSeparator,
   eComment
};

std::pair<unsigned, std::string_view> read_token(const char* pbszToken, const char* pbszEnd)
{
   return { 0, "" };
}

using namespace gd::log;
gd::log::logger<0>* plogger = gd::log::get_s();                                // get pointer to logger 0

std::pair<bool, std::string> convert_to_token(const char* pbszBegin, const char* pbszEnd)
{
   using namespace gd::types;
   const char* pbszPosition = pbszBegin;
   while(pbszPosition < pbszEnd)
   {
      auto uType = get_ctype_g( *pbszPosition );
      if( uType & ctype_g( "space", tag_ask_compiler{})) { pbszPosition++; continue; }
      // read token
   }

   return { true, "" };
}


TEST_CASE( "[logging] colors", "[logging]" ) {
   plogger->append( std::make_unique<gd::log::printer_console>( "console" ));
   plogger->set_severity(eSeverityNumberVerbose);

   LOG_INFORMATION("- Convert string into tokens");
   LOG_INFORMATION("  - Tokens are: Literal (value), Identifier (variable), Opderator, Keyword, Separator");

   LOG_FATAL("LOG_FATAL");
   LOG_ERROR("LOG_ERROR");
   LOG_WARNING("LOG_WARNING");
   LOG_INFORMATION("LOG_INFORMATION");
   LOG_DEBUG("LOG_DEBUG");
   LOG_VERBOSE("LOG_VERBOSE");
   LOG_NONE("LOG_NONE");
}