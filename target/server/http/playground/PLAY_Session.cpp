// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <psapi.h>
#endif

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[session] test uri logic", "[session]") 
{
   CSessions sessions;
   sessions.Initialize(20000);
}