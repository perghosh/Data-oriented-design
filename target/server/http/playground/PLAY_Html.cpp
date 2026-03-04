// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd_tools/html/gd_tools_html_document.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[html] element creation", "[html]" ) {
   using namespace gd::tools::html;

   element elementRoot( "html" );
   elementRoot.add_attribute( "lang", "en" );
   elementRoot.add_attribute( "lang", "SWE" );

}
