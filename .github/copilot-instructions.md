# Instructions for AI coding assistants

These instructions describe the conventions used in this repository. Apply them
when writing, completing, editing, or reviewing code.

## 1. Priority and scope

When instructions appear to conflict, use this order:

1. Produce correct, safe, maintainable code that meets the user's request.
2. Follow instructions that apply to the specific file, target, or language.
3. Match the conventions in the code immediately surrounding the change.
4. Apply the general conventions in this file.

Do not sacrifice correctness to satisfy a naming or formatting preference. Do not
make unrelated changes. Preserve existing public behavior unless the request
requires changing it.

Use the most specific instructions available. For a completion inside an
existing file, its surrounding code is the strongest style example. If an
existing local pattern conflicts with this guide, preserve it in a small
completion unless the requested change requires resolving the inconsistency.

## 2. Completion and edit behavior

- For Fill-in-the-Middle (FIM), use both the code before and after the insertion
  point. Produce code that fits between them and leaves the surrounding syntax
  valid.
- Continue the nearest consistent local pattern: names, formatting, types,
  error handling, ownership, and API usage.
- Infer types from declarations, expressions, function signatures, and relevant
  API definitions. Do not infer a type solely from a variable-name prefix.
- With C++ `auto`, determine the deduced type from its initializer
  and the APIs involved before choosing a type-based name prefix. If the type
  cannot be established from available context, use the established local
  naming pattern without inventing type information.
- Preserve existing identifiers. Do not rename unrelated variables or rewrite
  surrounding code just to make the completion conform to this guide.
- Prefer the smallest complete change that satisfies the request. Do not add
  speculative abstractions, dependencies, comments, or error handling.
- Do not invent APIs, class members, files, or project conventions. Inspect
  declarations and nearby call sites when the required behavior is unclear.
- If essential requirements cannot be determined from the available context,
  ask a focused question rather than silently choosing behavior.
- After editing, run the most relevant available build, test, or static check.
  Report checks that could not be run; never imply that unrun checks passed.

## 3. Variable naming: Hungarian notation

Use Hungarian notation for variables: local variables, parameters, structured
bindings, and data members. The prefix communicates the variable's type or
category; the remaining name describes its full meaning.

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

For standard-library containers and project types, use the recognizable type
name in lowercase followed by the full semantic name:
`vectorPendingTransactions`, `pairSelect`, `queryUserBalanceUpdate`.
For a templated type, use its principal type name, not its full template
spelling, as the prefix.

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

In C#, apply the same type-based naming when `var` is used. Follow the local
project's C# casing and syntax conventions; do not replace `var` with an
explicit type just to expose a type prefix.

### Narrow exceptions

- A trivial loop counter may be `i` or `u` when its meaning is obvious and its
  scope is only the loop.
- Use `it` alone only for a short, obvious iterator scope; otherwise include a
  searchable name, such as `itEvent`.
- A short-lived throwaway local may use a trailing underscore to mark the
  exception, for example `list_`. Keep this exception rare and local.
- Do not use these exceptions for domain concepts, parameters, or members.

## 4. Method and type naming

Do not apply Hungarian notation to method names. Use the naming style for the
repository layer:

| Layer | Method naming |
|---|---|
| Core/shared library, including `external/gd/` | `lowercase_with_underscores`, like the STL |
| Corporate library | `PascalCase`, without underscores |
| Target-specific application code | `PascalCase`, without underscores |
| Tests and playcode | Follow the surrounding file |

Use concise, established names where their meaning is clear. Collection APIs
should generally follow STL names such as `size`, `empty`, `find`, `insert`,
`erase`, `begin`, `end`, and `contains`. Do not rename an existing API merely
to match this list.

Keep class, enum, and public API naming consistent with the surrounding
directory and target. Do not infer a naming convention from a different layer.

## 5. C++ formatting and implementation

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
- Use `assert` consistently with nearby code. Do not add alignment spaces merely
  to push an assertion to a fixed column.

## 6. Types, pointers, and ownership

- Prefer the ownership model already used by the surrounding code.
- Use smart pointers for newly introduced ownership where compatible with the
  surrounding API. Keep the `p` prefix for smart and raw pointer variables.
- Use raw pointers for non-owning observers only when appropriate for the API;
  make their non-owning lifetime clear in a nearby comment when it is not
  obvious.
- Avoid introducing raw owning pointers. If existing APIs require one, follow
  the established cleanup and lifetime pattern explicitly.
- Do not change ownership or lifetime semantics as part of an unrelated edit.

## 7. Comments and documentation

Comments should explain why code exists or clarify non-obvious behavior. Do not
add comments that merely restate the code.

- Match the comment style in the file.
- Document public or non-obvious methods when useful. Use Doxygen tags such as
  `@brief`, `@param`, `@return`, and `@tparam` where they apply; do not add empty
  or irrelevant tags.
- Document each template parameter with `@tparam` when documenting a template.
- Use `///<` for a data-member comment when a short same-line explanation is
  appropriate.
- Preserve existing search tags when editing tagged code. Use the project's tag
  format: `tagname [tag: context_words] [summary: short summary]`.

## 8. Repository-specific context

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
| `gd_variant_arg.h` | `gd`: `arg`, `args_view`, and `args` |
| `gd_arguments.h` | `gd::argument`: compact arguments buffer |
| `gd_arguments_shared.h` | `gd::argument::shared`: shared/COW arguments |
| `gd_arguments_io.h` | `gd::argument`: arguments serialization |
| `gd_table.h` | `gd::table`: table primitives |
| `gd_table_table.h` | `gd::table`: fixed-column table |
| `gd_table_arguments.h` | `gd::table::arguments`: table with dynamic row columns |
| `gd_table_column.h` | `gd::table::detail`: column metadata |
| `gd_table_index.h` | `gd::table`: integer index |
| `gd_table_io.h` | `gd::table`: table serialization and output |

Treat this list as navigation help, not a replacement for reading the relevant
declarations and implementations. Verify exact signatures and semantics in the
code before using an API.
