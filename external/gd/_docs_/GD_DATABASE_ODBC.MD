### Overview: `gd::database::odbc` – Modern ODBC Wrapper for C++

`gd::database::odbc` is a **clean, modern C++ wrapper** around ODBC (Open Database Connectivity), making it simple to connect to any ODBC-compliant database (SQL Server, PostgreSQL, MySQL, Oracle, Access, Excel, etc.) with minimal boilerplate.

It provides two main classes:

| Class              | Purpose                                                                 | Typical Use Case |
|--------------------|-------------------------------------------------------------------------|------------------|
| `database`         | Manages ODBC connection (open/close/execute)                            | Main database object |
| `cursor`           | Executes prepared statements & iterates results                         | SELECT queries |

#### Key Benefits

- **Simple, RAII-based** API – no manual handle management
- **Variant-based** binding & reading – works with `gd::variant_view`
- **Error handling** – returns `std::pair<bool, std::string>` (success + error)
- **Prepared statements** – safe parameter binding
- **Cross-platform** – works on Windows/Linux/macOS (with ODBC drivers)

---

### Quick Start – Connect & Query in Minutes

#### 1. Include the Header

```cpp
#include "gd_database_odbc.h"
using namespace gd::database::odbc;
```

#### 2. Basic Usage – Connect, Insert, Query

```cpp
void example_odbc() {
   // 1. Open connection using DSN or connection string
   database db;
   auto [ok, err] = db.open("DSN=MySqlServer;UID=sa;PWD=secret");
   // Or: db.open("DRIVER={SQL Server};SERVER=localhost;DATABASE=mydb;Trusted_Connection=Yes");
   if (!ok) { std::cerr << "Connect failed: " << err << "\n"; return; }

   // 2. Execute DDL or non-query
   db.execute("CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY, name NVARCHAR(100), age INT)");

   // 3. Insert with parameters
   cursor insert(&db);
   insert.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
   insert.bind_parameter(1, "Bob");
   insert.bind_parameter(2, 35);
   insert.execute();

   // 4. Query data
   cursor select(&db);
   select.open("SELECT id, name, age FROM users WHERE age > ?", { 30 });

   while (select.next()) {
      auto id   = select[0].as<int>();
      auto name = select[1].as<std::string>();
      auto age  = select[2].as<int>();
      std::cout << id << ": " << name << " (" << age << ")\n";
   }
}
```

#### 3. Using Connection String Arguments

```cpp
gd::argument::arguments args;
args.set("DRIVER", "{SQL Server}");
args.set("SERVER", "localhost");
args.set("DATABASE", "mydb");
args.set("Trusted_Connection", "Yes");

database db;
db.open(args);
```

#### 4. Single-Value Queries (ask)

```cpp
gd::variant count;
db.ask("SELECT COUNT(*) FROM users", &count);
std::cout << "Total users: " << count.as<uint64_t>() << "\n";
```

#### 5. Transactions

```cpp
db.transaction({ "BEGIN" });
// ... multiple operations ...
db.transaction({ "COMMIT" }); // or "ROLLBACK"
```

#### 6. Named Columns & Dynamic Binding

```cpp
cursor c(&db);
c.open("SELECT id, name FROM users");
while (c.next()) {
   auto name = c["name"].as<std::string>();
   std::cout << name << "\n";
}

// Bulk bind
std::vector<gd::variant_view> row = { "Charlie", 28 };
cursor insert(&db);
insert.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
insert.bind_parameter(row);
insert.execute();
```

---

### When to Use It

| Scenario                              | Recommended? | Why |
|---------------------------------------|--------------|-----|
| Access any ODBC database              | Yes          | Universal connectivity |
| SQL Server, PostgreSQL, MySQL, Oracle | Yes          | Excellent driver support |
| Legacy systems (Access, Excel)        | Yes          | ODBC is perfect |
| High-performance bulk inserts         | Maybe        | Use prepared statements |
| Very large result sets                | Maybe        | Fetch in chunks |

---

### Summary

`gd::database::odbc` is your **modern, safe, and simple ODBC wrapper** for C++:

- **Easy connection** via DSN or connection string
- **RAII** for handles (no leaks)
- **Variant-based** data access
- **Prepared statements** with safe binding
- **Clear error handling** with messages

Start with:
```cpp
database db;
db.open("DSN=MyDatabase");
cursor c(&db);
c.open("SELECT * FROM my_table");
while (c.next()) { /* process row */ }
```

Connect to any ODBC database with clean, modern C++ code!
