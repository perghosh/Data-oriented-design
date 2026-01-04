### Overview: `gd::table::io` – Table Output Formatting (CSV, JSON, SQL, CLI, etc.)

`gd::table::io` (defined in `gd_table_io.h`) provides a **flexible, high-quality output system** for converting `gd::table::table` and `gd::table::dto::table` objects into various human-readable and machine-readable formats.

It supports:
- **CSV** (with or without headers)
- **JSON** (array of objects, named objects, column-based)
- **SQL** (INSERT statements)
- **CLI** (pretty grid/console output)
- **Raw** (unformatted data dump)
- **Custom formatting** (via callbacks)

All functions are designed to work seamlessly with both fixed-schema `table` and dynamic `dto::table` types.

#### Key Features

| Format       | Description                                                                 | Header Support | Custom Formatting |
|--------------|-----------------------------------------------------------------------------|----------------|-------------------|
| **CSV**      | Standard comma-separated values with proper quoting                         | Yes            | Yes (via callback) |
| **JSON**     | Array of objects, named objects, or column-based JSON                       | Yes            | Yes               |
| **SQL**      | Generates `INSERT INTO` statements with proper escaping                     | No             | No                |
| **CLI**      | Pretty-printed grid with aligned columns (ideal for console output)         | Yes            | Yes               |
| **Raw**      | Unformatted data dump (useful for debugging or binary transfer)             | No             | No                |

#### Main Functions

| Function Signature (simplified)                          | Purpose                                      | Formats Supported |
|-----------------------------------------------------------|----------------------------------------------|-------------------|
| `to_string(table, ..., tag_io_csv)`                       | Convert table to CSV string                  | CSV               |
| `to_string(table, ..., tag_io_json)`                      | Convert table to JSON string                 | JSON              |
| `to_string(table, ..., tag_io_cli)`                       | Pretty grid output for console               | CLI               |
| `to_string(table, ..., tag_io_raw)`                       | Raw data output                              | Raw               |
| `write_insert_g(table_name, table, ..., tag_io_sql)`      | Generate SQL INSERT statements               | SQL               |
| `read_g(table, csv_string, ...)`                          | Parse CSV back into table                    | CSV (input)       |

#### Quick Start Examples

```cpp
#include "gd_table_io.h"
using namespace gd::table;

// Assume `tbl` is a populated gd::table::dto::table

// 1. Simple CSV output (with header)
std::string csv = to_string(tbl, tag_io_header{}, tag_io_csv{});

// 2. JSON array of objects
std::string json = to_string(tbl, tag_io_json{});

// 3. Pretty console table (CLI)
std::string console = to_string(tbl, tag_io_cli{});

// 4. SQL INSERT statements
std::string sql;
write_insert_g("users", tbl, sql, tag_io_sql{});

// 5. Custom CSV with formatting callback
std::string custom_csv;
to_string(tbl, 
   [](const std::string_view& value, std::string& out) {
      // Custom escaping or transformation
      out = "\"" + std::string(value) + "\"";
      return true;
   }, 
   custom_csv, tag_io_csv{});
```

#### When to Use It

| Scenario                              | Best Format | Why |
|---------------------------------------|-------------|-----|
| Export to spreadsheets/tools          | CSV         | Universal compatibility |
| API responses / web services          | JSON        | Standard, readable |
| Logging or console display            | CLI         | Human-readable grid |
| Database migration / scripting        | SQL         | Ready-to-run INSERTs |
| Debugging / raw data transfer         | Raw         | No transformation |

#### Summary

`gd::table::io` is your **one-stop solution** for exporting table data in multiple formats:

- **CSV** → file export, Excel
- **JSON** → APIs, web
- **CLI** → console/logs
- **SQL** → database scripts
- **Custom** → full control via callbacks

Use the simple `to_string(..., tag_io_xxx)` overloads for most cases, or dive into the detailed versions for fine-tuned control.

Perfect for logging, exporting, debugging, or integrating with other systems!
