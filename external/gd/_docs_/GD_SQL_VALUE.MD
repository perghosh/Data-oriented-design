[← TOC](TOC.md)

### Short Tutorial: `gd::sql` – Safe SQL Value Formatting and Templating System

The `gd::sql` namespace provides a **comprehensive, type-safe SQL value formatting and templating system**. It automatically handles SQL escaping, type conversion, and parameter substitution, helping you prevent SQL injection while building complex queries dynamically.

It's ideal for:
- Building dynamic SQL queries with type-safe parameter binding
- Formatting variant values (numbers, strings, binary, GUIDs) for SQL statements
- Parameterized bulk inserts and updates
- SQL template replacement with conditional expressions
- Cross-platform SQL generation with proper escaping

#### Core Components

| Component | Purpose |
|-----------|---------|
| `append_g()` | Format `variant`/`variant_view` values as SQL literals |
| `append_ascii()` | Convert and escape ASCII text as UTF8 SQL strings |
| `append_utf8()` | Escape UTF8 text for SQL (handles quotes) |
| `append_binary()` | Convert binary data to hexadecimal representation |
| `make_bulk_g()` | Generate bulk parameterized SQL (for multi-row inserts) |
| `replace_g()` | Template replacement with brace syntax `{...}` |


#### Core Concepts

- **Automatic SQL Escaping**: Single quotes in strings are automatically doubled (`'` → `''`)
- **Type-Aware Formatting**: Each variant type is formatted appropriately for SQL (numbers unquoted, strings quoted, binary as hex)
- **Template Replacement**: Use brace syntax `{name}` or `{index}` for named or positional parameters
- **Raw Mode**: Prefix with `=` to insert values without SQL formatting (`{=value}`)
- **Required Values**: Prefix with `*` to enforce presence (`{*name}`)
- **Conditional Expressions**: Use `{?expression;true;false}` for conditional SQL generation

#### Basic Usage: Value Formatting

```cpp
#include "gd_sql_value.h"
#include "gd_variant.h"

using namespace gd;

int main() {
    std::string sql;
    
    // Simple string with automatic escaping
    append_ascii((uint8_t*)R"(O'Reilly's book)", sql);
    // Result: 'O''Reilly''s book'
    
    // Number formatting
    variant v_int = 42;
    append_g(v_int, sql);
    // Result: 42
    
    // String value
    variant v_str = "Hello";
    append_g(v_str, sql);
    // Result: 'Hello'
    
    // Binary data as hex
    uint8_t guid[16] = {0x01, 0x02, ...};
    variant v_binary = variant_view::binary(guid, 16);
    append_g(v_binary, sql);
    // Result: 0102030405060708090A0B0C0D0E0F10
    
    // NULL handling
    variant v_null;
    append_g(v_null, sql);
    // Result: NULL
    
    return 0;
}
```

#### Template Replacement with Named Arguments

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    arguments args;
    args.append("name", "Alice");
    args.append("age", 30);
    args.append("table", "users");
    
    // Simple template replacement
    std::string sql = replace_g(
        "SELECT * FROM {table} WHERE name = {name} AND age > {age}",
        args,
        tag_brace{}
    );
    // Result: SELECT * FROM 'users' WHERE name = 'Alice' AND age > 30
    
    return 0;
}
```

#### Positional Arguments

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    arguments args;
    args.append(10);      // index 0
    args.append(20);      // index 1
    args.append(30);      // index 2
    
    // Using empty braces for positional args
    std::string sql = replace_g(
        "SELECT * FROM t WHERE x > {} AND y < {} OR z = {}",
        args,
        tag_brace{}
    );
    // Result: SELECT * FROM t WHERE x > 10 AND y < 20 OR z = 30
    
    // Or using numeric indices
    sql = replace_g(
        "VALUES ({0}, {1}, {0})",
        args,
        tag_brace{}
    );
    // Result: VALUES (10, 20, 10)
    
    return 0;
}
```

#### Advanced: Required and Raw Values

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    arguments args;
    args.append("name", "Alice");
    
    // Required values - return error if missing
    auto [success, error] = replace_g(
        "SELECT * FROM users WHERE name = {*name} AND id = {*id}",
        args,
        sql_out,
        tag_brace{}
    );
    // success = false, error = "required value not found: id"
    
    // Raw values - insert without SQL formatting
    args.append("column_name", "my_column");
    std::string sql = replace_g(
        "SELECT {=column_name} FROM table",
        args,
        tag_brace{}
    );
    // Result: SELECT my_column FROM table (note: no quotes around my_column)
    
    return 0;
}
```

#### Keep Not Found - Multiple Pass Replacement

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    // First pass - partial replacement
    arguments args1;
    args1.append("name", "Alice");
    
    std::string sql = replace_g(
        "SELECT {name}, {age}, {city} FROM users",
        args1,
        tag_brace{},
        tag_keep_not_found{}
    );
    // Result: SELECT 'Alice', {age}, {city} FROM users
    
    // Second pass - complete replacement
    arguments args2;
    args2.append("age", 30);
    args2.append("city", "NYC");
    
    sql = replace_g(sql, args2, tag_brace{}, tag_keep_not_found{});
    // Result: SELECT 'Alice', 30, 'NYC' FROM users
    
    return 0;
}
```

