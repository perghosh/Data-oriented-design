#include "../Application.h"

#include <iostream>

#include "main.h"

#include "../Command.h"

#include "string.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[string] test", "[string]")
{
   string stringText("Bye");
   stringText.append("Hello", 5);

   stringText += " Good";

   std::cout << stringText.c_str() << std::endl;;
}