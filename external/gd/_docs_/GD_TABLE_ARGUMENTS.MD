[← TOC](TOC.md)

### Overview: `gd::table::arguments::table` – Flexible Table with Per-Row Extra Columns

`gd::table::arguments::table` is a **powerful, dynamic columnar table** that extends `gd::table::table` with the ability to store **extra, row-specific columns** using a per-row `gd::argument::arguments` object.

This makes it ideal for:
- **Dynamic data** where rows may have different additional fields
- **Semi-structured data** (e.g., database query results with extra metadata)
- **Configuration tables** with optional key-value pairs
- **JSON-like row data** without losing columnar performance

#### Key Features Compared to `gd::table::table`

| Feature                     | `gd::table::table`                  | `gd::table::arguments::table` (this one)                  |
|-----------------------------|--------------------------------------|-----------------------------------------------------------|
| **Fixed Columns**           | Yes                                  | Yes                                                       |
| **Extra Per-Row Columns**   | No                                   | Yes (via `gd::argument::arguments` per row)               |
| **Null & Row Status**       | Optional                             | Optional + Arguments support                              |
| **Memory Layout**           | Single contiguous block              | Single block + per-row arguments object                   |
| **Use Case**                | Static schema, high-performance      | Flexible schema, dynamic fields                           |

---

### Quick Start Guide – Start Using It Today

#### 1. Include the Header

```cpp
#include "gd_table_arguments.h"   // or gd_table.h depending on your setup
using namespace gd::table::arguments;
```

#### 2. Basic Usage – Create a Dynamic Table

```cpp
table tbl(tag_full_meta{});  // nulls, row status + arguments support

// Add fixed columns
tbl.column_add("rstring", 0, "path");
tbl.column_add("uint64", 0, "size");
tbl.prepare();  // Finalize structure

// Add a row with fixed values
auto uRow = tbl.row_add_one();
tbl.row_set(uRow, { 
   {"path", "C:\\test\\file.txt"}, 
   {"size", 12345}
});

// Add extra dynamic fields (automatically creates arguments object for this row)
tbl.cell_set_argument(uRow, "owner", "Alice");
tbl.cell_set_argument(uRow, "modified", "2025-01-01");

// Read fixed or dynamic fields the same way
std::string owner = tbl.cell_get_variant_view(uRow, "owner").as_string();
uint64_t size = tbl.cell_get<uint64_t>(uRow, "size");
```

#### 3. Working with Row Arguments

```cpp
// Check if row has extra arguments
if (tbl.row_is_arguments(uRow)) {
   auto* args = tbl.row_get_arguments_pointer(uRow);
   std::cout << args->get("owner").as_string() << "\n";  // "Alice"
}

// Add or modify extra fields
tbl.cell_add_argument(uRow, "tags", {"important", "backup"});

// Get all extra fields as arguments object
gd::argument::arguments extra = tbl.row_get_arguments(uRow);
```

#### 4. Harvest & Plant – Move Data Around

```cpp
// Extract specific rows into another table
std::vector<uint64_t> rows = {0, 2};
gd::table::dto::table harvested;
tbl.harvest(rows, harvested);

// Plant fixed columns into a static table
table static_tbl(tag_null{});
static_tbl.column_add("rstring", "path");
static_tbl.column_add("uint64", "size");
tbl.plant(static_tbl, "path");  // copies path column
```

#### 5. Iteration & Search

```cpp
// Iterate all rows (fixed + dynamic fields)
for (auto row : tbl) {
   auto path = row.cell_get_variant_view("path").as_string_view();
   auto owner = row.cell_get_variant_view("owner", tag_arguments{}).as_string_view();
   std::cout << path << " owned by " << owner << "\n";
}

// Find rows with specific dynamic field
int64_t row = tbl.find("owner", "Alice");
```

---

### When to Use `gd::table::arguments::table`

| Scenario                              | Use This? | Why |
|---------------------------------------|-----------|-----|
| Rows with optional/extra fields       | Yes       | Per-row arguments object |
| JSON-like data with varying keys      | Yes       | Flexible, dynamic columns |
| Database results with metadata        | Yes       | Fixed schema + extra info |
| Strict fixed-schema data              | Maybe     | Use `gd::table::table` instead |
| Very high-performance needs           | Maybe     | Extra arguments add overhead |

---

### Summary

`gd::table::arguments::table` gives you the **best of both worlds**:
- **Columnar performance** for fixed fields
- **Dynamic flexibility** for per-row extra fields

Use it when your data has a **core fixed structure** but needs **optional or row-specific metadata**.

Start with:
```cpp
table tbl(tag_full_meta{});
tbl.column_add(...);
tbl.prepare();
```

Then add fixed values with `row_set()` and dynamic fields with `cell_set_argument()`.

Enjoy powerful, flexible, and still fast tabular data!
