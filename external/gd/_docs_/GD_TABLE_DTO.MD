### Short Tutorial: `gd::table::dto::table` – High-Performance Columnar Data Transfer Object (DTO)

`gd::table::dto::table` (aliased from the internal `table_column_buffer`) is a **highly optimized, contiguous-memory columnar table** designed for **fast data transfer and temporary storage** inside C++ applications.

It's perfect for:
- Moving structured data between functions/modules
- Intermediate results in data processing pipelines
- High-performance scenarios needing SIMD-friendly layouts
- Database query results or CSV/JSON imports
- When you need **null support** like SQL databases

#### Key Features

- **Single contiguous memory block** → excellent cache locality
- **Columnar storage** → ideal for vectorized/SIMD operations
- **Supports null values** (per-cell, like databases)
- **Row status flags** (deleted, modified, etc.)
- **Reference types** for large blobs/strings without copying
- **Fast add/find/sort** operations
- **Zero-copy views** via `variant_view`

#### Memory Layout

All data lives in one block:
```
[Cell values (column-major)] + [Optional per-row metadata (null flags + status)]
```

Example with 3 columns (int32, string, double):
```
Row 0: [int32][string...][double]
Row 1: [int32][string...][double]
...
[Optional: null bitmap + row status per row]
```

#### Basic Usage

```cpp
#include "gd_table_column-buffer.h"
using namespace gd::table::dto;  // table = dto::table

int main() {
    // 1. Create table with null support
    table tbl(100, table::eTableFlagNull64);  // up to 64 columns can be null

    // 2. Add columns
    tbl.column_add("id", "int32");
    tbl.column_add("name", "string", 64);     // max 64 chars
    tbl.column_add("score", "double");
    tbl.column_add("active", "bool");

    tbl.prepare();  // Finalize structure

    // 3. Add rows
    tbl.row_add({"1", "Alice", 95.5, true});
    tbl.row_add({"2", "Bob", 87.0, false});
    tbl.row_add({"3", nullptr, 91.2, true});  // NULL name

    // 4. Access cells
    double score = tbl.cell_get<double>(0, 2);           // Row 0, column "score"
    std::string name = tbl["name"][1].as<std::string>(); // Row 1

    // 5. Find row
    int64_t row = tbl.find("name", "Alice");
    if (row >= 0) {
        std::cout << "Found Alice at row " << row << "\n";
    }

    // 6. Iterate rows (C++17 structured binding)
    for (auto [id, name, score, active] : tbl) {
        std::cout << id.as<int>() << ": " << name.as_string()
                  << " (" << score.as<double>() << ")\n";
    }

    // 7. Harvest column data
    auto scores = tbl.harvest<double>("score");  // std::vector<double>

    return 0;
}
```

#### Advanced Features

```cpp
// SIMD-friendly layout
table simd_tbl(1000, 0, 0, "double", 8, tag_prepare{});  // 8 double columns
// Data is 64-byte aligned → perfect for AVX512

// Copy-on-write sharing
table tbl2 = tbl1;           // Fast reference copy
tbl2.cell_set(0, 0, 999);    // Triggers deep copy only when modified

// Sort by column
tbl.sort("score", false);    // Descending

// Find all matching rows
auto rows = tbl.find_all("active", true);

// Null handling
tbl.cell_set_null(5, "name");
bool is_null = tbl.cell_is_null(5, "name");
```

#### When to Use `gd::table::dto::table`

| Scenario                              | Recommended? | Why |
|---------------------------------------|--------------|-----|
| Transferring query results            | Yes          | Null support + columnar |
| High-performance numeric processing   | Yes          | SIMD-friendly layout |
| Temporary in-memory tables            | Yes          | Fast allocation/access |
| Need SQL-like NULL semantics          | Yes          | Built-in null flags |
| Long-term persistent storage          | No           | Use database or file format |
| Very small datasets (<10 rows)        | Maybe        | Overhead not worth it |

#### Summary

`gd::table::dto::table` is the **go-to choice** for moving structured, heterogeneous data around in performance-critical C++ code. It combines:

- Database-like features (nulls, row status)
- Extreme performance (contiguous, columnar, SIMD-ready)
- Flexible API (variant_view integration)
- Efficient memory use (single allocation)

Use it whenever you're passing tabular data between components — it's the "Data Transfer Object" pattern done right for modern C++.