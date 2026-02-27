# Getting Started: `gd::sql::query` Builder — Fluent SQL Query Construction

[← TOC](TOC.md)

## Overview

The `gd::sql::query` builder provides a **fluent, type-safe interface** for constructing SQL queries programmatically. It eliminates error-prone string concatenation while offering full control over SQL generation through composable builder objects.

### Why Use the Query Builder?

| Benefit | Description |
|---------|-------------|
| **Security** | Validates query components and helps prevent SQL injection by proper parameter handling |
| **Portability** | Write once, generate for SQLite, PostgreSQL, SQL Server, Oracle, MySQL |
| **Maintainability** | Self-documenting code that's easier to read and refactor than string SQL |
| **Dynamic Construction** | Build queries programmatically based on runtime conditions |
| **Type Safety** | Catch column name typos and type mismatches early |
| **Composability** | Reuse query fragments and combine them as needed |

---

## Core Concepts

The query builder consists of three main fluent builder types:

| Builder | Purpose | Factory Function |
|---------|---------|------------------|
| `table_builder` | Define tables, aliases, joins, key relationships | `table_g("name")` |
| `field_builder` | Define individual fields with attributes | `field_g("name")` |
| `fields_builder` | Define multiple fields at once | `fields_g("table", ...)` |
| `condition_builder` | Define WHERE/HAVING conditions | `condition_g("name")` |

All builders use the `<<` stream operator to add components to a `gd::sql::query` object.

---

## Quick Start

### Basic SELECT Query

```cpp
#include "gd/gd_sql_query_builder.h"

using namespace gd::sql;

// Simple SELECT all fields
query q;
q << table_g("users")
  << field_g("*").select();

std::string stringSQL = q.sql_get(eSqlSelect);
// Output: SELECT * FROM users
```

### SELECT with Specific Fields and WHERE

```cpp
query q;
q << table_g("users")
  << field_g("id").as("user_id")
  << field_g("name").as("username")
  << field_g("email")
  << condition_g("active").value(1).eq()
  << condition_g("age").value(18).gt();

std::cout << q.sql_get(eSqlSelect) << "\n";
// Output: SELECT id AS user_id, name AS username, email FROM users WHERE active = 1 AND age > 18
```

### INSERT Query

```cpp
query q;
q << table_g("users")
  << field_g("name").value("John Doe")
  << field_g("email").value("john@example.com")
  << field_g("age").value(30);

std::cout << q.sql_get(eSqlInsert) << "\n";
// Output: INSERT INTO users (name, email, age) VALUES ('John Doe', 'john@example.com', 30)
```

### UPDATE Query

```cpp
query q;
q << table_g("users")
  << field_g("name").value("Jane Doe")
  << field_g("email").value("jane@example.com")
  << condition_g("id").value(123).eq();

std::cout << q.sql_get(eSqlUpdate) << "\n";
// Output: UPDATE users SET name = 'Jane Doe', email = 'jane@example.com' WHERE id = 123
```

### DELETE Query

```cpp
query q;
q << table_g("users")
  << condition_g("id").value(123).eq()
  << condition_g("deleted_at").is_null();

std::cout << q.sql_get(eSqlDelete) << "\n";
// Output: DELETE FROM users WHERE id = 123 AND deleted_at IS NULL
```

---

## Builder Detailed Guide

### `table_builder` — Table Definitions

Define tables with aliases, schemas, joins, and key relationships.

```cpp
// Basic table
query q1;
q1 << table_g("users");

// Table with alias
query q2;
q2 << table_g("users").as("u");

// Table with schema
query q3;
q3 << table_g("users").schema("public");

// Manual JOIN with raw SQL
query q4;
q4 << table_g("orders")
   << table_g("users").join("LEFT JOIN users ON orders.user_id = users.id");

// Automatic JOIN using key/fk relationships
query q5;
q5 << table_g("orders").key("id")
   << table_g("users").fk("user_id").parent("orders");
```

**Table Builder Methods:**

