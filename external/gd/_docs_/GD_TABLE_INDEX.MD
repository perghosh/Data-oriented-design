[← TOC](TOC.md)

### Overview: `gd::table::index` – Fast Lookup Indexes for Tables

`gd::table::index` provides lightweight, **sorted index classes** to speed up lookups in `gd::table::table` and `gd::table::dto::table` objects.

It is designed to:
- Accelerate **frequent searches** by value (e.g., find row by ID, name, or key)
- Support **integer** and **string** columns
- Work as a **helper object** — not stored inside the table

#### Key Classes

| Class             | Purpose                                      | Best For |
|-------------------|----------------------------------------------|----------|
| `index_int64`     | Sorted index for 64-bit integer values       | IDs, counters, timestamps |
| `index_string`    | Sorted index for string values (string_view) | Names, keys, codes |

#### Main Features

- **Sorted vector** of `<value, row_index>` pairs
- **Fast binary search** via `find()` → O(log n)
- **Build once, use many** – ideal for static or semi-static tables
- **Type-safe** – each index knows its column type

---

### Quick Start – Create & Use an Index

#### 1. Include the Header

```cpp
#include "gd_table_index.h"
using namespace gd::table;
```

#### 2. Create an Index from a Table

```cpp
// Assume you have a table with "id" (int64) and "name" (string) columns
table tbl; // ... populated with data ...

// Build index on integer column (e.g., ID)
auto idx_id = create_index_g<index_int64>(tbl, "id");

// Build index on string column (e.g., name)
auto idx_name = create_index_g<index_string>(tbl, "name");
```

#### 3. Fast Lookup by Value

```cpp
// Find row by ID = 42
auto [found, row] = idx_id.find(42);
if (found) {
   std::cout << "Row with ID 42: " << row << "\n";
   // Access data: tbl.cell_get_variant_view(row, "name")
}

// Find row by name = "Alice"
auto [found_str, row_str] = idx_name.find("Alice");
if (found_str) {
   std::cout << "Found Alice in row " << row_str << "\n";
}
```

---

### When to Use Indexes

| Scenario                              | Recommended? | Why |
|---------------------------------------|--------------|-----|
| Frequent lookups by ID or key         | Yes          | O(log n) vs O(n) scan |
| Static or rarely changing tables      | Yes          | Build index once |
| Large tables (10k+ rows)              | Yes          | Big speedup |
| Dynamic tables with frequent inserts  | Maybe        | Rebuild index after changes |
| Very small tables (<100 rows)         | No           | Linear scan is faster |

---

### Summary

`gd::table::index` adds **fast lookup capability** to your tables:

- `index_int64` → perfect for IDs, numbers, timestamps
- `index_string` → great for names, codes, keys
- `create_index_g<...>(table, column)` → easy one-liner to build
- `find(value)` → returns `<bool, row_index>` pair

Use it whenever you need to **quickly find rows by a specific column value** — especially on larger or frequently queried tables.

Simple, fast, and lightweight!
