## GD (General development)

Code in the gd namespace can't be dependent of anything external, only core C++ is needed to use gd code.
There are some database objects that is turned on based on macros but the rest is default c++.

**Note**: The file `gd_types.h` in this folder is very very important
Almost all other code in the gd folder is based on the type system found in `gd_types.h`

## Core objects
gd have three main objects and they are variant, arguments and table. These three objects exists in 
different forms and are used to store values in different ways.
- **variant** - is used to store any value, it is a general value object
- **arguments** - is used to store multiple named/unamed values in one single buffer.
- **table** - is used to store values in a table format like rows and columns.

There is different versions of these objects, optimized for speed, memory and flexibility.
All objects within these three categories are very similar from development perspective, 
they have similar methods but are optimized for different use cases. If you know how to use one of them, 
you know the rest.

### Main files
`gd_types.h` - the type system for gd is found here, constants etc  
`gd_variant.h` and `gd_variant_view.h` - these are the main value objects that is used to store values  
`gd_arguments.h` - Stores values as named arguments, each value can have an name. 


| File | Description |
| - | - |
| `gd_arguments` | Named arguments object. Values are stored with name in a byte array (cache friendly)  |
| `gd_arguments_shared` | Similar to `gd_arguments` with more speed and are able to store more data, less limitations compared to `gd_arguments` |
| `gd_cli_options` | Configure arguments passed to applications in command line  |
| `gd_com` | Logic similar to MS COM (Component Object Model) technology  |
| `gd_database_odbc` | Connect and work with databases using odbc, most RDMS have odbc drivers |
| `gd_database_sqlite` | Connect and work with sqlite |
| `gd_log_logger` | Flexible logger to generate information in code |
| `gd_log_logger_define` | Macros to compile internal logger functionality or remove it from binary |
| `gd_log_logger_printer` | Default printers used in logger, easy to write your own and by looking at the default printers you can see how it is done |
| `gd_log_logger_printer2` | Specialized printers with some sort of functionality to log information |
| `gd_parse` | Parse logic, for use to read csv, uri, json, sql etc   |
| `gd_sql_query` | Build sql queries |
| `gd_sql_value` | Format C++ string for use in SQL statements, methods like append, replace and more to manipulate strings |
| `gd_table_column-buffer` | Table object optimized for use as dto object, transfer table data or work with table data on the fly and then delete it |
| `gd_table_table` | Table object optimized to store member data for some C++ object  |
| `gd_table_io` | Read and Write information to and from table objects in different formats, like console, csv, sql and more |
| `gd_types` | GD core type constants, all code in GD depends on same type system that is defined there. <br> `gd_types` also contains name and methods to compare type names, could also be used in other situations because they are highly optimized. |
| `gd_variant` | Variant object are able to store any C++ primitive value and some common extended data types, variant holds meta data about value and can be used to generalise logic |
| `gd_utf8` | String operations, there are a lot of operations working with strings, focus on utf8 formats but should work on almost all utf8 and ascii strings |
| `gd_uuid` | Code to work with uuid values |
| `io/gd_io_archive_stream` | Read and write objects, suitable for serialization and deserialization  |
| `io/gd_io_repository_stream` | Read and write information to and from streams, like files, memory, network etc and store multiple streams in a repository |


```cmakefile
# Define the base path for gd library
set(GD_BASE_PATH ${CMAKE_SOURCE_DIR}/external/gd)

# Collect all .cpp and .h files recursively and by component
file(GLOB_RECURSE GD_SOURCES_ALL ${GD_BASE_PATH}/*.cpp ${GD_BASE_PATH}/*.h)
file(GLOB GD_SOURCES_ROOT ${GD_BASE_PATH}/*.cpp ${GD_BASE_PATH}/*.h)
file(GLOB GD_SOURCES_COM ${GD_BASE_PATH}/com/*.cpp ${GD_BASE_PATH}/com/*.h)
file(GLOB GD_SOURCES_CONSOLE ${GD_BASE_PATH}/console/*.cpp ${GD_BASE_PATH}/console/*.h)
file(GLOB GD_SOURCES_DATABASE ${GD_BASE_PATH}/database/*.cpp ${GD_BASE_PATH}/database/*.h)
file(GLOB GD_SOURCES_EXPRESSION ${GD_BASE_PATH}/expression/*.cpp ${GD_BASE_PATH}/expression/*.h)
file(GLOB GD_SOURCES_IO ${GD_BASE_PATH}/io/*.cpp ${GD_BASE_PATH}/io/*.h)
file(GLOB GD_SOURCES_PARSE ${GD_BASE_PATH}/parse/*.cpp ${GD_BASE_PATH}/parse/*.h)

# Store component source lists in a variable for easy iteration
set(GD_LIBRARY_COMPONENTS "ALL;ROOT;COM;CONSOLE;DATABASE;EXPRESSION;IO;PARSE")
foreach(component IN LISTS GD_LIBRARY_COMPONENTS)
    set(GD_COMPONENT_${component}_SOURCES ${GD_SOURCES_${component}})
endforeach()

# Exclude non-source files from the root and all sources
list(FILTER GD_SOURCES_ROOT EXCLUDE REGEX "LICENSE\\.txt|README\\.MD")
list(FILTER GD_SOURCES_ALL EXCLUDE REGEX "LICENSE\\.txt|README\\.MD")

# Debug output to verify the variables
message(STATUS "GD_BASE_PATH: ${GD_BASE_PATH}")
message(STATUS "GD_SOURCES_ALL contains ${GD_SOURCES_ALL}")
message(STATUS "GD_SOURCES_ROOT contains ${GD_SOURCES_ROOT}")
message(STATUS "GD_SOURCES_COM contains ${GD_SOURCES_COM}")
message(STATUS "GD_SOURCES_CONSOLE contains ${GD_SOURCES_CONSOLE}")
message(STATUS "GD_SOURCES_DATABASE contains ${GD_SOURCES_DATABASE}")
message(STATUS "GD_SOURCES_EXPRESSION contains ${GD_SOURCES_EXPRESSION}")
message(STATUS "GD_SOURCES_IO contains ${GD_SOURCES_IO}")
message(STATUS "GD_SOURCES_PARSE contains ${GD_SOURCES_PARSE}")
```

     