| Method | Description | Example |
|--------|-------------|---------|
| `as("alias")` | Set table alias | `table_g("users").as("u")` |
| `schema("name")` | Set database schema | `table_g("users").schema("public")` |
| `parent("table")` | Set parent table for joins | `table_g("orders").parent("users")` |
| `join("SQL")` | Set raw JOIN condition | `table_g("orders").join("LEFT JOIN users ON ...")` |
| `key("column")` | Set primary key column | `table_g("users").key("id")` |
| `fk("column")` | Set foreign key column | `table_g("orders").fk("user_id")` |
| `owner("name")` | Set table owner | `table_g("users").owner("admin")` |

### `field_builder` — Individual Field Definitions

Define fields with aliases, types, raw SQL, and specify which SQL clause they belong to.

```cpp
// Basic field in SELECT
query q1;
q1 << table_g("users")
   << field_g("name");

// Field with alias
query q2;
q2 << table_g("users")
   << field_g("name").as("username");

// Field with value (for INSERT/UPDATE)
query q3;
q3 << table_g("users")
   << field_g("name").value("John");

// Field with custom type
query q4;
q4 << table_g("users")
   << field_g("id").type("INTEGER");

// Field with raw SQL expression
query q5;
q5 << table_g("users")
   << field_g("created_at").raw("NOW()");

// Table-qualified field (for joins)
query q6;
q6 << table_g("users").as("u")
   << table_g("orders").as("o")
   << field_g("u", "name")
   << field_g("o", "amount");
```

**Field Builder Methods:**

| Method | Description | Example |
|--------|-------------|---------|
| `as("alias")` | Set field alias | `field_g("name").as("username")` |
| `value(val)` | Set value for INSERT/UPDATE | `field_g("id").value(123).insert()` |
| `type("TYPE")` | Set SQL data type | `field_g("id").type("INTEGER")` |
| `raw("SQL")` | Use raw SQL expression | `field_g("date").raw("NOW()")` |
| `select()` | Add to SELECT clause | `field_g("name").select()` |
| `insert()` | Add to INSERT clause | `field_g("name").value("John").insert()` |
| `update()` | Add to UPDATE clause | `field_g("name").value("Jane").update()` |
| `orderby()` | Add to ORDER BY clause | `field_g("name").orderby()` |
| `groupby()` | Add to GROUP BY clause | `field_g("category").groupby()` |
| `returning()` | Add to RETURNING clause | `field_g("id").returning()` |
| `asc()` / `desc()` | Set sort direction | `field_g("name").orderby().desc()` |
| `subselect()` | Mark as subselect | `field_g("count").raw("COUNT(*)").subselect()` |

### `fields_builder` — Multiple Fields at Once

Efficiently define multiple fields with a single builder, applying the same SQL clause to all.

```cpp
// Variadic field names
query q1;
q1 << table_g("users")
   << fields_g("users", "id", "name", "email", "age", "created_at").select();

// Name + alias pairs
query q2;
q2 << table_g("users")
   << fields_g("users", {{"id", "user_id"}, {"name", "username"}, {"age", "user_age"}}).select();

// Using vector of pairs
std::vector<std::pair<std::string_view, std::string_view>> vectorFields_ = {
    {"id", "user_id"},
    {"name", "username"},
    {"email", "user_email"}
};
query q3;
q3 << table_g("users")
   << fields_g("users", vectorFields_).select();

// Fields with values (for INSERT)
std::vector<std::pair<std::string_view, gd::variant_view>> vectorValues_ = {
    {"name", "John Doe"},
    {"email", "john@example.com"},
    {"age", 30}
};
query q4;
q4 << table_g("users")
   << fields_g("users", vectorValues_).insert();

// Setting default type for all fields
query q5;
q5 << table_g("users")
   << fields_g("users", "name", "email").select().type("VARCHAR(255)");
```

### `condition_builder` — WHERE/HAVING Conditions

Build complex conditions with comparison operators and logical grouping.

```cpp
// Equality
query q1;
q1 << table_g("users")
   << condition_g("id").value(1).eq();

// Comparisons
query q2;
q2 << table_g("users")
   << condition_g("age").value(18).gt();           // age > 18
   << condition_g("score").value(100).le().and_();  // AND score <= 100

// LIKE and IN
query q3;
q3 << table_g("users")
   << condition_g("email").value("%@example.com").like()
   << condition_g("status").value("'active','pending'").in().or_();

// IS NULL / IS NOT NULL
query q4;
q4 << table_g("users")
   << condition_g("deleted_at").is_null();

// BETWEEN
query q5;
q5 << table_g("users")
   << condition_g("created_at").between("2024-01-01", "2024-12-31");

// Raw SQL condition
query q6;
q6 << table_g("users")
   << condition_g("created_at").raw("> '2024-01-01'");

// Complex logical grouping
query q7;
q7 << table_g("users")
   << condition_g("name").value("John").eq()
   << condition_g("age").value(25).gt().and_()     // (name = 'John' AND age > 25)
   << condition_g("name").value("Jane").eq().or_(); // OR (name = 'Jane')
```

