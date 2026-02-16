# **GD code** Documentation in brief format

GD (general development) are code that are used to work with primitive and common derived values.
Only dependencies are C++20 and the standard library.

Why GD code?
GD code is designed to provide **runtime** support for efficient and flexible data handling and make code safer. Code checks it's correctness at runtime, ensuring that data is handled correctly and safely.

## Core dependency for gd code

Core constants for gd type system, enables components to "talk" to each other.

- [gd Type System](GD_TYPES.MD) - *Core type system with unique identifiers and metadata for component communication*
- [Compiler](GD_COMPILER.MD) - *Lightweight C++ standard and compiler detection utilities*

## Variant Documentation

Variants are a powerful data type that can hold any value, including other variants. They are used extensively in the gd type system.

- [Variant](GD_VARIANT.MD) - *Flexible type-safe container for any primitive or common dynamic value*
- [Variant Arguments](GD_VARIANT_ARG.MD) - *Named argument containers for flexible key-value parameter handling*
- [Variant View](GD_VARIANT_VIEW.MD) - *Non-owning lightweight view into variant data for zero-copy access*

## Arguments Documentation

Pack primitive and common derived values into a byte buffer for efficient storage and access with focus on memory size. Like key-value pairs in one single buffer.

- [Arguments](GD_ARGUMENTS.MD) - *Compact, stack-friendly key-value buffer with minimal overhead*
- [Shared Arguments](GD_ARGUMENTS_SHARED.MD) - *Reference-counted argument buffer with copy-on-write semantics*

## Table Documentation

High performance table operations.

- [Table Arguments](GD_TABLE_ARGUMENTS.MD) - *Dynamic columnar table with per-row extra fields support*
- [Table DTO](GD_TABLE_DTO.MD) - *High-performance contiguous columnar data transfer object*
- [Table Index](GD_TABLE_INDEX.MD) - *Fast lookup indexes for tables with binary search capability*
- [Table I/O](GD_TABLE_IO.MD) - *Table output formatting for CSV, JSON, SQL, CLI, and custom formats*
- [Table Table](GD_TABLE_TABLE.MD) - *Optimized member table for long-lived objects with shared columns*

## Optimized STL dropin replacements

Objects that are optimized for performance and memory usage that mimic the standard library containers.

- [Arena](GD_ARENA.MD) - *High-performance memory allocator*
- [Arena with borrow](GD_ARENA_BORROW.MD) - *High-performance memory allocator with borrow support*
- [Optimized Vector](GD_VECTOR.MD) - *Optimized vector variants*

## Command Line Documentation

Command line logic for terminal applications.

- [CLI Options](GD_CLI_OPTIONS.MD) - *Comprehensive command-line interface parser with subcommand support*

## Database Documentation

Database operations and management. Work with any RDBMS that is compatible with the ODBC API or Sqlite

- [Format SQL](GD_SQL_VALUE.MD) - *Safe SQL Value Formatting and Templating*
- [Database ODBC](GD_DATABASE_ODBC.MD) - *Modern ODBC wrapper for universal database connectivity*
- [Database SQLite](GD_DATABASE_SQLITE.MD) - *Simple and safe SQLite wrapper with modern C++ interface*

## File Operations

Miscellaneous file operations that work on any OS

- [File](GD_FILE.MD) - *Cross-platform file and path utilities with enhanced functionality*

## Logger and log Operations

Add logging and log operations for applications.

- [Logger](GD_LOG_LOGGER.MD) - *Flexible, thread-safe logging framework with multiple outputs*
- [Log](GD_LOG_LOGGER_DEFINE.MD) - *Convenience macros and definitions for simplified logging operations*

## UTF8 Documentation

Miscellaneous UTF8 operations. Work with any UTF8 encoded string.

- [UTF8 Support](GD_UTF8.MD) - *High-performance UTF-8 string utilities for modern C++*
