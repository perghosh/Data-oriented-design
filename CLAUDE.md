# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

CMake with Ninja generator, C++20/23, using presets defined in `CMakePresets.json`.

```bash
# Configure (Linux)
cmake --preset linux-debug

# Build all targets
cmake --build out/build/linux-debug

# Build a specific target
cmake --build out/build/linux-debug --target cleaner
cmake --build out/build/linux-debug --target TEST_Arguments
```

Build output goes to `out/build/<preset-name>/`. Available presets: `linux-debug`, `x64-debug`, `x64-clang-debug`, `macos-debug`.

External sources are collected via `cmake/include_external.cmake` into CMake variables (`GD_SOURCES_ALL`, `external_catch2`, `external_sqlite`, `external_pugixml`, etc.) that targets reference directly — no separate build for external libraries.

## Running Tests

Tests use [Catch2](https://github.com/catchorg/Catch2) (amalgamated, in `external/catch2/`). Each test target is a standalone executable:

```bash
# Run a specific test executable
./out/build/linux-debug/test/TEST_GD
./out/build/linux-debug/target/TOOLS/FileCleaner/tests/TEST_Arguments
./out/build/linux-debug/target/TOOLS/FileCleaner/tests/TEST_Expression

# Run with Catch2 filter
./out/build/linux-debug/target/TOOLS/FileCleaner/tests/TEST_File "[section-tag]"
```

Test targets in `test/`: `TEST_GD` (core gd library tests).
Test targets in `target/TOOLS/FileCleaner/tests/`: `TEST_Arguments`, `TEST_Expression`, `TEST_File`, `TEST_File2`, `TEST_Repository`, `TEST_History`.

## Project Structure

```
external/gd/          # GD (General Development) library — the primary shared library
external/catch2/      # Test framework
external/pugixml/     # XML
external/sqlite/      # SQLite
external/boost/       # Boost safe_numerics
external/jsoncons/    # JSON
source/application/   # Reusable application-level code (ApplicationBasic, database metadata)
cmake/                # CMake helper scripts (include_external.cmake)
target/TOOLS/FileCleaner/  # "cleaner" CLI tool — file organization/searching
target/TOOLS/Backup/       # Backup tool
target/server/http/        # HTTP server
misc/howto/                # HOWTO example executables
test/                      # General gd library tests
```

## The GD Library (`external/gd/`)

GD (General Development) is the core internal library — header+implementation pairs, C++20, no external dependencies beyond STL. Full TOC: `external/gd/_docs_/TOC.md`.

### Variant / Value types
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_variant.h` | `gd` | `variant` — owning type-safe value (any primitive + common derived) |
| `gd_variant_view.h` | `gd` | `variant_view` — non-owning view into variant data |
| `gd_variant_arg.h` | `gd` | `arg`, `args_view`, `args` — named key-value pairs using variants |

### Arguments (compact key-value buffers)
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_arguments.h` | `gd::argument` | `arguments` — memory-compact key-value byte buffer |
| `gd_arguments_shared.h` | `gd::argument::shared` | `arguments` — speed-optimized variant with ref-count / COW |
| `gd_arguments_io.h` | `gd::argument` | Serialize arguments to JSON, URI, YAML |

### Tables
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_table.h` | `gd::table` | Core primitives: `cell`, `column`, `row`, `rows`, `range`, `page` |
| `gd_table_table.h` | `gd::table` | `table` — fixed-column table, thread-safe shared column metadata |
| `gd_table_arguments.h` | `gd::table::arguments` | `table` — table where each row can have extra dynamic columns |
| `gd_table_column.h` | `gd::table::detail` | `column` DTO — type, size, name, alias metadata |
| `gd_table_index.h` | `gd::table` | `index_int64` — fast binary-search index over a table column |
| `gd_table_io.h` | `gd::table` | Stream tables as CSV, JSON, SQL, CLI; tag dispatchers |
| `table/gd_table_formater.h` | `gd::table` | Additional table formatting helpers |

### SQL
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_sql_query.h` | `gd::sql` | `query` with nested `table`, `field`, `condition` — builds SELECT/FROM/WHERE |
| `gd_sql_query_builder.h` | `gd::sql` | Fluent builders: `table_g`, `field_g`, `condition_g`, `fields_g` |
| `gd_sql_value.h` | `gd::sql` | Value escaping/formatting functions for safe SQL string construction |
| `gd_sql_types.h` | `gd::sql` | SQL type constants |

### Database
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_database_odbc.h` | `gd::database::odbc` | ODBC wrapper — `database`, `cursor` for cross-platform DB access |
| `gd_database_sqlite.h` | `gd::database::sqlite` | SQLite wrapper — `database`, `cursor`, `database_i`, `cursor_i` |
| `database/gd_database_io.h` | `gd::database` | I/O helpers for database operations |

### CLI & Console
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_cli_options.h` | `gd::cli` | `options` — full CLI parser with flags, subcommands, hierarchical args |
| `console/gd_console_print.h` | `gd::console` | ANSI color/formatting output; tag dispatchers for terminal control |

### File & I/O
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_file.h` | `gd::file` | `path` — cross-platform file read/write/delete/directory listing |
| `gd_file_rotate.h` | `gd::file` | Log file rotation utilities |
| `io/gd_io_archive.h` | `gd::io` | Tag dispatchers: `tag_io_read`, `tag_io_write` |
| `io/gd_io_repository_stream.h` | `gd::io::stream` | `repository` — file-based repository with entry add/read/list/remove |

### Logging
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_log_logger.h` | `gd::log` | `logger` (singleton), `message`, `i_printer` — extensible log framework |
| `gd_log_logger_define.h` | `gd::log` | Macros: `LOG`, `LOG_`, `LOG_IF`, `LOG_RAW` with severity variants |

### Strings & UTF-8
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_utf8.h` | `gd::utf8` | UTF-8 char ops, counting, encoding conversion, split, validate |
| `gd_utf8_string.h` | `gd::utf8` | UTF-8 string class |
| `gd_strings.h` | `gd` | `strings32`, `pointer::strings`, `view::strings` — multi-string containers |
| `gd_string.h` | `gd` | Lightweight string helpers |

### Parsing
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_parse.h` | `gd::parse` | `next_*`, `moveto_*`, `read_*`, `skip_*` — low-level text parsing |
| `parse/gd_parse_json.h` | `gd::parse::json` | Shallow JSON object parse, value type detection |
| `parse/gd_parse_uri.h` | `gd::parse::uri` | URI/URL path and query string parsing |
| `parse/gd_parse_match_pattern.h` | `gd::parse` | `patterns` — fast multi-pattern string matching |
| `parse/gd_parse_format_string.h` | `gd::parse` | Format string parsing utilities |

### Memory & Containers
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_arena.h` | `gd` | `arena` — block-chained allocator, 32-bit aligned, bulk allocation |
| `gd_arena_borrow.h` | `gd::arena::borrow` | `arena` — fixed-capacity arena with borrowed or owned storage |
| `gd_vector.h` | `gd::stack` | `vector<T>` — small-buffer-optimized vector (SVO) |

### Expression Engine
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `expression/gd_expression.h` | `gd::expression` | Expression parsing/evaluation with operator precedence and runtime |
| `expression/gd_expression_runtime.h` | `gd::expression` | Runtime evaluation of parsed expressions |

### Types & Utilities
| Header | Namespace | Key types / purpose |
|--------|-----------|---------------------|
| `gd_types.h` | `gd::types` | `enumTypeNumber`, `enumTypeGroup`, `enumType` — core type ID system |
| `gd_compiler.h` | `gd` | C++ standard and compiler detection macros |
| `gd_uuid.h` | `gd` | UUID generation/handling |
| `gd_binary.h` | `gd` | Binary data utilities |
| `gd_translate.h` | `gd` | String translation/mapping |
| `math/gd_math.h` | `gd::math` | `point` template, algebra utilities (C++20) |

## Code Architecture Layers

| Layer | Location | Naming | Notes |
|-------|----------|--------|-------|
| General/core | `external/gd/` | `lowercase_underscore` (STL-like) | No external deps, C++20 STL only |
| Source/corporate | `source/` | `PascalCase` + namespaces, classes prefix `C` | Reusable across targets |
| Target | `target/*/` | `PascalCase`, no namespaces | Target-specific, not reused |
| Play/test | `*/playground/`, `*/tests/` | No rules | Experimental |

## Naming Conventions (Hungarian Notation)

This project uses an evolved Hungarian notation. **Full domain concept names are mandatory** — never abbreviate semantic concepts.

### Variable Prefixes (for primitives)

| Prefix | Type | Good | Bad |
|--------|------|------|-----|
| `b` | bool | `bIsActive` | `bAct` |
| `i` | signed int (all sizes, incl. char) | `iTransactionSequence` | `iSeq` |
| `u` | unsigned int (all sizes, incl. size_t) | `uMessageType` | `uMsgType` |
| `d` | double/float | `dExchangeRate` | `dRate` |
| `p` | pointer/smart pointer | `pTransactionContext` | `pCtx` |
| `string` | std::string / string_view | `stringSessionToken` | `strTok` |
| `e` | enum value | `eBodyType` | — |
| `it` | iterator | `it` (in for-loop) | — |
| `m_` | member variable prefix | `m_uRowCount` | — |

### Scope Postfixes
| Postfix | Meaning |
|---------|---------|
| `_g` | global |
| `_s` | static / file scope |
| `_d` | debug only (`#ifndef NDEBUG`) |

### Other Types (objects, containers)
Use full lowercase class name as prefix, then camelCase name:
```cpp
std::vector<int> vectorNumbers;
std::pair<std::string_view, std::string_view> pairSelect;
gd::sql::query queryInsert;
pugi::xml_node xmlnodeQueries;
```

### Escape Hatch
Local throw-away variables that are verbose to name fully: append `_` suffix.
```cpp
std::vector<column> list_;   // declaration is clear from context
for( const auto& s_ : v_ )  // one-liner lambda/loop
```

### Method Names
No Hungarian notation on method names. Minimal words. No underscore between words at source/target level.

## Comment Style

- Inline comments aligned to **column 80**
- Member variable docs: `///< description`
- Method docs (full):
  ```cpp
  /** -------------------------------------------------------------------------- MethodName
   * @brief brief description
   * @param iVariable description
   * @return bool True if succeeded
   */
  ```
- Method docs (simple, in headers):
  ```cpp
  /// Brief description -------------------------------------------- MethodName
  ```
- Use markdown in comments: backticks for variables, **bold** for important things

## Search Tags

Used in comments to make code searchable:
```
@FILE, @CLASS, @API, @TASK, @PROJECT, @TODO, @NOTE, @CRITICAL, @DEBUG, @DEPRECATED
```

Format: `@TAGNAME [tag: context] [summary: short_summary] [description: longer]`

Task/project tracking via `@PROJECT [name: x]` and `@TASK [project: x] [user: per] [status: open]`.

## Code Formatting

```cpp
// if: space inside parens, no space after 'if'
if( condition ) { statement; }

// Multi-statement: Allman style
if( condition )
{
   statement1;
   statement2;
}

// Asserts: placed far right (~column 100)
const auto* ptable_ = pdocument->CACHE_Get("history");                          assert( ptable_ != nullptr );
```
