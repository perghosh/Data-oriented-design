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
