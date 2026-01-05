# **GD code** Documentation in brief format

GD (general development) are code that are used to work with primitive and common derived values.
Only dependencies are C++20 and the standard library.

## Core Documentation

*Core constants for gd type system, enables components to "talk" to each other.*

- [gd Type System](GD_TYPES.MD) - <span style="color: #808080;">Core type system with unique identifiers and metadata for component communication</span>
- [Compiler](GD_COMPILER.MD) - <span style="color: #808080;">Lightweight C++ standard and compiler detection utilities</span>

## Variant Documentation

*Variants are a powerful data type that can hold any value, including other variants. They are used extensively in the gd type system.*

- [Variant](GD_VARIANT.MD) - <span style="color: #808080;">Flexible type-safe container for any primitive or common dynamic value</span>
- [Variant Arguments](GD_VARIANT_ARG.MD) - <span style="color: #808080;">Named argument containers for flexible key-value parameter handling</span>
- [Variant View](GD_VARIANT_VIEW.MD) - <span style="color: #808080;">Non-owning lightweight view into variant data for zero-copy access</span>

## Arguments Documentation

*Pack primitive and common derived values into a byte buffer for efficient storage and access with focus on memory size. Like key-value pairs in one single buffer.*

- [Arguments](GD_ARGUMENTS.MD) - <span style="color: #808080;">Ultra-compact, stack-friendly key-value buffer with minimal overhead</span>
- [Shared Arguments](GD_ARGUMENTS_SHARED.MD) - <span style="color: #808080;">Reference-counted argument buffer with copy-on-write semantics</span>

## Table Documentation

*High performance table operations.*

- [Table Arguments](GD_TABLE_ARGUMENTS.MD) - <span style="color: #808080;">Dynamic columnar table with per-row extra fields support</span>
- [Table DTO](GD_TABLE_DTO.MD) - <span style="color: #808080;">High-performance contiguous columnar data transfer object</span>
- [Table Index](GD_TABLE_INDEX.MD) - <span style="color: #808080;">Fast lookup indexes for tables with binary search capability</span>
- [Table I/O](GD_TABLE_IO.MD) - <span style="color: #808080;">Table output formatting for CSV, JSON, SQL, CLI, and custom formats</span>
- [Table Table](GD_TABLE_TABLE.MD) - <span style="color: #808080;">Optimized member table for long-lived objects with shared columns</span>

## Command Line Documentation

*Command line logic for terminal applications.*

- [CLI Options](GD_CLI_OPTIONS.MD) - <span style="color: #808080;">Comprehensive command-line interface parser with subcommand support</span>

## Database Documentation

*Database operations and management. Work with any RDBMS that is compatible with the ODBC API or Sqlite*

- [Database ODBC](GD_DATABASE_ODBC.MD) - <span style="color: #808080;">Modern ODBC wrapper for universal database connectivity</span>
- [Database SQLite](GD_DATABASE_SQLITE.MD) - <span style="color: #808080;">Simple and safe SQLite wrapper with modern C++ interface</span>

## File Operations

*Miscellaneous file operations that work on any OS*

- [File](GD_FILE.MD) - <span style="color: #808080;">Cross-platform file and path utilities with enhanced functionality</span>

## Logger and log Operations

*Add logging and log operations for applications.*

- [Logger](GD_LOG_LOGGER.MD) - <span style="color: #808080;">Flexible, thread-safe logging framework with multiple outputs</span>
- [Log](GD_LOG_LOGGER_DEFINE.MD) - <span style="color: #808080;">Convenience macros and definitions for simplified logging operations</span>

## UTF8 Documentation

*Miscellaneous UTF8 operations. Work with any UTF8 encoded string.*

- [UTF8 Support](GD_UTF8.MD) - <span style="color: #808080;">High-performance UTF-8 string utilities for modern C++</span>
