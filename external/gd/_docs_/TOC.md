# **GD code** Documentation in brief format

GD (general development) are code that are used to work with primitive and common derived values.
Only dependencies are C++20 and the standard library.

## Core Documentation

*Core constants for gd type system, enables components to "talk" to each other.*

- [gd Type System](GD_TYPES.MD)
- [Compiler](GD_COMPILER.MD)

## Variant Documentation

*Variants are a powerful data type that can hold any value, including other variants. They are used extensively in the gd type system.*

- [Variant](GD_VARIANT.MD)
- [Variant Arguments](GD_VARIANT_ARG.MD)
- [Variant View](GD_VARIANT_VIEW.MD)

## Arguments Documentation

*Pack primitive and common derived values into a byte buffer for efficient storage and access with focus on memory size. Like key-value pairs in one single buffer.*

- [Arguments](GD_ARGUMENTS.MD)
- [Shared Arguments](GD_ARGUMENTS_SHARED.MD)

## Table Documentation

*High performance table operations.*

- [Table Arguments](GD_TABLE_ARGUMENTS.MD)
- [Table DTO](GD_TABLE_DTO.MD)
- [Table Index](GD_TABLE_INDEX.MD)
- [Table I/O](GD_TABLE_IO.MD)
- [Table Table](GD_TABLE_TABLE.MD)

## Command Line Documentation

*Command line logic for terminal applications.*

- [CLI Options](GD_CLI_OPTIONS.MD)

## Database Documentation

*Database operations and management. Work with any RDBMS that is compatible with the ODBC API or Sqlite*

- [Database ODBC](GD_DATABASE_ODBC.MD)
- [Database SQLite](GD_DATABASE_SQLITE.MD)

## File Operations

*Miscellaneous file operations that work on any OS*

- [File](GD_FILE.MD)

## Logger and log Operations

*Add logging and log operations for applications.*

- [Logger](GD_LOG_LOGGER.MD)
- [Log](GD_LOG_LOGGER_DEFINE.MD)

## UTF8 Documentation

*Miscellaneous UTF8 operations. Work with any UTF8 encoded string.*

- [UTF8 Support](GD_UTF8.MD)
