#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"

#include "main.h"

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[file] load file into document", "[file]" ) {
   CApplication application;

   for( auto i = 0; i < 10; i++ ) 
   {
      auto stringName = GENERATE_RandomName( 10 + i );
      application.DOCUMENT_Add( stringName );
   }

   for( auto it = application.DOCUMENT_Begin(); it != application.DOCUMENT_End(); it++ )
   {
      std::cout << "Document: " << ( *it )->GetName() << std::endl;
   }

   application.DOCUMENT_Clear();

}