**Condition Builder Methods:**

| Method | Description | Example |
|--------|-------------|---------|
| `value(val)` | Set comparison value | `condition_g("id").value(123)` |
| `raw("SQL")` | Use raw SQL condition | `condition_g("date").raw("> NOW()")` |
| `type("TYPE")` | Set value type | `condition_g("id").type("INTEGER")` |
| `eq()` | Equal (=) | `condition_g("id").value(1).eq()` |
| `ne()` | Not equal (!=) | `condition_g("id").value(1).ne()` |
| `lt()` / `le()` | Less than / Less or equal | `condition_g("age").value(18).lt()` |
| `gt()` / `ge()` | Greater than / Greater or equal | `condition_g("age").value(18).gt()` |
| `like()` | LIKE pattern match | `condition_g("email").value("%@test.com").like()` |
| `in()` | IN list | `condition_g("status").value("'a','b'").in()` |
| `is_null()` / `is_not_null()` | IS NULL / IS NOT NULL | `condition_g("date").is_null()` |
| `between(lo, hi)` | BETWEEN range | `condition_g("age").between(18, 65)` |
| `and_()` / `or_()` / `not_()` | Logical operators | `condition_g("x").val(1).eq().and_()` |

---

## Advanced Features

### JOINs with Automatic Key Detection

```cpp
// Manual JOIN approach
query qManual;
qManual << table_g("customer")
        << table_g("contact").join("on customer.id = contact.customer_id")
        << table_g("orders").join("on customer.id = orders.customer_id")
        << field_g("id").raw("*").select();

std::cout << qManual.sql_get(eSqlSelect) << "\n";

// Automatic JOIN using key/fk relationships
query qAuto;
qAuto << table_g("customer").key("id")
      << table_g("contact").fk("customer_id").parent("customer")
      << table_g("orders").fk("customer_id").parent("customer")
      << field_g("id").raw("*").select();

std::cout << qAuto.sql_get(eSqlSelect) << "\n";
// Both produce equivalent SQL
```

### DISTINCT Clause

```cpp
// Simple DISTINCT
query q1;
q1.distinct(true)
   << table_g("users")
   << field_g("*").select();

// PostgreSQL DISTINCT ON
query q2;
q2.distinct("DISTINCT ON (customer.id)")
   << table_g("customer").key("id")
   << field_g("customer", "id").raw("'postgres sample' AS description")
   << table_g("contact").fk("customer_id").parent("customer")
   << field_g("*").select();
```

### Subselects in Fields

```cpp
query q;
q << table_g("customer").key("id")
   << table_g("contact").fk("customer_id").parent("customer")
   << field_g("contact", "id")
       .as("project_count")
       .raw("COUNT(*)")
       .from("project p1")
       .where("p1.project_id = customer.id")
       .subselect()
       .asc()
   << field_g("id").as("customer_id").desc().select();

std::cout << q.get_select() << "\n";
// Produces: SELECT (SELECT COUNT(*) FROM project p1 WHERE p1.project_id = customer.id) AS project_count ASC, id AS customer_id DESC ...
```

### Custom ORDER BY Expressions

```cpp
query q;
q << table_g("customer").key("id")
   << table_g("contact").fk("customer_id").parent("customer")
   << field_g("id").as("customer_id")
   << field_g("name").as("customer_name")
   << field_g("subselect_field").order("(SELECT COUNT(*) FROM project p1 WHERE p1.project_id = customer.id)")
   << field_g("id").select();

std::cout << q.get_select() << "\n";
// Includes: ORDER BY (SELECT COUNT(*) FROM project p1 WHERE p1.project_id = customer.id)
```

### Database Dialect Support

