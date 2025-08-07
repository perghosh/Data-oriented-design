# Cleaner.exe Documentation

## Introducing cleaner – A Fast, Focused Code Search Tool

`cleaner` is a lightweight console application that integrates seamlessly with popular editors and IDEs like Visual Studio, Visual Studio Code, etc.

`cleaner` complements the search functionality in these tools and is built to be fast and efficient when it comes to searching through files and folders, counting lines, listing lines that match specific patterns, and handling file and directory operations.

Searching effectively is powerful, but searching through everything—especially in large codebases—can be time-consuming and resource-intensive. cleaner is designed to handle these situations. It allows searching in parts, searching within sections of code, and presenting search results in a way that suits the intended purpose.

Why the name `cleaner`? Managing code often means cleaning—removing clutter, refactoring, or reorganizing. But first, you need to find what needs fixing. cleaner helps you search smarter, so you can clean better.


---

## Table of Contents
- [Install](#install)
- [Usage](#usage)
- [Commands](#commands)
  - [config](#config)
  - [count](#count)
  - [find](#find)
  - [list](#list)
  - [copy](#copy)
  - [dir](#dir)
  - [history](#history)
  - [help](#help)
  - [version](#version)
- [Scripting and Advanced Usage](#scripting-and-advanced-usage)
- [Examples](#examples)
- [Supported File Extensions](#supported-file-extensions-and-languages)
- [Notes](#notes)

---

## Install

Easiest is to just copy cleaner executable to windows/system32 or /usr/local/bin if you are running windows or linux.

## Usage

```
cleaner.exe <command> [options]
```

- **command**: The operation to perform (e.g., `count`, `list`, `dir`, etc.).
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

### config
Manages configuration settings for cleaner.  
`cleaner-configuration.json` file is used to store configuration settings for cleaner and is placed in the user's home directory under `~/.cleaner/` on Linux or `%USERPROFILE%\.cleaner\` on Windows.

**Options:**
- `-create`: Create a new configuration file in the user's home directory.
- `-edit`: Edit the existing configuration file if any file is associated with the extension `json`.

### count
Counts lines in files or folders, or counts pattern occurrences.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns (e.g., `*.h;*.cpp`).
- `--pattern <patterns>`: Patterns to count, separated by `,` or `;`. Note that adding a pattern switches to counting pattern occurrences instead of lines.
- `--source <path;path>`: File or folder to count lines in. If not specified, the current directory is used. Cleaner supports positional arguments for source path, so you can use `cleaner count /home/username/dev/project/src/`. Multiple sources are separated by `;`.
- `--ignore <match;match;match>`: Ignore files or folders matching the specified patterns. Multiple patterns are separated by `;`.
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--segment <name>` : Segment in code to find patterns. Valid segment **name** are: code, **comment**, **string**, and **all** (all = all code).
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

### find
Find is similar to `list` command except that it isn't line based, it searches for patterns in all code for source files and that menas that it is possible to find patterns that spans multiple lines.
Using more advanced regex patterns is better suited for this command.
Also remember that code is stripped of comments and strings before searching. Finding patterns in comments or strings is not supported in this command. There you need to use `list` command instead. Reason for this is that `find` is targeted for finding patterns in code logic, and to simplify the search queries.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns, multiple patterns are separated by `;`.
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--rpattern <regex>`: Regular expression pattern to search for. Use multiple `--rpattern` to find more than one pattern.
- `--kv <keys@format>` : Key or keys to read values for and format how keys are found in code. This is a shortcut for `--keys` and `--kv-format` options. If keys contains `@` this doesn not work. 
  Example: `--kv "key1,key2,key3@[]:"`
- `--keys <key1;key2,...>`: Keys to read values for. This is used to extract key-value pairs from the code to produce a report from the code. When multiple keys are specified in `keys` option they are separated by `;` and the value for each key is printed on a separate line in the output. 
- `--header <key>`: Header to include in the output for key-value pairs. Key need to be within `keys` and value for that is put in header.
- `--kv-format <format>`: Format how keys are found in code. 2 or 3 characters are expected, like `{}:` for `{key: value}`, `[]:` for `[key: """value with space and new line characters"""]` for  or `()=`. The key value logic tries to figure out how to read it and flexibilitye is the key. If format is not specified, it defaults to separate key with space character and read value until end of line or double space characters.
- `--source <path>`: File or folder to search for patterns. If not specified, the current directory is used. Cleaner support positional arguments for source path, so you can use `cleaner list /home/username/dev/project/src/`.
- `--ignore <match;match;match>`: Ignore files or folders matching the specified patterns. Multiple patterns are separated by `;`.
- `--segment <name>` : Segment in code to find patterns. Valid segment **name** are: code, **comment**, **string**, and **all** (all = all code).
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--context <lines>`: Show surrounding lines of a match, where `<lines>` is the number of lines to show before and after the match or if only one number is specified it will show that many lines after the match.
- `--max <count>`: Limit the number of results.
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
- `--ignore <match;match;match>`: Ignore files or folders matching the specified patterns. Multiple patterns are separated by `;`.
- `--rpattern <regex>`: Regular expression pattern to search for.
- `--segment <name>` : Segment in code to find patterns. Valid segment **name** are: code, **comment**, **string**, and **all** (all = all code).
- `--recursive <depth>`: Perform the operation recursively up to the specified depth.
- `--context <lines>`: Show surrounding lines of a match, where `<lines>` is the number of lines to show before and after the match or if only one number is specified it will show that many lines after the match.
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
Dir differs from normal dir/ls in that it addapted to work for developers or people that work with code related files. For example it can filter files basedon whats inside the file.

**Options:**
- `--filter <pattern>`: Filter files using wildcard patterns, multiple patterns are separated by `;` (e.g., `*.h;*.cpp`).
- `--pattern <patterns>`: Patterns to search for, separated by `,` or `;`.
- `--source <path>`: Directory to list.
- `--ignore <match;match;match>`: Ignore files or folders matching the specified patterns. Multiple patterns are separated by `;`.
- `--segment <name>` : Segment in code to find patterns. Valid segment **name** are: code, **comment**, **string**, and **all** (all = all code).
- `--sort <column>`: Sort the output by column name when result is printed to console.
- `-R`: Search recursively in current or selected folder as deep as possible (max 16 levels).

---

### history (not ready yet)
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

## Scripting and Advanced Usage

### Rule and how to use rules
Cleaner supports advanced scripting capabilities, allowing users to define custom rules and patterns for searching and manipulating code segments. This feature is particularly useful for complex searches or when integrating cleaner into larger workflows. Or format output to for different purposes.  
Sample is to show code between `@code` and `@endcode` markers:
```
cleaner find -R --source "path/to/source" --pattern "@TAG;#database" -match-all --rule "select-between:@code,@endcode"
```
This will extract sample code between `@code` and `@endcode` markers in the source files, that are tagged with `@TAG` and `#database`.  

Avalable rules are:
- `select-between:<start>,<end>`: Selects text between specified start and end markers.
- `select-line:<marker>`: Selects line at the specified line marker. If no marker is specified, it selects line where match was found.
- `select-all`: Selects all text in the segment.

---

## Examples

### Count lines in files found in active folder
Go to folder and write (Windows): `cleaner.exe count` or Linux: `cleaner count`  
Sort by column name "filename" and print to console: `cleaner count -R --sort filename`
Sort on code lines and only count lines in cpp files: `cleaner count -R --sort code --filter *.cpp`

### List rows matching specified patterns in files or folders
`cleaner list /home/username/dev/project/src/ --pattern text-to-find --recursive 3`

### Count lines in a folder recursively
Using `-R` to search recursively and `--source` to specify the folder:  
`cleaner.exe count --source "C:\dev\project" --recursive 3 --filter ".cpp;.h" --output "result.txt"`

### List patterns in files
`cleaner.exe list --source "D:\tests" --recursive 2 --pattern "TEST_CASE,EXPECT,static_assert"`

### List files in Visual Studio ouput window (make sure that one instance of visual studio is open)
`cleaner list --source "project-full-path" -vs --pattern "TEST_CASE,EXPECT,static_assert"`

### Use the dir command to list *.cpp* files in current directory and subfolders (no source = current directory)
`cleaner dir -R "--filter", "*.cpp"`

### View last page for the largest files
`cleaner count -R "--page", "-1"`

### With "context" option, you can see the surrounding lines of a match
`cleaner list -R --filter "*.h" --pattern "#pragma once" --context 20` - This probably shows some includes statements after the match
`cleaner list -R --filter "*.h" --pattern "using pointer" --context "-10 20"` - Shows 10 lines before and 20 lines after an may inform about member types.

### Find patterns in code logic
`cleaner find -R --source "C:\dev\project" --rpattern "if\s*\([^\)]*\)\s*[^\s{][^\n;]*" --context 3 --max 10` - Finds `if` statements without `{}` conditions and shows 3 lines of context around each match.

### Find code between markers
`cleaner find -R --source "C:\dev\project\filename.cpp" --pattern "#database" --rule "select-between:@code,@endcode"`  
Finds code between `@code` and `@endcode` markers in the selected source file were pattern `#database` is found.

### Key value paris samples
`cleaner find -R --source "C:\dev\project" --segment comment --pattern "@file" --keys "@file,@brief"` -Prints the doxygen @file and @brief values for files.  
  
if you have **TODO** markers in your code you could format them like this: `// TODO: [description: "Add logging to <file>"]`  
`cleaner find -R --source "C:\dev\project" --segment comment --pattern "TODO:" --keys "description" --kv-format "[]:"` - This will find all TODO markers in comments and print the description value for each TODO marker.  
*Same command as above*   
`cleaner find -R --source "C:\dev\project" --pattern "c--TODO:" --kv "description@[]:"`

*Produce output with header and where key value pairs are placed in columns, sample output under*   
`cleaner find -R --source "C:\dev\project" --kv-format", "[]:" --pattern "@TASK" --kv "name;date;description;sample" --header "name"`  
```
+- config ----------------------------------------------------------------------+
C:\dev\home\DOD\target\TOOLS\FileCleaner\cli\CLIConfig.h(69)
description: ## Open configuration file if it exists.
             For windows this file should be placed in `C:\Users\<username>\AppData\Local\cleaner\cleaner-configuration.json`.
             For linux this file should be placed in `~/.local/share/cleaner/cleaner-configuration.json`.
```


---

## Supported File Extensions and Languages

| Extension | Language | Comment Styles | String Formats |
|-----------|----------|---------------|---------------|
| `.cpp`, `.c`, `.h`, `.hpp`, `.cc`, `.cxx`, `.hxx`, `.ipp`  | C/C++ | Line: `//`<br>Block: `/* */` | Standard strings with escape chars<br>Raw strings: `R"()"` |
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
| `.bat`, `.cmd` | Batch/Command | Line: `REM`<br>Line: `::` | Double and single quotes with escape chars |
| `.ps1` | PowerShell | Line: `#`<br>Block: `<#` `#>` | Double and single quotes with escape chars<br>Here-strings: `@"` `"@` |
| `.mak`, `.makefile`, `.ninja` | Make/Ninja | Line: `#` | None defined |
| `.ini` | INI/Config | Line: `;`<br>Line: `#` | None defined |

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
