# Cleaner.exe Documentation

`cleaner.exe` is a console application designed to perform various file and folder operations, such as search in files or counting lines etc, it has strong search capabilities, allowing users to find specific patterns in files and folders. The application is built with a focus on performance and usability, making it suitable for developers and system administrators who need to manage large codebases or datasets efficiently.

---

## Table of Contents
- [Usage](#usage)
- [Commands](#commands)
  - [count](#count)
  - [list](#list)
  - [copy](#copy)
  - [dir](#dir)
  - [history](#history)
  - [help](#help)
  - [version](#version)
- [Examples](#examples)
- [Supported File Extensions](#supported-file-extensions-and-languages)
- [Notes](#notes)

---

## Usage

```
cleaner.exe <command> [options]
```

- **command**: The operation to perform (e.g., `count`, `list`, `db`, etc.).
- **options**: Additional parameters specific to the command.

---

### Usage Examples with *
Cleaner support some shortcuts to make it esier to use.
The `*` (or `.`, `**` beacuse `*` is specific in linux) added after command means that cleaner should scan all files in current folder and subfolders. Its a shortcut for `-R` and `--source *`.

- `cleaner count *` count lines in all files in current folder and subfolders.
- `cleaner count * assert,for,if,while` counts the patterns "assert", "for", "if", "while" in all files in current folder and subfolders.
- `cleaner list * assert` lists all rows matching pattern "assert" in all files in current folder and subfolders.`
- `cleaner dir *` lists all files in current folder and subfolders.


## Commands

### count
Counts lines in files or folders, or counts pattern occurrences.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns (e.g., `*.h;*.cpp`).
- `--pattern <patterns>`: Patterns to count, separated by `,` or `;`. Note that adding a pattern switches to counting pattern occurrences instead of lines.
- `--source <path>`: File or folder to count lines in. If not specified, the current directory is used. Cleaner supports positional arguments for source path, so you can use `cleaner count /home/username/dev/project/src/`.
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--output <file>`: Save the output to a file.
- `--page <index>`: Print the output to the console from slected page, default page size is 10 lines.
- `--page-size <size>`: Specify the number of lines per page for console output.
- `--sort <column>`: Sort the output by column name when result is printed to console.
- `--stats <type>`: Show statistics for the specified pattern. Currently "sum" is the only statistic available.
- `--table <name>`: Specify a table name for SQL output.
- `--comment <chars>`: Pair of characters marking start and end for comments.
- `--string <chars>`: Pair of characters marking start and end for strings.
- `-R`: Search recursively in current or selected folder as deep as possible (max 16 levels).
- `--vs`: Adapt to Visual Studio output window format, making files clickable.
- `--win`: Windows-specific functionality, adapting to features specific to Windows.

---

### list
Lists rows matching specified patterns in files or folders.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns, multiple patterns are separated by `;`.
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--source <path>`: File or folder to search for patterns. If not specified, the current directory is used. Cleaner support positional arguments for source path, so you can use `cleaner list /home/username/dev/project/src/`.
- `--rpattern <regex>`: Regular expression pattern to search for.
- `--script <pattern>`: Additional filter for script files.
- `--max <count>`: Limit the number of results.
- `--segment <type>`: Segment in code to find patterns. Valid segments are: code, **comment**, **string**, and **all** (all = all code).
- `-R`: Search recursively in current or selected folder as deep as possible (max 16 levels).
- `--vs`: Adapt to Visual Studio output window format, making files clickable.
- `--win`: Windows-specific functionality, adapting to features specific to Windows.

---

### copy (not ready yet)
Copies files from source to destination.

**Options:**
- `--source <path>`: File to copy.
- `--destination <path>`: Destination where file is copied to.
- `--backup`: If destination file exists, make a backup before copying.

---

### dir
Lists files in a directory.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns, multiple patterns are separated by `;` (e.g., `*.h;*.cpp`).
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--source <path>`: Directory to list.
- `-R`: Search recursively in current or selected folder as deep as possible (max 16 levels).

---

### history
Handles command history.

**Options:**
- `--create`: Initialize history logic, creates folders and files needed to manage history, also enables configuration settings.

---

### help
Displays help information for the application.

---

### version
Displays the version of the application.

---

## Examples

### Count lines in files found in active folder
Go to folder and write (Windows): `cleaner.exe count` or Linux: `cleaner count`  
Sort by column name "filename" and print to console: `cleaner count -R --sort filename`
Sort on code lines and only count lines in cpp files: `cleaner count -R --sort code --filter *.cpp`

### List rows matching specified patterns in files or folders
`cleaner list /home/username/dev/project/src/ --pattern text-to-find --recursive 3`

### Count lines in a folder recursively
`cleaner.exe count --source "C:\dev\project" --recursive 3 --filter ".cpp;.h" --output "result.txt"`

### List patterns in files
`cleaner.exe list --source "D:\tests" --recursive 2 --pattern "TEST_CASE,EXPECT,static_assert"`

### List files in Visual Studio ouput window (make sure that one instance of visual studio is open)
`cleaner list --source "project-full-path" -vs --pattern "TEST_CASE,EXPECT,static_assert"`

### Use the dir command to list *.cpp* files in current directory and subfolders (no source = current directory)
`cleaner dir -R "--filter", "*.cpp"`

### View last page for the largest files
`cleaner count -R "--page", "-1"`

---

## Supported File Extensions and Languages

| Extension | Language | Comment Styles | String Formats |
|-----------|----------|---------------|---------------|
| `.cpp`, `.c`, `.h`, `.hpp` | C/C++ | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Raw strings: `R"()"` |
| `.cs` | C# | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Verbatim strings: `"""..."""` |
| `.fs` | F# | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Verbatim strings: `"""..."""` |
| `.kt` | Kotlin | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Multiline strings: `"""..."""` |
| `.swift` | Swift | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Multiline strings: `"""..."""` |
| `.java` | Java | Line: `//`<br>Block: `/* */` | Standard strings with escape chars |
| `.js` | JavaScript | Line: `//`<br>Block: `/* */` | Double and single quotes with escape chars<br>Template literals: `` ` ` `` |
| `.ts` | TypeScript | Line: `//`<br>Block: `/* */` | Double and single quotes with escape chars<br>Template literals: `` ` ` `` |
| `.jsx` | React JSX | Line: `//`<br>Block: `/* */` and `{/* */}` | Double and single quotes with escape chars<br>Template literals: `` ` ` `` |
| `.tsx` | React TSX | Line: `//`<br>Block: `/* */` and `{/* */}` | Double and single quotes with escape chars<br>Template literals: `` ` ` `` |
| `.go` | Go | Line: `//`<br>Block: `/* */` | Double quotes with escape chars<br>Raw string literals: `` ` ` `` |
| `.rs` | Rust | Line: `//`<br>Block: `/* */` | Standard strings with escape chars |
| `.html`, `.xml` | HTML/XML | Block: `<!-- -->` | Double quoted attributes |
| `.css` | CSS | Block: `/* */` | Double quoted values |
| `.py` | Python | Line: `#` | Double quoted strings |
| `.sql` | SQL | Line: `--`<br>Block: `/* */` | Double quoted strings |
| `.php` | PHP | Line: `//` and `#`<br>Block: `/* */` | Double and single quotes with escape chars |
| `.lua` | Lua | Line: `--`<br>Block: `--[[` `]]` | Double and single quotes with escape chars<br>Raw strings: `[[` `]]` |
| `.rb` | Ruby | Line: `#`<br>Block: `=begin` `=end` | Double and single quotes with escape chars |
| `.json` | JSON | None | Double quoted strings |
| `.pl`, `.pm` | Perl | Line: `#` | Double and single quotes with escape chars |
| `.sh`, `.bash` | Shell/Bash | Line: `#` | Double quotes with escape chars<br>Single quotes without escape chars |
| `.yaml`, `.yml` | YAML | Line: `#` | Double and single quoted strings |
| `.toml` | TOML | Line: `#` | Double and single quoted strings |
| `.dart` | Dart | Line: `//`<br>Block: `/* */` | Double quotes with escape chars<br>Raw strings: `r"..."` |
| `.clj` | Clojure | Line: `;` | Double quotes with escape chars |
| `.vim` | Vim Script | Line: `"` | Double quotes with escape chars<br>Single quotes with escape chars |
| `.txt`, `.md` | Plain text/Markdown | None | None defined |

---

## Notes
- The application supports recursive operations with a configurable depth, or using `-R` for maximum depth (16 levels).
- Outputs can be saved to files or printed to the console.
- Wildcard patterns are supported for filtering files.
- SQL-related commands require a valid table name and database configuration.
- Visual Studio integration is available with the `--vs` option for clickable file paths.
- Command history can be enabled and configured for persistent command tracking.

---

For further details, refer to the source code or contact the development team.