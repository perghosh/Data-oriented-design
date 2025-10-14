# Tutorial: Using Expressions

This tutorial will guide you through the process of using expressions in the FileCleaner tool. Expressions allow you to create dynamic and flexible rules for file management based on various criteria.

## What are Expressions?

Expressions in cleaner are similar to whats used in excel or sql where conditions can be created using logical operators (AND, OR, NOT) and functions to evaluate file attributes such as name, size, values found in code etc.

## Basic Syntax

List files in a directory:
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

*Note: if expected result is not shown check for `.gitignore` file in the directory.*

Some basic expressions:
```bash
# Find files containing the word "main"
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"str::find( filename, 'main', 0) not -1\""
```
```bash
# Count files larger than 1MB
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"size > 1000\""
```
```bash
# List all .cpp files
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"extension == '.cpp'\""
```
```bash
# Find files modified in the last 7 days
cleaner dir --source "C:\dev\home\DOD\target\TOOLS\FileCleaner\cli" --where "\"days < 7\""
```
