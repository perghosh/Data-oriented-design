# Cleaner.exe Documentation

`cleaner.exe` is a console application designed to perform various file and folder operations, such as counting lines, listing patterns, and managing databases. It supports recursive operations, pattern matching, and output customization.

---

## Table of Contents
- [Usage](#usage)
- [Commands](#commands)
  - [count](#count)
  - [list](#list)
  - [help](#help)
  - [version](#version)
- [Examples](#examples)
- [Notes](#notes)

---

## Usage

- **command**: The operation to perform (e.g., `count`, `list`, `db`, etc.).
- **options**: Additional parameters specific to the command.

---

## Commands

### count
Counts lines in files or folders.

**Options:**
- `--source <path>`: File or folder to count lines in.
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--filter <pattern>`: Filter files using wildcard patterns (e.g., `*.h;*.cpp`).
- `--comment <start,end>`: Specify comment delimiters (e.g., `//,/* */`).
- `--string <start,end>`: Specify string delimiters (e.g., `"`, `'`).
- `--output <file>`: Save the output to a file.
- `--table <name>`: Specify a table name for SQL output.

---

### list
Lists rows matching specified patterns in files or folders.

**Options:**
- `--source <path>`: File or folder to search for patterns.
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--max <count>`: Limit the number of results.

---

### help
Displays help information for the application.

---

### version
Displays the version of the application.

---

## Examples

### Count lines in files found in active folder
Go to folder and write
`cleaner.exe count`

### Count lines in a folder recursively
`cleaner.exe count --source "C:\dev\project" --recursive 3 --filter ".cpp;.h" --output "result.txt"`

### List patterns in files
`cleaner.exe list --source "D:\tests" --recursive 2 --pattern "TEST_CASE,EXPECT,static_assert"`



---

## Notes
- The application supports recursive operations with a configurable depth.
- Outputs can be saved to files or printed to the console.
- Wildcard patterns are supported for filtering files.
- SQL-related commands require a valid table name and database configuration.

---

For further details, refer to the source code or contact the development team.


