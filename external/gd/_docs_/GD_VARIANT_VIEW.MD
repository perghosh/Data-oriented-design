### Overview: `gd::variant_view` – A Non-Owning, Lightweight Runtime Value View

The `gd::variant_view` class is a **non-owning counterpart** to `gd::variant`. It has the **exact same memory layout (16 bytes)** and supports nearly all the same operations — type inspection, safe retrieval, comparison, arithmetic — but **does not allocate or free any memory**.

It's designed for **read-only access** to data that lives elsewhere (e.g., string literals, stack buffers, `gd::variant` objects, database rows), making it perfect for high-performance parsing, serialization, or temporary views without copying.

#### Key Differences from `gd::variant`

| Feature                  | `gd::variant`                     | `gd::variant_view`                     |
|--------------------------|-----------------------------------|----------------------------------------|
| Owns data                | Yes (deep copies strings/binary)  | No (only references external data)     |
| Heap allocation          | Yes (for strings, binary, etc.)   | Never                                  |
| Destructor behavior      | Frees owned memory                | Does nothing                           |
| Construction from literals | Copies                            | Direct view (no copy)                  |
| Binary compatible        | Yes                               | Yes — can safely cast between them     |

#### Basic Usage

```cpp
#include "gd_variant.h"
using namespace gd;

int main() {
    // Direct views — no allocation or copying!
    variant_view v1 = 42;                     // int32_t
    variant_view v2 = 3.14;                    // double
    variant_view v3 = "Hello World";           // points directly to literal
    variant_view v4 = true;                    // bool

    std::string owned = "Owned string";
    variant_view v5 = owned;                   // views the string's data
    variant_view v6 = std::string_view("temp"); // also a view

    // Type inspection (same as variant)
    std::cout << v3.type_name() << "\n";       // "string"
    std::cout << v3.is_string() << "\n";       // true

    // Safe retrieval using as<T>()
    int         i = v1.as<int>();              // 42
    double      d = v2.as<double>();           // 3.14
    std::string s = v3.as<std::string>();      // copies only when needed
    std::string_view sv = v3.as<std::string_view>(); // zero-cost view!

    // Direct access (fast!)
    std::cout << v3.c_str() << "\n";           // "Hello World"
    std::cout << v3.get_string_view() << "\n"; // std::string_view

    // Arithmetic and comparison
    if (v1 > variant_view(30)) { /* true */ }
    auto sum = v1 + 10;                        // returns int32_t

    // Convert to owning variant when needed
    variant ownedCopy = v3.as_variant();       // deep copy now happens

    return 0;
}
```

#### Construction Options (Non-Owning by Default)

```cpp
variant_view v1("text");                    // views null-terminated string
variant_view v2("data", 4);                 // views exactly 4 chars
variant_view v3(buffer, length);            // binary data view
variant_view v4(utf8("utf8 text"));         // explicit UTF-8 view

// From existing variant (clears allocation flag!)
variant owned = "owned";
variant_view view_from_variant(owned);      // safe, binary-compatible cast
```

#### Interoperability with `gd::variant`

Because they have **identical layout**, you can safely convert:

```cpp
variant v = "Hello";
variant_view vv(v);              // direct binary reinterpretation (clears alloc flag)
variant back = vv.as_variant();  // creates owning copy

// Static helper functions
variant_view::copy_s(vv, v);     // view → owning variant
variant_view::copy_s(v, vv);     // owning variant → view (no copy of data!)
```

#### When to Use `variant_view`

- Parsing configuration files, JSON, CSV, database results
- Temporary views into large data structures
- High-performance loops over heterogeneous data
- Passing read-only values across API boundaries
- Avoiding unnecessary copies when you already own the data

#### Performance Tips

- Prefer `as<std::string_view>()` or `get_string_view()` — zero-cost
- Use `c_str()` or `data()` for direct buffer access
- Never store `variant_view` to data that goes out of scope!

#### Summary

`gd::variant_view` gives you **all the convenience and safety** of `gd::variant` — type queries, conversions, formatting, debugging — but with **zero overhead** and **no ownership**.

Use `variant` when you need to own and manage values.  
Use `variant_view` when you just want to **look at** values safely and efficiently.

Together, they form a powerful duo for handling dynamic, heterogeneous data in modern C++.