#### Conditional Expressions (Preprocess Mode)

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    arguments args;
    args.append("is_admin", true);
    args.append("user_id", 42);
    
    // Conditional SQL based on boolean value
    std::string sql = replace_g(
        stringSource,
        [&args](const std::string_view& name) -> variant_view {
            return args[name].as_variant_view();
        },
        nullptr,
        tag_preprocess{}
    );
    
    // With template: "{?is_admin;SELECT * FROM admin_table;SELECT * FROM user_table} WHERE id = {user_id}"
    // If is_admin is true:  SELECT * FROM admin_table WHERE id = 42
    // If is_admin is false: SELECT * FROM user_table WHERE id = 42
    
    return 0;
}
```

The preprocess syntax is:
```
{?expression;true_value;false_value}
```

- `expression` - name or expression to evaluate
- `true_value` - SQL to include if expression is true/truthy
- `false_value` - (optional) SQL to include if expression is false/null

#### Bulk Operations

```cpp
using namespace gd::sql;

int main() {
    // Generate bulk insert SQL
    auto [sectionCount, bulkSql, tailSql] = make_bulk_g(
        "(?, ?, ?, ?)",    // Fixed part per row (parameter placeholders)
        "?, ?, ?, ?",      // Parameter template per row
        150,               // Total count
        100                // Bulk count (rows per batch)
    );
    
    // sectionCount = 1 (one full bulk section)
    // bulkSql = "(?, ?, ?, ?), (?, ?, ?, ?), ... repeated 100 times"
    // tailSql = "(?, ?, ?, ?), ... repeated 50 times (remaining rows)
    
    // Usage:
    std::string fullSql = "INSERT INTO users (id, name, age, city) VALUES " + bulkSql;
    if (sectionCount > 0) {
        // Execute bulkSql with 100 * 4 = 400 parameters
    }
    if (!tailSql.empty()) {
        // Execute tailSql with 50 * 4 = 200 parameters
    }
    
    return 0;
}
```

#### Type-Specific Formatting Behavior

| Variant Type | SQL Format | Example |
|--------------|------------|---------|
| `unknown/null` | `NULL` | `NULL` |
| `bool`/`bit` | `0` or `1` | `1` |
| Integers (8/16/32/64 bit) | Unquoted | `42`, `-100` |
| Float | Unquoted with %f | `3.141593` |
| Double | Unquoted with %.17g | `3.141592653589793` |
| String | Quoted, escaped | `'O''Reilly'` |
| UTF8 String | Quoted, escaped | `'café'` |
| Binary | Hex string | `0102030405...` |
| GUID | Hex string (16 bytes) | `0102030405060708090A0B0C0D0E0F10` |

#### Raw vs Normal Mode

```cpp
using namespace gd::sql;

variant_view column_name = "my_table";
variant_view value = "Alice's";

std::string sql;

// Normal mode - SQL-safe
append_g(column_name, sql);      // Result: 'my_table' (quoted)
append_g(value, sql);            // Result: 'Alice''s' (escaped)

sql.clear();

// Raw mode - literal insertion
append_g(column_name, sql, tag_raw{});  // Result: my_table (no quotes)
append_g(value, sql, tag_raw{});        // Result: Alice's (no escaping)
```

Use `tag_raw` for:
- Table/column names in SQL
- SQL keywords (e.g., `ASC`, `DESC`)
- Already-escaped SQL literals

#### Error Handling

```cpp
using namespace gd::argument;
using namespace gd::sql;

int main() {
    arguments args;
    args.append("name", "Alice");
    
    std::string sql_out;
    
    // Check for errors in replacement
    auto [success, error] = replace_g(
        "SELECT * FROM {name} WHERE id = {*missing_id}",
        args,
        sql_out,
        tag_brace{}
    );
    
    if (!success) {
        std::cerr << "Error: " << error << std::endl;
        // Output: Error: required value not found: missing_id
    }
    
    return 0;
}
```

#### Complete Example: Building a Complex Query

```cpp
#include "gd_sql_value.h"
#include "gd_arguments.h"

using namespace gd::argument;
using namespace gd::sql;

std::string build_search_query(const arguments& params) {
    // Define template with conditional expressions
    const char* template_sql = 
        R"(SELECT {=columns} 
           FROM {=table}
           {?has_name_filter;WHERE name = {name};}
           {?has_age_filter;{?has_name_filter;AND ;WHERE }age > {age_min};}
           LIMIT {limit})";
    
    return replace_g(template_sql, params, tag_preprocess{});
}

int main() {
    arguments params;
    
    // Set table and columns (raw - not quoted)
    params.append("table", variant_view::string("users"));
    params.append("columns", variant_view::string("id, name, email"));
    
    // Set filters (conditional)
    params.append("has_name_filter", false);
    params.append("has_age_filter", true);
    params.append("age_min", 18);
    
    // Set limit
    params.append("limit", 100);
    
    std::string sql = build_search_query(params);
    // Result: 
    // SELECT id, name, email 
    // FROM users
    // WHERE age > 18
    // LIMIT 100
    
    return 0;
}
```

#### Performance Considerations

- **Bulk Operations**: Use `make_bulk_g` for multi-row inserts to minimize round trips
- **String Reuse**: `std::string&` parameters in `append_g` avoid allocations
- **Stack Allocation**: Binary conversion uses stack for buffers < 1KB
- **Precompiled Templates**: Reuse template strings and only vary the arguments

#### Summary

The `gd::sql` system provides a **safe, flexible, and efficient** way to build SQL statements:

- **Type-safe** value formatting from `variant`/`variant_view`
- **Automatic escaping** prevents SQL injection vulnerabilities
- **Template-based** replacement with named and positional arguments
- **Conditional expressions** for dynamic SQL generation
- **Bulk operation support** for high-throughput scenarios
- **Raw mode** for table names, column names, and already-escaped values

Together with the `gd::argument` system, it forms a powerful foundation for building database applications with type safety and security. Whether you need simple parameterized queries or complex dynamic SQL with conditional logic, `gd::sql` has you covered.
