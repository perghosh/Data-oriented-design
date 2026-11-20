## Searching the code with cleaner

Use `C:\Tools\cleaner.exe` for ALL code searching and file listing, instead of reading whole files,
guessing, or using PowerShell `dir` / `Get-ChildItem`.
Call it from PowerShell by full path: `& 'C:\Tools\cleaner.exe' <command> ...`

### Commands
- `list`: single-line matches (names, calls, declarations). `find` takes the same options but matches across lines
- `count`: matches per file. Use for scoping only, then use `list` or `find` for exact lines before editing
- `dir`: list files (not folders). With `--pattern` only files containing the text

### Get the options before the first use
Before using a command for the first time in a chat, run
`& 'C:\Tools\cleaner.exe' help <list|count|dir> -llm` (`list` also covers `find`) and follow its output.
Do not guess option names: unknown options print help and `ERROR: Unknown option`.

### Always
- Filter is the first positional argument, e.g. `"*.h;*.cpp"`. Use `--filter` if it is not first.
- Set `--current-directory "<folder>"`. Add `-R` to include subfolders.
- Bound output: `--max N` for `list`, `find` and `dir`; `--page-size N` for `count`.
- Use `-llm` (single dash) on every command. With `list` and `find`, also use `--context N` to read surrounding code.
- Separate several literal patterns with `;`. Repeat `--rpattern` for several regex expressions.

### Output
`list` and `find`: lines look like `path:line - [text]`. `dir`: a `Current Directory:` line, then paths relative to it.
Ignore progress lines ("Find in files: ..."). The exit code is often -1 even when the output is correct, so judge success from the output.

## Priority and scope

1. **Correctness first:** Never sacrifice correctness or break existing public behavior to satisfy a style rule.
2. **Local context over general rules:** Match the conventions in the immediately surrounding code first; fall back to this guide when no local pattern exists.
3. **Minimal scope:** Make only the requested changes. Do not refactor or rename unrelated code.

## Completion and edit behavior

- **Context & style:** Match the surrounding code's formatting, error handling,
  ownership, and API usage. For Fill-in-the-Middle (FIM), ensure the code fits
  cleanly between the code before and after the cursor.
- **Types & `auto`:** Determine exact types from visible declarations and API
  signatures, and name `auto` variables after their deduced type. When a
  declaration is out of sight, trust the codebase's Hungarian prefixes as
  reliable type indicators.
- **No guessing:** Never invent APIs, class members, or files—inspect headers
  or call sites if unsure, and ask a focused question if requirements are
  ambiguous.
- **Minimal edits:** Do not rename existing identifiers, refactor surrounding
  code, or add speculative abstractions or error handling.
- **Single-target build:** Never build the entire solution or run tests unless
  requested. After editing, build ONLY the specific target/project owning the
  modified file(s) (e.g., MSBuild `/t:TargetName` or CMake `--target <name>`)
  and fix any compiler errors before finishing.


## Variable naming: Hungarian notation

Use Hungarian notation for variables: local variables, parameters, structured
bindings, and data members. The prefix communicates the variable's type or
category; the remaining name describes its full meaning. Reason why this is 
that patterns are central to the codebase, and type and pattern describe the
variable's role without needing to read implementation details.

### Type and category prefixes

| Prefix | Use for | Examples |
|---|---|---|
| `b` | `bool` | `bActive`, `m_bFirst` |
| `i` | signed integers; `char` when used as a character/code unit | `iTransactionSequence`, `iCharacter` |
| `u` | unsigned integers, including `size_t` | `uMessageType`, `uIndex` |
| `d` | floating-point values | `dExchangeRate`, `dXAxis` |
| `p` | pointers, including smart pointers | `pTransactionContext`, `pnodeOwner` |
| `e` | enum values | `eShape`, `eJson` |
| `string` | `std::string`, `std::string_view`, or equivalent strings | `stringSessionToken`, `stringViewName` |
| `it` | iterators | `itEvent`, `itCurrent` |
| type name | objects whose type is not covered above | `vectorPendingTransactions`, `queryInsert` |

For a class data member, put `m_` before the type prefix or type-based name:
`m_bActive`, `m_uRowCount`, `m_pTransactionContext`,
`m_vectorPendingTransactions`.

Use the full, searchable domain term. Do not shorten meaningful words:
`uMessageType`, not `uMsgType`; `stringSessionIdentifier`, not `sessId`;
`uProcessedItemCount`, not `itemCnt`.

For standard-library containers and project types, use the complete object name
 in lowercase followed by the full semantic name:
