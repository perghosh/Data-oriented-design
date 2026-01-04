### Overview: `gd::variant` – A Flexible, Type-Safe Runtime Value Holder

The `gd::variant` class is a powerful, lightweight C++ variant type designed to store **any primitive value** (numbers, bool, pointers) or **common dynamic types** (strings, binary data, GUIDs) in a single object, while owning its data and managing memory automatically.

It's similar in spirit to `std::variant` but more practical for real-world use: it supports dynamic allocation for variable-sized data (like strings), runtime type queries, safe conversions, and easy debugging — all in just **16 bytes**.

#### Key Features
- Stores one value of many types (int, double, string, binary, etc.)
- Automatically manages heap allocation for large/string data
- Rich type inspection (`is_int()`, `type_name()`, etc.)
- Safe retrieval and conversion (`as<T>()`, `as_string()`, `convert()`)
- Supports arithmetic, comparison, and assignment for compatible types
- Copy/move semantics with proper deep copying

#### Basic Usage

```cpp
#include "gd_variant.h"
using namespace gd;

int main() {
    variant v1 = 42;                  // int32_t
    variant v2 = 3.14;                 // double
    variant v3 = "Hello World";       // owns a copied string
    variant v4 = true;                 // bool
    variant v5 = std::string("Owned"); // also owns the string

    // Type inspection
    std::cout << v1.type_name() << "\n";  // "int32"
    std::cout << v3.is_string() << "\n";  // true

    // Safe retrieval using as<T>()
    int         i = v1.as<int>();         // 42
    double      d = v2.as<double>();      // 3.14
    std::string s = v3.as<std::string>(); // "Hello World"
    bool        b = v4.as<bool>();        // true

    // Or classic getters
    std::string text = v3.as_string();    // same as above

    // Arithmetic (only on numeric types)
    variant sum = v1 + 100;                // variant(142)
    v1 += 8;                               // v1 now holds 50

    // Comparison
    if (v1 > variant(40)) { /* true */ }

    // Conversion between types
    variant floatVar = 123;
    floatVar.convert(variant_type::eTypeCDouble); // now a double ≈ 123.0
    double val = floatVar.as<double>();

    return 0;
}
```

#### Construction Options

```cpp
variant v1(123);                     // int32_t
variant v2("text");                  // allocates and owns copy
variant v3("temp", 4, true);         // non-owning view-like (raw pointer)
variant v4(utf8("utf8 text"));       // explicit UTF-8 string
variant v5(binary_data, length);     // binary blob (allocates copy)
```

Use the third `bool` parameter or tag types (`tag_no_allocate`, `tag_raw`) to create **non-owning** variants when you don't want allocation.

#### Type Queries & Safety

```cpp
if (v.is_int())     { int i = v.as<int>(); }
if (v.is_string())  { std::string s = v.as_string(); }
if (v.is_null())    { /* empty/unknown */ }

std::cout << v.type_name(); // human-readable: "string", "int64", "binary", etc.
```

#### Memory Management

- Small values (numbers, pointers) are stored inline — no allocation.
- Large values (strings, binary) trigger heap allocation with `eFlagAllocate`.
- Destructor and assignment automatically free owned memory via `clear()`.

#### Advanced: Non-Owning View (`variant_view`)

The header mentions a forward-declared `variant_view` class — it has the **same layout** but does **not own** data. You can safely cast `variant*` → `variant_view*` when you want read-only access without copying.

#### Why Use `gd::variant`?

- Simple, fast, and predictable (fixed 16-byte size)
- Great for configuration, database rows, JSON-like data, scripting bridges
- Better than `std::any` (faster type checks) or `std::variant` (supports dynamic sizes)

That's it! With `gd::variant`, you get the flexibility of dynamic typing with the safety and performance of modern C++. Perfect for heterogeneous data handling.
