### Overview: `gd::table::table` – Optimized Member Table for Columnar Data

`gd::table::table` is a **high-performance, contiguous-memory columnar table** designed specifically to be used as a **member variable** in C++ classes or structs — especially in multithreaded or long-lived objects.

It is the **member-friendly sibling** of `gd::table::dto::table` (the DTO version optimized for data transfer).  

#### Key Differences

| Feature                     | `gd::table::dto::table`               | `gd::table::table` (this one)                  |
|-----------------------------|----------------------------------------|------------------------------------------------|
| **Primary Use**             | Data transfer, temporary results       | Member variable, long-lived objects            |
| **Column Metadata**         | Owned internally                       | Shared via `detail::columns*` (ref-counted)    |
| **Thread Safety**           | Not designed for it                    | Safe when columns are shared                   |
| **Memory Ownership**        | Full ownership                         | Shared column info, efficient copies           |
| **Typical Scenario**        | Query results, CSV parsing             | Configuration tables, caches, session data     |

#### Why Use It as a Member?

- **Shared column metadata** → multiple tables can share the same column layout (zero-copy)
- **Reference-counted columns** → safe to copy/move tables without duplicating column info
- **Thread-safe column access** → multiple threads can read from tables with shared columns
- **Efficient resizing** → grows only the data block, columns stay shared

---

### Quick Start Guide – Get Started in Minutes

#### 1. Include the Header

```cpp
#include "gd_table_table.h"   // or gd_table.h depending on your setup
using namespace gd::table;
```

#### 2. Basic Usage – Create a Table as a Class Member

```cpp
class UserSession {
   table m_tblPreferences;   // Table is a member!

public:
   UserSession() {
      // Define columns once
      m_tblPreferences = table( 0, { 
         { "rstring", 0, "key"   },
         { "rstring", 0, "value" }
      }, tag_prepare{} );
   }

   void loadPrefs() {
      // Add some rows
      m_tblPreferences.row_add({ "theme",   "dark" });
      m_tblPreferences.row_add({ "lang",    "en"   });
      m_tblPreferences.row_add({ "refresh", "60"   });
   }

   std::string get(const std::string_view& key) const {
      auto row = m_tblPreferences.find("key", key);
      if (row >= 0) return m_tblPreferences.cell_get<std::string>(row, "value");
      return "";
   }
};
```

#### 3. Shared Columns – Multiple Tables with Same Layout

```cpp
class Application {
   detail::columns* m_pSharedColumns = nullptr; // Shared across instances

public:
   Application() {
      // Define columns once
      m_pSharedColumns = new detail::columns();
      m_pSharedColumns->add({ "int32", 0, "id" });
      m_pSharedColumns->add({ "rstring", 64, "name" });
   }

   ~Application() { m_pSharedColumns->release(); }

   table createUserTable() {
      return table(m_pSharedColumns, 100, 0, 50); // 100 initial rows, grow by 50
   }
};

void example() {
   Application app;
   table tblUsers  = app.createUserTable();
   table tblAdmins = app.createUserTable(); // Same column layout, shared!
   // No extra memory for column info
}
```

#### 4. Common Operations – Simple & Fast

```cpp
// Add a row with values
tbl.row_add({ 42, "Alice" });

// Get value by column name
int id = tbl.cell_get<int>(0, "id");
std::string name = tbl.cell_get<std::string>(0, "name");

// Find row
int64_t row = tbl.find("name", "Alice");

// Iterate rows (C++17 structured bindings)
for (auto row : tbl) {
   auto [id, name] = row.get_variant_view();
   std::cout << id.as<int>() << ": " << name.as_string() << "\n";
}

// Trim & clean input
tbl.cell_set(0, "name", trim("   Bob   ", tag_view{})); // "Bob"
```

---

### When to Use `gd::table::table`

| Scenario                              | Use This? | Why |
|---------------------------------------|-----------|-----|
| Configuration/settings tables         | Yes       | Shared columns, fast lookup |
| User session data                     | Yes       | Long-lived, thread-safe reads |
| Cache tables                          | Yes       | Efficient resizing & copying |
| Query results (temporary)             | Maybe     | Use `dto::table` instead |
| Very small tables (<10 rows)          | Maybe     | Overhead may not be worth it |

---

### Summary

`gd::table::table` is the **perfect choice** when you want a **fast, columnar table as a member variable** in your classes:

- Shared column definitions → zero-copy column info
- Efficient memory use → grows only data
- Safe for multithreading → when columns are shared
- Same powerful API as `dto::table` → `find()`, `sort()`, `split()`, `harvest()`, etc.

Use it for **configuration**, **session data**, **caches**, or any **long-lived structured data** in your application.

Start with the simple constructor + `tag_prepare{}` and grow from there!