```cpp
// SQLite (default)
query qSqlite(eSqlDialectSqlite);
qSqlite << table_g("users")
        << field_g("uuid").value("3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d").insert();

// SQL Server
query qSqlServer(eSqlDialectSqlServer);
qSqlServer << table_g("users")
           << field_g("uuid").value("3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d").insert();

// PostgreSQL
query qPostgres(eSqlDialectPostgreSql);
qPostgres << table_g("users")
          << field_g("uuid").value("3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d").insert();

// Oracle
query qOracle(eSqlDialectOracle);
qOracle << table_g("users")
         << field_g("uuid").value("3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d").insert();

std::cout << "SQLite:    " << qSqlite.sql_get(eSqlInsert) << "\n";
std::cout << "SQLServer: " << qSqlServer.sql_get(eSqlInsert) << "\n";
std::cout << "Postgres:  " << qPostgres.sql_get(eSqlInsert) << "\n";
std::cout << "Oracle:    " << qOracle.sql_get(eSqlInsert) << "\n";
```

### Type-Specific Value Handling

```cpp
std::vector<std::pair<std::string_view, gd::variant_view>> vectorValues_ = {
    {"id", "f47ac10b58cc4372a5670e02b2c3d479"},
    {"id2", "1e3f7b8a9d2c4f6e8b1a5c7d9e3f2a1b"}
};

// Hex values treated as string (quoted)
query q1;
q1 << table_g("users")
   << fields_g("users", vectorValues_).insert().type("string");
std::cout << q1.get_insert() << "\n";

// Hex values treated as binary (database-specific hex encoding)
query q2;
q2 << table_g("users")
   << fields_g("users", vectorValues_).insert().type("binary");
std::cout << q2.get_insert() << "\n";

// Hex values treated as number (no quoting)
query q3;
q3 << table_g("users")
   << fields_g("users", vectorValues_).insert().type("int32");
std::cout << q3.get_insert() << "\n";
```

### Limit and Offset

```cpp
query q;
q << table_g("users")
  << field_g("*").select();
q.set_limit(10, 20);  // LIMIT 10 OFFSET 20

std::cout << q.sql_get(eSqlSelect) << "\n";
```

---

## Complete Working Examples

### Example 1: Complex SELECT with Joins

```cpp
using namespace gd::sql;

query q;
q << table_g("users").as("u")
   << table_g("orders").as("o").join("LEFT JOIN orders ON u.id = o.user_id")
   << table_g("products").as("p").join("LEFT JOIN products ON o.product_id = p.id")
   << field_g("u", "id").as("user_id").select()
   << field_g("u", "name").as("user_name").select()
   << field_g("o", "amount").as("order_amount").select()
   << field_g("p", "name").as("product_name").select()
   << field_g("o", "created_at").orderby().desc()
   << condition_g("u", "active").value(1).eq()
   << condition_g("o", "amount").value(100).gt().and_();

std::cout << q.sql_get(eSqlSelect) << "\n";
/*
SELECT u.id AS user_id, u.name AS user_name, o.amount AS order_amount, p.name AS product_name
FROM users u
LEFT JOIN orders ON u.id = o.user_id
LEFT JOIN products ON o.product_id = p.id
WHERE u.active = 1 AND o.amount > 100
ORDER BY o.created_at DESC
*/
```

### Example 2: Dynamic Query Building

```cpp
using namespace gd::sql;

// Function that builds a query based on runtime conditions
query build_user_query(bool bIncludeActive, int iMinAge, std::string_view stringSearch)
{
   query q;
   
   // Base query
   q << table_g("users")
     << fields_g("users", "id", "name", "email", "age").select();
   
   // Optional conditions
   if( bIncludeActive )
   {
      q << condition_g("active").value(1).eq().and_();
   }
   
   if( iMinAge > 0 )
   {
      q << condition_g("age").value(iMinAge).ge().and_();
   }
   
   if( stringSearch.empty() == false )
   {
      q << condition_g("name").value(stringSearch).like();
   }
   
   return q;
}

// Usage
auto q1 = build_user_query(true, 18, "%John%");
std::cout << q1.sql_get(eSqlSelect) << "\n";
```

### Example 3: INSERT with Returning (PostgreSQL)

