#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

// - command History
//    - save command arguments and date
//       - save to file
//       - save all commands that excecuted or failed
//       - save command same format as input
//    - history size
//       - clear history
//       - limit size of history
//    - print out history
//       - print out history with date
//       - print out history with command
//       - print out history with command and date
//    - return history

