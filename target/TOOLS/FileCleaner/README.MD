# Cleaner.exe User Guide (BETA!!!)

## Getting Started (simple)

**Windows**
- Place cleaner in `system32` or `windows` folder, or add the path to the system PATH variable.
- Open a command prompt and go to folder with source code and type `cleaner count`.
- Default settings will scan files in current folder and print result to console.

**Linux**
- Place cleaner in `/usr/local/bin/` or `/usr/bin/`
- Open a command prompt and go to folder with source code and type `cleaner count`.
- Default settings will scan files in current folder and print result to console.

## Overview

Cleaner.exe is a powerful utility for analyzing source code files. It counts lines, measures code sections (comments and strings), and presents detailed statistics in a tabular format with summary information.

## Key Features

- Count total lines in source files
- Measure code, comments, and strings separately
- Generate character counts
- Process multiple files recursively
- Filter files based on custom criteria
- Export results in various formats

## Usage

### Basic Command Structure

```
cleaner.exe [global options] command [command options]
```

### Global Options

| Option | Description |
|--------|-------------|
| `--logging` | Turn on logging |
| `--logging-csv` | Add CSV logger for log information |
| `--print` | Print results from command |
| `--recursive [depth]` | Process directories recursively (specify depth) |
| `--database [folder]` | Set folder where logger places log files |
| `--statements [file]` | Specify file containing SQL statements |

### Count Command

The primary command is `count`, which analyzes source files and provides statistics.

```
cleaner.exe count --source [filepath] [options]
```

#### Count Options

| Option | Description |
|--------|-------------|
| `--source`, `-s` | File or directory to count lines in |
| `--comment [chars]` | Pair of characters marking start and end for comments |
| `--string [chars]` | Pair of characters marking start and end for strings |
| `--filter [pattern]` | Filter to use for file selection |
| `--recursive [depth]` | Specify recursion depth for directory traversal |

## Example Usage

### Count Lines in a Single File

```
cleaner.exe count --source myfile.cpp
```

### Recursive Analysis with Depth 4

```
cleaner.exe count --source C:\dev\project --recursive 4
```

### Count with Custom Comment and String Delimiters

```
cleaner.exe count --source script.py --comment "#" --string "''"
```

## Output Format

The output displays a table with the following columns:

| Column | Description |
|--------|-------------|
| folder | Directory containing the file |
| filename | Name of the file |
| count | Total number of lines |
| code | Number of code lines (excluding comments and strings) |
| characters | Number of characters in code |
| comment | Number of comment blocks/lines |
| string | Number of string literals |

A summary row is included at the bottom of the table with totals for all analyzed files.

## Example Output

```
folder                            filename       count    code     characters comment string 
+---------------------------------+--------------+--------+--------+---------+-------+-------+ 
| C:\dev\home\DOD\external\sqlite | shell.c      |  33700 |  19597 |  360177 |  1943 |  3641 | 
| C:\dev\home\DOD\external\sqlite | sqlite3.c    | 261454 | 141845 | 3074163 | 27270 |  6932 | 
| C:\dev\home\DOD\external\sqlite | sqlite3.h    |  13715 |   1812 |   56969 |   834 |     6 | 
| C:\dev\home\DOD\external\sqlite | sqlite3ext.h |    719 |    613 |   28285 |    69 |     1 | 
| Total:                          |              | 309588 | 163867 | 3519594 | 30116 | 10580 | 
+---------------------------------+--------------+--------+--------+---------+-------+-------+
```

## Tips for Effective Use

- Use the `--filter` option to focus on specific file types (e.g., "*.cpp" for C++ files)
- Set an appropriate recursion depth to control how deeply nested directories are processed
- Enable logging with `--logging` when troubleshooting or for record-keeping
- The `--print` flag ensures results are displayed in the console

## Advanced Usage

For complex source code analysis, combine multiple options:

```
cleaner.exe count --source C:\projects --recursive 3 --filter "*.cpp;*.h" --comment "/**/" --string "\"\"" --logging
```

This command will analyze C++ source files up to 3 directories deep, with appropriate comment and string delimiters, while logging the process.