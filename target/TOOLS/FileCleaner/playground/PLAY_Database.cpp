#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#include "gd/gd_arguments_common.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[database] test database in cleaner", "[database]" ) {
   std::string stringFolder = FOLDER_GetRoot_g( "test__" );
   gd::argument::arguments arguments_;
   arguments_.append("test-folder", stringFolder);
   arguments_.append("database-file", "test.db");
   std::cout << arguments_.print( "Folder where database are places is {} and database file is: {}\n" );
}


TEST_CASE( "[database] test database in cleane r", "[database]" ) {
   std::string stringFolder = FOLDER_GetRoot_g( "test__" );
   gd::argument::shared::arguments arguments_;
   arguments_.append("test-folder", stringFolder);
   arguments_.append("database-file", "test.db");
   std::cout << arguments_.print( "Folder where database are places is {} and database file is: {}\n" );
}
