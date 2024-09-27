## GD (General development)

Code in the gd namespace can't be dependent of anything external, only core C++ is needed to use gd code.
There are some database objects that is turned on based on macros but the rest is default c++.

**Note: The file `gd_types.h` in this folder is very very important**
Almost all other code in the gd folder is based on the type system found in `gd_types.h`

### Main files
`gd_types.h` - the type system for gd is found here, constants etc  
`gd_variant.h` and `gd_variant_view.h` - these are the main value objects that is used to store values  
`gd_arguments.h` - Stores values as named arguments, each value can have an name. 


| File | Description |
| - | - |
| `gd_arguments` | Named arguments object. Values are stored with name in a byte array (cache friendly)  |
| `gd_sql_value` | Format C++ string for use in SQL statements, methods like append, replace and more to manipulate strings |
| `gd_table_column-buffer` | Table object optimized for use as dto object, transfer table data or work with table data on the fly and then delete it |
| `gd_table_table` | Table object optimized to store member data for some C++ object  |
| `gd_table_io` | Read and Write information to and from table objects in different formats, like console, csv, sql and more |
| `gd_variant` | Variant object are able to store any C++ primitive value and some common extended data types, variant holds meta data about value and can be used to generalise logic |
| `gd_types` | GD core type constants, all code in GD depends on same type system that is defined there. <br> `gd_types` also contains name and methods to compare type names, could also be used in other situations because they are highly optimized. |
