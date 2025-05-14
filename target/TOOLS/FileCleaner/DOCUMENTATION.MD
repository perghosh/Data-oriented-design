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
- `--output <file>`: Save the output to a file.
- `--pattern <patterns>`: Patterns to count, separated by `,` or `;`. Note that adding pattern to count switches to count number of times pattern is found instead of counting lines and segments
- `--sort`: Sort the output by column name, this is when result is printed to console.
- `--stats`: Show statistics for the specified pattern. For now "sum" is the only statistic available.
- `--table <name>`: Specify a table name for SQL output.
- `-R` Search recursively in current or selected folder as deep as possible (max 16 levels).

---

### list
Lists rows matching specified patterns in files or folders.

**Options:**
- `--max <count>`: Limit the number of results.
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--segment`: Segment in code to find patterns. Valid segments now are code, **comment**, **string**, and **all** (all = all code).
- `--source <path>`: File or folder to search for patterns.
- `-R` Search recursively in current or selected folder as deep as possible (max 16 levels).

---

### help
Displays help information for the application.

---

### version
Displays the version of the application.

---

## Examples

### Count lines in files found in active folder
Go to folder and write (windows) `cleaner.exe count` and linux `cleaner count`  
Sort by column name "filename" and print to console: `cleaner count -R --sort filename`
Sort on code lines and only count lines in cpp files: `cleaner count -R --sort code --filter *.cpp `

List rows matching specified patterns in files or folders.
`cleaner list /home/username/dev/project/src/ --pattern text-to-find --recursive 3`

Count lines in a folder recursively
`cleaner.exe count --source "C:\dev\project" --recursive 3 --filter ".cpp;.h" --output "result.txt"`

List patterns in files
`cleaner.exe list --source "D:\tests" --recursive 2 --pattern "TEST_CASE,EXPECT,static_assert"`



---

## Notes
- The application supports recursive operations with a configurable depth.
- Outputs can be saved to files or printed to the console.
- Wildcard patterns are supported for filtering files.
- SQL-related commands require a valid table name and database configuration.

---

For further details, refer to the source code or contact the development team.




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