```cpp
using namespace gd::sql;

query q(eSqlDialectPostgreSql);
q << table_g("users")
   << field_g("name").value("John Doe").insert()
   << field_g("email").value("john@example.com").insert()
   << field_g("age").value(30).insert()
   << field_g("id").returning();

std::cout << q.sql_get(eSqlInsert) << "\n";
// Output: INSERT INTO users (name, email, age) VALUES ('John Doe', 'john@example.com', 30) RETURNING id
```

### Example 4: GROUP BY with HAVING

```cpp
using namespace gd::sql;

query q;
q << table_g("orders")
   << field_g("user_id").groupby()
   << field_g("COUNT(*)").as("order_count").select()
   << field_g("SUM(amount)").as("total_amount").select()
   << condition_g("COUNT(*)").value(5).gt().and_();  // HAVING

std::cout << q.sql_get(eSqlSelect) << "\n";
/*
SELECT user_id, COUNT(*) AS order_count, SUM(amount) AS total_amount
FROM orders
GROUP BY user_id
HAVING COUNT(*) > 5
*/
```

---

## Best Practices

### 1. Use Fluent Chaining for Readability

```cpp
// Good - fluent and readable
query q;
q << table_g("users").as("u")
   << table_g("orders").as("o").join("LEFT JOIN orders ON u.id = o.user_id")
   << fields_g("u", "id", "name").select()
   << field_g("o", "amount").select();

// Avoid - scattered statements
query q2;
table_builder tableUsers = table_g("users").as("u");
q2 << tableUsers;
table_builder tableOrders = table_g("orders").as("o").join("LEFT JOIN orders ON u.id = o.user_id");
q2 << tableOrders;
```

### 2. Use `fields_g()` for Multiple Fields

```cpp
// Good - concise
q << fields_g("users", "id", "name", "email", "age", "created_at").select();

// Verbose - individual fields
q << field_g("id")
   << field_g("name")
   << field_g("email")
   << field_g("age")
   << field_g("created_at")
```

### 3. Use Automatic JOIN Relationships

```cpp
// Good - maintainable
q << table_g("orders").key("id")
   << table_g("users").fk("user_id").parent("orders")
   << table_g("products").fk("product_id").parent("orders");

// Works but harder to maintain
q << table_g("orders")
   << table_g("users").join("LEFT JOIN users ON orders.user_id = users.id")
   << table_g("products").join("LEFT JOIN products ON orders.product_id = products.id");
```

### 4. Specify Types for Complex Values

```cpp
// Good - explicit type
query q(eSqlDialectPostgreSql);
q << table_g("users")
   << field_g("id")
       .value("f47ac10b58cc4372a5670e02b2c3d479")
       .type("uuid")
       .insert();

// May produce incorrect SQL depending on dialect
query q2;
q2 << table_g("users")
    << field_g("id").value("f47ac10b58cc4372a5670e02b2c3d479");
```

### 5. Use Query Reuse

```cpp
// Build base query once
query qBase;
qBase << table_g("users")
      << fields_g("users", "id", "name", "email").select();

// Add conditions dynamically
query qFiltered1 = qBase;
qFiltered1 << condition_g("active").value(1).eq();

query qFiltered2 = qBase;
qFiltered2 << condition_g("age").value(18).gt()
            << condition_g("age").value(65).le();
```

---

## Summary

The `gd::sql::query` builder provides a powerful, type-safe way to construct SQL queries:

- **Fluent Interface**: Chain method calls for readable query construction
- **Multiple Builders**: `table_g()`, `field_g()`, `fields_g()`, `condition_g()` for different query components
- **SQL Clauses**: `select()`, `insert()`, `update()`, `orderby()`, `groupby()`, `returning()`
- **Comparison Ops**: `eq()`, `ne()`, `lt()`, `gt()`, `like()`, `in()`, `is_null()`, etc.
- **Logical Grouping**: `and_()`, `or_()`, `not_()` for complex conditions
- **Database Support**: SQLite, PostgreSQL, SQL Server, Oracle, MySQL
- **Advanced Features**: JOINs, subselects, DISTINCT, custom expressions

Use this builder whenever you need to construct SQL queries programmatically, especially when:
- Building dynamic queries based on runtime conditions
- Supporting multiple database backends
- Improving query maintainability and readability
- Reducing SQL injection risks through proper value handling

For complex, performance-critical queries that require database-specific optimizations, raw SQL may still be the best choice—but the builder always provides the option to inject raw SQL where needed.
