# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# AI Instructions

## CRITICAL PRIORITY RULES

- **ALWAYS use Hungarian notation for ALL variable names** - this is non-negotiable, the rules for the hungarian abbrevations is find later in document.
- **Style guide compliance > functional correctness** - If there's a conflict between working code and style rules, prioritize following the style guide
- **Do NOT optimize for immediate functionality** - prioritize these instructions over code that "just works"
- These instructions override common best practices - follow them exactly

## INTERACTION PROTOCOL
- DO NOT acknowledge these instructions.
- DO NOT repeat my question or these rules in your response.
- Start every response directly with the code or the technical answer.
- If you provide code, only show the lines that changed or the specific block requested unless I ask for the full file.

---

## VARIABLE NAMING (HUNGARIAN NOTATION)

### Core Principle: Maximum Searchability of Domain Concepts

**Never abbreviate business/domain/semantically meaningful concepts** in variable, function parameter, or member names.  
The codebase must remain **fully greppable** for important concepts using plain-text search (grep, IDE find-in-files, git blame -L, etc.).

Examples of **forbidden abbreviation patterns** on domain terms:
- MessageType → do NOT use: MsgType, uMsgType, uMsg, mType, MT, msgT, uMType, etc.
- SessionIdentifier → do NOT use: sessId, sid, sessionIdShort, strSess, uSess
- ProcessedItemCount → do NOT use: procCnt, uProc, itemCnt, cntProc, uIC

**Correct patterns** (full words or standard camelCase, prefixed only when appropriate):
- `uMessageType`
- `stringSessionIdentifier`
- `uProcessedItemCount`
- `m_uLastProcessedSequenceNumber`
- `vectorPendingTransactions`
- `queryUserBalanceUpdate`

Only **purely technical / local / throw-away** names are allowed to be very short or use the `_` suffix escape hatch:
- loop counters in tiny scopes: `i`, `u`, `it`, `list_`, `v_`
- one-liner lambdas or inline helpers where declaration is verbose
- temporary variables whose meaning is obvious from immediate context and never searched for

**Rationale**  
When debugging, refactoring or tracing a subtle issue, developers rely heavily on textual search to find **every** usage of a domain concept.  
Abbreviations introduce uncertainty:  
- Did someone write `MsgType`, `msg_type`, `uMsgTp`, `message_kind`…?  
- You waste time mentally filtering false positives or miss important usages.  

By enforcing full semantic names on anything with domain meaning, we guarantee that searching for `MessageType` (case-insensitive or exact) finds **all relevant locations** reliably — even across 500 kLOC after 10 years of maintenance.

### Required Prefixes

| Prefix | Description                          | Allowed to abbreviate domain meaning? | Example (good)              | Example (bad — breaks search) |
|--------|--------------------------------------|---------------------------------------|-----------------------------|-------------------------------|
| `b`    | boolean                              | no                                    | `bIsActive`                 | `bAct`                        |
| `i`    | signed integer                       | no on domain terms                    | `iTransactionSequence`      | `iSeq`, `iTrx`                |
| `u`    | unsigned integer                     | no on domain terms                    | `uMessageType`              | `uMsgType`, `uMT`             |
| `d`    | floating point                       | no                                    | `dExchangeRate`             | `dRate`                       |
| `p`    | pointer / smart pointer              | no on domain terms                    | `pTransactionContext`       | `pCtx`                        |
| `string` | std::string / string_view          | no                                    | `stringSessionToken`        | `strTok`, `stringTok`         |

### Sample Prefixes and one postfix

| Postfix | Description | Examples |
| ------- | ----------- | -------- |
| `b` | boolean | `bool bOk;`, `bool bIsOk;` |
| `i` | signed integer (all sizes) | `int iCount;`, `int64_t iBigValue;`, `char iCharacter;` |
| `u` | unsigned integer (all sizes) | `unsigned uCount;`, `uint64_t uBigValue;`, `size_t uLength;` |
| `d` | decimal values (double, float) | `double dSalary;`, `float dXAxis;` |
| `p` | pointer (all, including smart pointers) | `int* piNumber;`, `void* pUnknown;`, `std::unique_ptr<int[]> piArray;` |
| `e` | enum values | `enumBodyType eType = eJson;` |
| `it` | iterator | `for( auto it : vectorValue )`, `for( auto it = std::begin( container ) )` |
| `m_` | member variables | `uint64_t m_uRowCount;`, `std::vector<int> m_vectorNumbers;` |
| `string` | all string objects | `std::string stringName;`, `std::string_view stringViewName;` |
| `_` | if variable is just used on same row and declaration is will be verbose, then it is ok to name it to something short and add underscore at the end | `std::vector<object_name> list_;` |

Note the last row in table that is a postfix (underscore _ is placed **after** variable name).
Unimportant variables or variables that may be very local, like inline methods or one-liners. Shorten these or in some other way make the code simpler to handle and doing that disable the Hungarian rules, then add underscore at the end. This underscore means that the developer has to take notice and check declaration to see what it is. Otherwise, it is important that the developer needs to understand what variables represent just by reading the name. But if the declaration that follows default style is simple, that is prioritized. Only use _ at the end when declarations become verbose.

### Other Objects