```
│   gd_arguments.cpp
│   gd_arguments.h
│   gd_arguments_common.h
│   gd_arguments_shared.cpp
│   gd_arguments_shared.h
│   gd_cli_options.cpp
│   gd_cli_options.h
│   gd_com.h
│   gd_compiler.h
│   gd_database.h
│   gd_database_odbc.cpp
│   gd_database_odbc.h
│   gd_database_record.cpp
│   gd_database_record.h
│   gd_database_sqlite.cpp
│   gd_database_sqlite.h
│   gd_database_types.h
│   gd_debug.cpp
│   gd_debug.h
│   gd_expression_token.cpp
│   gd_expression_token.h
│   gd_file.cpp
│   gd_file.h
│   gd_file_rotate.cpp
│   gd_file_rotate.h
│   gd_log_logger.cpp
│   gd_log_logger.h
│   gd_log_logger_define.h
│   gd_log_logger_printer.cpp
│   gd_log_logger_printer.h
│   gd_log_logger_printer2.cpp
│   gd_log_logger_printer2.h
│   gd_math.h
│   gd_parse.cpp
│   gd_parse.h
│   gd_sql_query.cpp
│   gd_sql_query.h
│   gd_sql_value.cpp
│   gd_sql_value.h
│   gd_sql_values.h
│   gd_strings.cpp
│   gd_strings.h
│   gd_table.cpp
│   gd_table.h
│   gd_table_aggregate.h
│   gd_table_arguments.cpp
│   gd_table_arguments.h
│   gd_table_column-buffer.cpp
│   gd_table_column-buffer.h
│   gd_table_column.cpp
│   gd_table_column.h
│   gd_table_index.cpp
│   gd_table_index.h
│   gd_table_io.cpp
│   gd_table_io.h
│   gd_table_table.cpp
│   gd_table_table.h
│   gd_translate.cpp
│   gd_translate.h
│   gd_types.cpp
│   gd_types.h
│   gd_utf8.cpp
│   gd_utf8.h
│   gd_utf8.hpp
│   gd_utf8_2.cpp
│   gd_utf8_2.h
│   gd_utf8_string.cpp
│   gd_utf8_string.h
│   gd_uuid.h
│   gd_variant.cpp
│   gd_variant.h
│   gd_variant_common.h
│   gd_variant_view.cpp
│   gd_variant_view.h
│   LICENSE.txt
│   README.MD
│
├───com
│       gd_com_server.cpp
│       gd_com_server.h
│
├───console
│       gd_console_console.cpp
│       gd_console_console.h
│       gd_console_print.cpp
│       gd_console_print.h
│       gd_console_style.h
│
├───database
│       gd_database_io.cpp
│       gd_database_io.h
│
├───expression
│       gd_expression.cpp
│       gd_expression.h
│       gd_expression_method_01.cpp
│       gd_expression_method_01.h
│       gd_expression_operator.h
│       gd_expression_parse_state.cpp
│       gd_expression_parse_state.h
│       gd_expression_runtime.cpp
│       gd_expression_runtime.h
│       gd_expression_token.cpp
│       gd_expression_token.h
│       gd_expression_value.cpp
│       gd_expression_value.h
│       README.MD
│
├───io
│       gd_io_archive.h
│       gd_io_archive_memory.h
│       gd_io_archive_stream.cpp
│       gd_io_archive_stream.h
│       gd_io_keyboard.cpp
│       gd_io_keyboard.h
│       gd_io_repository_stream.cpp
│       gd_io_repository_stream.h
│
└───parse
        gd_parse_match_pattern.cpp
        gd_parse_match_pattern.h
        gd_parse_window_line.cpp
        gd_parse_window_line.h
```
