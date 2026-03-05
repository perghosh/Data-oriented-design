// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd/gd_file.h"
#include "gd_tools/html/gd_tools_html_document.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[html] element creation", "[html]" ) {
   using namespace gd::tools::html;

   element elementRoot( "html" );
   elementRoot.add_attribute( "lang", "en" );
   elementRoot.add_attribute( "lang", "SWE" );

}


TEST_CASE( "[html] load", "[html]" ) {
   using namespace gd::tools::html;

   std::filesystem::path pathCurrentDirecotry = std::filesystem::current_path();
   auto [bFound, stringRootFolder] = gd::file::closest_having_file_g( pathCurrentDirecotry.string(), ROOT_MARKER ); REQUIRE( bFound == true );

  
   std::string stringFileName( stringRootFolder );

   stringFileName += "resource/page_user_edit.html";
   gd::file::path pathFile( stringFileName );

   // Load the HTML file into a string
   std::string stringHtml;
   std::string stringError;
   std::tie( bFound, stringError ) = gd::file::read_file_g( pathFile, stringHtml ); REQUIRE( bFound == true );

   // ## Parse the HTML string into a document object
   parser parser_;
   document documentHtml = parser_.parse( stringHtml );

   auto uCount =  documentHtml.root()->size_all();

   // ## walk elments
   documentHtml.root()->walk( []( const element& elementCurrent ) -> bool
   {
      std::cout << "Element: " << elementCurrent.name() << ", content: " << elementCurrent.content() << "\n";
      for( const auto& [name_, value_] : elementCurrent.attributes() )
      {
         std::cout << "  Attribute: " << name_ << " = " << value_.as_string() << "\n";
      }
      return true;
   } );

}