`vectorPendingTransactions`, `pairSelect`, `queryUserBalanceUpdate`.
For a templated type, use its principal type name, not its full template
spelling, as the prefix.

### Scope and role suffixes

Apply these suffixes after the full name (never use `g_` or `s_` prefixes):

| Suffix | Scope / role | Applies to | Examples |
|---|---|---|---|
| `_s` | static | variables, methods | `uInstanceCount_s`, `create_instance_s()` |
| `_g` | global | variables, methods | `bIsInitialized_g`, `get_context_g()` |
| `_d` | debug | variables, methods | `bIsActive_d`, `dump_state_d()` |
| `_i` | interface | variables | `pshapeService_i`, `jsonParse_i` |

### `auto`, `var`, and inferred types

Type inference changes what is visible in the declaration, not the naming rule.
Choose the prefix using the type established by the initializer and API:

```cpp
auto bActive = window.IsActive();
auto uItemCount = vectorItems.size();
auto itEvent = vectorEvents.begin();
auto* pWindow = CreateWindow();
```

These names are appropriate only if the shown expressions actually return the
corresponding types. Do not choose `u` because a value is a count if its type is
signed, or `p` unless the result is a pointer.

### Narrow exceptions

- A trivial loop counter may be `i` or `u` when its meaning is obvious and its
  scope is only the loop.
- Use `it` alone only for a short, obvious iterator scope; otherwise include a
  searchable name, such as `itEvent`.
- A short-lived throwaway local may use a trailing underscore to mark the
  exception, for example `list_`. Keep this exception rare and local.
- Do not use these exceptions for domain concepts, parameters, or members.

## Method and type naming

Do not apply Hungarian notation to method or type names. Follow the layer rules below:

| Layer | Method naming | Class naming & namespace |
|---|---|---|
| Core/general library (`external/gd/`) | `lowercase_with_underscores` (STL style) | `lowercase_with_underscores`, inside namespace |
| Source / corporate library | `PascalCase` | `CPascalCase` (starts with `C`), inside namespace |
| Target-specific application code | `PascalCase` | `CPascalCase` (starts with `C`), no namespace |
| Tests and playcode | Follow surrounding file (relaxed rules) | Follow surrounding file (relaxed rules) |


Use concise, established names where their meaning is clear. Collection APIs
should generally follow STL names such as `size`, `empty`, `find`, `insert`,
`erase`, `begin`, `end`, and `contains`. Do not rename an existing API merely
to match this list.

Keep class, enum, and public API naming consistent with the surrounding
directory and target. Do not infer a naming convention from a different layer.

## C++ formatting and implementation

- Match the file's existing indentation and whitespace. In the core library,
  the common indentation is three spaces.
- Write conditions as `if( condition )`.
- Put multi-statement control-flow braces on separate lines (Allman style).
  A single short statement may use `if( condition ) { statement; }`.
- Keep lines at or below 120 characters where practical. Break long argument
  lists across lines when it improves readability.
- Prefer existing project helpers and APIs over duplicate implementations.
- Define template methods outside the class body when following project style,
  but keep template definitions in a location visible to their users (normally
  the header). Do not move a required template definition into an inaccessible
  source file.
- Use `assert` consistently with nearby code.

## Comments and documentation

- Always add Doxygen documentation comments (`@brief`, `@param`, `@return`, `@tparam`) to all new or significantly modified methods and functions, except trivial one-line getters/setters.
- For key public APIs, complex logic, or non-obvious usage, include a concise `@code` / `@endcode` usage example inside the Doxygen comment.
- Do not add inline comments that merely restate obvious code; reserve inline comments for explaining *why* non-obvious logic exists.
- Match the comment style in the file. Use `///<` for short same-line data-member explanations.
- Preserve existing search tags when editing tagged code: `tagname [tag: context_words] [summary: short summary]`.

## Repository-specific context

`external/gd/` contains the shared GD library and is a core part of this
repository. Prefer reusing its existing types and utilities when working in
project code. Do not inspect or modify unrelated `external/` libraries unless
the task specifically concerns them.

Useful GD headers:

| Header | Namespace / purpose |
|---|---|
| `gd_types.h` | `gd::types`: core type identifiers |
| `gd_variant.h` | `gd`: owning `variant` value |
| `gd_variant_view.h` | `gd`: non-owning `variant_view` |
| `gd_arguments.h` | `gd::argument`: compact arguments buffer |
| `gd_table_arguments.h` | `gd::table::arguments`: table with dynamic row columns |

Treat this list as navigation help, not a replacement for reading the relevant
declarations and implementations. Verify exact signatures and semantics in the
code before using an API.

