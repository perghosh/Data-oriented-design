# Code Analysis Module

This folder contains the code analysis automation components used by FileCleaner. The module provides expression-based code parsing, source code navigation, and table filtering capabilities.

## Overview

The code analysis module consists of two main components:

1. **Expression Module** (`Expression.h`, `Expression.cpp`) - Manages source code navigation, line-by-line parsing with state awareness, and custom expression methods
2. **Run Module** (`Run.h`, `Run.cpp`) - Executes expressions on tables with filtering and row selection capabilities

## Components

### Expression Module

#### ExpressionSource Class

The `ExpressionSource` class provides state-aware source code parsing and navigation. It can:

- **Navigate through files** line by line using `GotoLine()`
- **Handle parsing states** (comments, strings, etc.) to skip non-code content
- **Select specific content** from source code
- **Read large files efficiently** using a buffered window approach

Key methods:
- `OpenFile()` - Opens a file and prepares the state machine
- `GotoLine(uint64_t)` - Navigates to a specific line in the source
- `GetGotoLineText()` - Retrieves text at the current position
- `CloseFile()` - Closes the file and releases resources

#### Custom Expression Methods

The module injects custom methods into the expression engine:

| Method | Syntax | Description |
|--------|--------|-------------|
| `count_lines` | `count_lines(source)` | Counts lines in the source |
| `get_argument` | `get_argument(name, arguments_ptr)` | Retrieves an argument value by name |
| `get_cell_value` | `get_cell_value(column, row, table_ptr)` | Gets a cell value from a table (column can be string or int) |
| `select_all` | `select_all(source)` | Returns the entire source content |
| `select_between` | `select_between(end, start, source_ptr)` | Selects text between two markers (returns vector) |
| `select_line` | `select_line(marker_or_line, source_ptr)` | Selects a line (by marker or line number) |
| `select_lines` | `select_lines(...)` | Placeholder for multi-line selection |

### Run Module

The Run module provides expression execution capabilities for table operations:

#### Functions

- **`RunExpression_g()`** - Executes a general expression with arguments and table inputs
  - Takes expression string, code arguments, line list table, and snippet table
  - Processes the expression and populates results into tables

- **`RunExpression_Where_g()`** - Filters table rows based on expression conditions
  - Multiple overloads for different table types (`gd::table::dto::table`, `gd::table::arguments::table`)
  - Optional callback for handling filtered results
  - Modifies the table by removing non-matching rows

- **`RunExpression_WhereExpression_g()`** - Non-modifying version of Where
  - Executes where expression without modifying the table
  - Provides callback to handle filtered row indices

## Usage Pattern

The typical workflow involves:

1. **Create an ExpressionSource** pointing to a source file
2. **Open the file** to prepare the state machine
3. **Navigate to specific lines** using `GotoLine()`
4. **Execute expressions** with custom methods to extract or analyze code
5. **Filter table data** using `RunExpression_Where_g()` based on expression results

## State-Aware Parsing

The `ExpressionSource` class uses a state machine to handle different parsing contexts:

- Skips comments (single-line `//` and multi-line `/* */`)
- Handles string literals properly
- Tracks line positions accurately
- Efficiently processes large files using a rotating buffer window

## Dependencies

This module depends on:
- `gd_expression_runtime` - Core expression parsing and evaluation
- `gd_expression_method_01` - Standard mathematical and logical methods
- `gd_parse_window_line` - Buffered line reading
- `gd_table_dto` - Table data structures
- `gd_arguments_shared` - Argument management

## Tags

- `#expression` - Expression engine integration
- `#where` - Table filtering operations
- `#source` - Source code handling
- `#state` - Parser state management
