# Tutorial: Using Expressions

This tutorial will guide you through the process of using expressions in the FileCleaner tool. Expressions allow you to create dynamic and flexible rules for file management based on various criteria.

## What are Expressions?

Expressions in cleaner are similar to whats used in excel or sql where conditions can be created using logical operators (AND, OR, NOT) and functions to evaluate file attributes such as name, size, values found in code etc.

## Basic Syntax

### "dir": List files in a directory:

```bash
# List all files in the specified directory
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli"
```
This command lists all files in the specified directory.
Based on settings the output differ but filename, folder, days and size are shown.
To list files in subdirectories use the `--recursive` option:
```bash
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --recursive  5
```   

*Note: if expected result is not shown check for `.gitignore` file in the current directory.*

Some basic expressions:
```bash
# Find files containing the word "main"
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner" --where "\"str::find( filename, 'main', 0) not -1\""
```
```bash
# Count files larger than 1000 bytes
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner" --where "\"size > 1000\"" --recursive 5
```
```bash
# List all .cpp files
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"extension == '.cpp'\""
```
```bash
# Find files modified in the last 7 days
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"days < 7\""
```

### "count": Count content information in files:

```bash
# Count without specifying a pattern will count lines, lines with code, code characters, number of comments and number of strings.
cleaner count --source "C:/dev/home/DOD/target/TOOLS" --recursive 3

# folder                                   filename            count   code    characters
#                                                                                       comment
#                                                                                              string
#  +----------------------------------------+------------------+-------+-------+--------+------+------+
#  | C:\dev\home\DOD\target\TOOLS\Backup    | CMakeLists.txt   |    41 |       |        |      |      |
#  | C:\dev\home\DOD\target\TOOLS\Backup    | Application.cpp  |   345 |   177 |   4534 |   37 |   38 |
#  | C:\dev\home\DOD\target\TOOLS\Backup    | Application.h    |   164 |    71 |   2683 |   39 |    3 |
```



Based on these columns you can create expressions to filter the results.
For example, to find files with more than 100 lines of code:
```bash
# Find files with more than 100 lines of code
cleaner count --source "C:/dev/home/DOD/target/TOOLS" --recursive 3 --where "\"code > 100\""
```

Count only specific files by using the `--filter` option and here only `.cpp` files are counted that have more than 100 lines of code.
```bash
# Count files lines and show files with more than 100 lines of code (code only)
cleaner count --source "C:/dev/home/DOD/target/TOOLS" --filter "*.cpp" --recursive 3 --where "code > 100"
```

List lines in cpp files that have less code than half the total line count.
```bash
cleaner count --source "C:/dev/home/DOD/target/TOOLS" --filter "*.cpp" --recursive 3 --where "(code * 2) < count"
```


### "list": List files based on content patterns:

```bash
# List files with "catch(" in their source content (not comment or strings)
cleaner list --source "C:/dev/home/DOD/target/TOOLS" --recursive 3 --logging-severity "debug" --pattern "catch("
```
