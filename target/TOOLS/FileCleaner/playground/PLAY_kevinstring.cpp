#include "../Application.h"

#include <iostream>

#include "main.h"

#include "../Command.h"

#include "string.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[string] test", "[string]")
{
   /*string stringText("Bye");
   stringText.append("Hello", 5);

   stringText += " Good";*/

   //string stringText2("Hello Good");
   //string stringSubString = stringText2.substr(5, 3);

   string stringTestText("Hello World");
   
   stringTestText.insert(6, "C++ ");

   stringTestText.assign("Bye");
   stringTestText.replace(2, "hello", 5);
   stringTestText += "World";

   stringTestText = "Tree";

   std::cout << stringTestText.c_str() << std::endl;;
}