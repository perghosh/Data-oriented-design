
# Cleaner.exe User Guide

`cleaner.exe` is a command-line tool designed to analyze source code files, counting lines, code sections, comments, and strings, and presenting the results in a tabular format with a summary row. It is particularly useful for developers and project managers who need insights into code metrics for single files or entire directories.

## Features

- **Line Counting**: Counts total lines, code lines, and characters in source files.
- **Section Analysis**: Identifies and counts comments and string literals in the code.
- **Recursive Directory Scanning**: Supports recursive file analysis with customizable depth.
- **Flexible Output**: Outputs results in a clean table format, with optional CSV logging.
- **Customizable Filters**: Allows filtering files to analyze specific types or patterns.
- **Logging Options**: Supports detailed logging, including CSV format for further analysis.

## Command: `count`

The `count` command is the primary function of `cleaner.exe`. It analyzes source files and generates a table with the following metrics:

- **count**: Total number of lines in the file.
- **code**: Number of lines containing executable code (excluding comments and strings).
- **characters**: Total number of characters in the code lines.
- **comment**: Number of comments (e.g., `//`, `/* */` in C-like languages).
- **string**: Number of string literals (e.g., text enclosed in quotes).

### Example Output

Running the command in PowerShell:

```powershell
cleaner count --recursive 4

Output:

cleaner count --recursive 4                                                           in pwsh at 23:47:28
[info....] == Command: count
[info....] == --source: C:\dev\home\DOD\external\sqlite
[info....] == --recursive: 4
[info....] count = total number of lines
[info....] code = number of code lines
[info....] characters = number of code characters
[info....] comment = number of comments in code
[info....] string = number of strings in code

folder                            filename       count    code     characters comment string
+---------------------------------+--------------+--------+--------+---------+-------+-------+
| C:\dev\home\DOD\external\sqlite | shell.c      |  33700 |  19597 |  360177 |  1943 |  3641 |
| C:\dev\home\DOD\external\sqlite | sqlite3.c    | 261454 | 141845 | 3074163 | 27270 |  6932 |
| C:\dev\home\DOD\external\sqlite | sqlite3.h    |  13715 |   1812 |   56969 |   834 |     6 |
| C:\dev\home\DOD\external\sqlite | sqlite3ext.h |    719 |    613 |   28285 |    69 |     1 |
| Total:                          |              | 309588 | 163867 | 3519594 | 30116 | 10580 |
+---------------------------------+--------------+--------+--------+---------+-------+-------+

Program Options
cleaner.exe supports various command-line options to customize its behavior. Below is a list of global and command-specific options.
Global Options
These options apply to the overall operation of cleaner.exe:

    --logging: Enables logging of operations for debugging or tracking.
        Example: cleaner --logging count
    --logging-csv: Enables logging in CSV format for structured data output.
        Example: cleaner --logging-csv count
    --print: Ensures results are printed to the console.
        Example: cleaner --print count
    --recursive <depth>: Enables recursive directory scanning with a specified depth.
        Example: cleaner count --recursive 4
        <depth>: An integer specifying how many subdirectory levels to scan.
    --database <folder>: Sets the folder where log files are stored.
        Example: cleaner --database C:\logs count
    --statements <file>: Specifies a file containing SQL statements (for advanced use cases).
        Example: cleaner --statements queries.sql count

count Command Options
These options are specific to the count command:

    --source, -s <path>: Specifies the file or directory to analyze.
        Example: cleaner count --source C:\dev\sqlite
    --comment <start,end>: Defines the characters marking the start and end of comments (e.g., //,\\n for single-line comments or /*,*/ for multi-line).
        Example: cleaner count --comment "/*,*/"
    --string <start,end>: Defines the characters marking the start and end of string literals (e.g., " " or ' ').
        Example: cleaner count --string "\"" "\""
    --filter <pattern>: Filters files to analyze based on a pattern (e.g., *.c for C files). If empty, all files are counted.
        Example: cleaner count --filter "*.c"

Usage Examples

    Count lines in a single file:
    powershell

    cleaner count --source C:\dev\sqlite\shell.c

    Analyzes shell.c and outputs a table with line, code, character, comment, and string counts.
    Analyze a directory recursively:
    powershell

    cleaner count --source C:\dev\sqlite --recursive 4 --filter "*.c"

    Scans all .c files in the sqlite directory up to 4 levels deep.
    Enable CSV logging:
    powershell

    cleaner count --source C:\dev\sqlite --logging-csv --database C:\logs

    Logs results in CSV format to the specified log directory.
    Custom comment and string markers:
    powershell

    cleaner count --source C:\dev\file.c --comment "/*,*/" --string "\"" "\""

    Analyzes file.c with custom comment (/* */) and string (" ") markers.

Notes

    Ensure the --source path is valid and accessible.
    Use --filter to avoid processing irrelevant files in large directories.
    The --recursive option without a depth value may scan all subdirectories, which could be resource-intensive.
    Logging options (--logging, --logging-csv) are useful for auditing or integrating with other tools.
    If comment or string markers are not specified, cleaner.exe uses default markers based on common programming languages (e.g., //, /* */ for comments, " " for strings in C-like languages).

Getting Started

    Install: Ensure cleaner.exe is available on your system (consult your system administrator or documentation for installation).
    Run: Open a terminal (e.g., PowerShell, Command Prompt) and execute cleaner.exe with the desired options.
    Review Output: Check the console for the tabulated results or log files for detailed records.

For further assistance, consult the help command (if available, e.g., cleaner --help) or contact the tool's support team.


You can copy the above text directly into a `.md` file to create a formatted Markdown document. Let me know if you need further adjustments!