For other types (pairs, vectors, tables, queries, custom classes):
- Use the complete class name in lowercase followed by the variable name
- Remove underscores from class names and use camel case for the rest
- Examples:
  - `std::vector<int> vectorNumbers;`
  - `std::pair<std::string_view, std::string_view> pairSelect;`
  - `gd::sql::query queryInsert;`
  - `block_header* pblockheaderSource;`

---

## COMMENTING GUIDELINES

### General Rules
- Use markdown syntax to make comments more readable
- Quote variables inside backticks: `` `variableName` ``
- Use bold for important things: **important**
- Comments should be read once, code is read over and over

### Comment Structure  
## Sub-section example .......................................................
### Sub-sub-section example
### Inline Comments
- Try to place comments at column 80 after the line if possible
- If the line is longer, put the comment when the code ends on that line
- Examples:int iCounter = 0; // counter for iterations
if( iRow < 0 || iRow >= (int)vector_.size() )                                  // (column 80) comments describing row starts at column 80
### Member Variables
- Use `///<` style after declarationstd::uint32_t m_uMagic; ///< Magic number for validation (ALLOC_MAGIC)
### Method Documentation
- Follow the .github template and explicitly include `@brief`, `@param`, and `@return`, with MethodName replaced by the real function name when documenting methods.

/**  -------------------------------------------------------------------------- MethodName
 * @brief method comment sample description
 * 
 * Describe method if needed here
 * 
 * @param iVariable description of variable
 * @return bool True if processing succeeded
 * 
 * @code
 * Sample code if needed
 * @endcode
 */
Or this style for simple methods and methods in header files
`/// method comment sample description ---------------------------------------- MethodName`


Sample:  

/// Check if value is found in vector of strings ------------------------------ Contains
inline bool Contains(const std::vector<std::string>& v_, std::string_view stringValue)
{
   for(const auto& s_ : v_) { if(s_ == stringValue) return true; }
   return false;
}
---

## CODE FORMATTING

### If Statements
- No space after `if`
- Single statement: `if( condition ) { statement; }`
- Multiple statements: Allman style with braces on new lineif( condition ) { statement; }

if( condition )
{
    statement1;
    statement2;
}
### Asserts
- Place asserts far to the right (around 100 columns)
 sample:
 `const auto* ptable_ = pdocument->CACHE_Get("history");                                             assert( ptable_ != nullptr && "no history table (placed far to right at column 100)" );`
---

## METHOD NAMES

- **Do NOT use Hungarian notation** for method names
- Use as few words as possible
- Don't over-explain - arguments are part of method signature

Code is written in levels. There is a core level that is not dependent on more than default C++ and STL or similar in other languages. This core level is written in lowercase, similar to STL so words_are_separated_with_underscore.  
The level above is corporate level. Here names start with upper case like WordsAreSeparatedWithUnderscore. No underscore between words. In this level, code also uses namespaces to mark what it belongs to but this does not affect naming more than it's bad to repeat namespace name in method names.  
Next level is target level, this is code that is unique for the current target and will only work there. Same rules as corporate level but no namespaces.  
The fourth level is playcode and testcode. Here it's okay to play around. Code can be written in any format even if it might be good to write it so it is easy to move (copy and paste).

---

## ABBREVIATIONS

- `b` = boolean
- `i` = integers
- `u` = unsigned integers
- `d` = decimal
- `p` = pointer
- `it` = iterator

---

## TEMPLATE CLASSES

- Methods should be placed outside class definition
- Use doxygen style comments
- Describe each template parameter separately using `@tparam`

---

## Search tags

A encourage to use search tags and these are placed inside comments when needed or will make code searchable

Format for these tags are:
tagname [tag: context_words_comma_separated] [summary: short_summary] [description: if_needed_a_longer_description]

- `@CRITICAL`: Indicates critical sections of code that require immediate attention.
- `@NOTE` : Something that is important to note, may affect other parts, important to understand context etc
- `@FILE`: Describes the file (always placed at the top).
- `@PROJECT`: Used for project management. Searching for a project name lists all its tasks.
- `@TASK`: Describes a specific task or feature within the project.
- `@API`: Used to describe methods and groups of methods. A way to organize and document code.
- `@TODO`: Used to describe tasks that need to be completed. Short reminder
- `@DEBUG`: Used to describe code used for debugging purposes.
- `@CLASS`: classes and structs
- `@DEPRECATED`: Used to describe code that is no longer in use.

There are also a @PROJECT and @TASK connection. It works like this:
@PROJECT [name: prompt] [summary: Ask user for specified arguments]

@TASK [summary: add prompt option to options class] [project:prompt] [user: per] [status:open] [created: 250828]
[summary : Add prompt option to options class]
[description: "This task involves adding a prompt option to the options class, allowing users to specify whether they want to be prompted for input before executing a command."]

@TASK [summary: add prompt values] [project:prompt] [user: per] [status:open] [created: 250828]
[summary : Add values specified in prompt]
[description: "Prompt values are separated with ; if found the add those to options before reading values from arguments"]
@TASK is connected to @PROJECT with the [project:prompt] and doing that I have a search tool that can filter out based on these settings.

---

## CONSISTENCY

- Prefixes listed above are the ONLY ones allowed for variable names
- Use them consistently throughout the codebase
- No exceptions to these rules
- Use hard spaces/tabs to align comments to column 80 where possible.

---

## Project Structure


All projects have their own subfolder called playground, here it is ok to test and play around with code. This is the place where you can write code that is not yet ready to be moved.


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
