[← TOC](TOC.md)

### Overview: `gd::database::sqlite` – Simple & Safe SQLite Wrapper

`gd::database::sqlite` provides a **modern, clean C++ wrapper** around SQLite, making it easy to:
- Open/create databases
- Execute SQL statements
- Query results with cursors
- Handle transactions & last insert IDs

It includes three main classes:

| Class              | Purpose                                                                 | Typical Use Case |
|--------------------|-------------------------------------------------------------------------|------------------|
| `database`         | Manages SQLite connection (open/close/execute)                          | Main database object |
| `cursor`           | Executes prepared statements & iterates results                         | SELECT queries |
| `database_i` / `cursor_i` | COM-style interfaces for plugin/extensibility (optional)                | Advanced integration |

#### Key Benefits

- **Simple API** – no raw `sqlite3_*` calls needed for 95% of use cases
- **Variant-based** – easy to bind/read values with `gd::variant_view`
- **Error handling** – returns `std::pair<bool, std::string>` (success + error message)
- **Resource safety** – RAII for connections and statements
- **Thread-safe** when used properly (SQLite itself is thread-safe in serialized mode)

---

### Quick Start – Use SQLite in 5 Minutes

#### 1. Include the Header

```cpp
#include "gd_database_sqlite.h"
using namespace gd::database::sqlite;
```

#### 2. Basic Usage – Open, Insert, Query

```cpp
void example_sqlite() {
   // Open (or create) database
   database db;
   auto [ok, err] = db.open("test.db");
   if (!ok) { std::cerr << "Failed: " << err << "\n"; return; }

   // Execute DDL or DML
   db.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");

   // Insert with parameters
   cursor insert(&db);
   insert.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
   insert.bind_parameter(1, "Alice");
   insert.bind_parameter(2, 42);
   insert.execute();

   // Get last insert ID
   int64_t new_id = db.get_insert_key_raw();

   // Query
   cursor select(&db);
   select.open("SELECT id, name, age FROM users WHERE age > ?", { 30 });

   while (select.next()) {
      auto id   = select[0].as<int64_t>();
      auto name = select[1].as<std::string>();
      auto age  = select[2].as<int>();
      std::cout << id << ": " << name << " (" << age << ")\n";
   }
}
```

#### 3. Single-Value Queries (ask)

```cpp
gd::variant count;
db.ask("SELECT COUNT(*) FROM users WHERE age > 30", &count);
std::cout << "Users older than 30: " << count.as<uint64_t>() << "\n";
```

#### 4. Transactions

```cpp
db.transaction({ "BEGIN" });
// ... multiple inserts ...
db.transaction({ "COMMIT" }); // or "ROLLBACK"
```

#### 5. Using Cursor with Named Columns

```cpp
cursor c(&db);
c.open("SELECT id, name FROM users");
while (c.next()) {
   auto name = c["name"].as<std::string>();
   auto id   = c["id"].as<int64_t>();
   std::cout << id << ": " << name << "\n";
}
```

#### 6. Dynamic Binding (vector or initializer_list)

```cpp
std::vector<gd::variant_view> params = { "Bob", 35 };
cursor insert(&db);
insert.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
insert.bind_parameter(params);
insert.execute();
```

---

### When to Use It

| Scenario                              | Recommended? | Why |
|---------------------------------------|--------------|-----|
| Simple embedded database needs        | Yes          | Clean API, no boilerplate |
| Configuration storage                 | Yes          | Easy key-value with SQL |
| Small to medium datasets              | Yes          | SQLite is perfect |
| High-concurrency writes               | Maybe        | Use WAL mode + serialized access |
| Complex ORM needs                     | No           | Consider heavier libraries |

---

### Summary

`gd::database::sqlite` is your **go-to SQLite wrapper** when you want:
- **Simple, modern C++** interface
- **Safe resource management** (RAII)
- **Easy variant-based** data binding
- **Error handling** without exceptions

Start with:
```cpp
database db;
db.open("mydata.db");
cursor c(&db);
c.open("SELECT * FROM my_table");
while (c.next()) { /* process row */ }
```

Fast, reliable SQLite access without the usual hassle!
