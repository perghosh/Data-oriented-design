### Short Tutorial: `gd::arg`, `gd::arg_view`, `gd::args`, and `gd::args_view` – Flexible Key-Value Argument Containers

These four lightweight classes form a powerful system for handling **named arguments** (key-value pairs) in C++, similar to Python's keyword arguments or JavaScript object literals, but with strong typing, zero-overhead views, and full integration with `gd::variant`/`variant_view`.

They come in two pairs:

- **`arg_view` + `args_view`** → **Non-owning**: perfect for temporary, read-only, zero-cost argument passing.
- **`arg` + `args`** → **Owning**: stores copies of keys and values, suitable for long-lived storage or modification.

#### Core Building Blocks

```cpp
struct arg_view {               // Non-owning key-value pair
    std::string_view   key;     // view into existing string
    gd::variant_view   value;   // view into existing variant data
};

struct arg {                    // Owning key-value pair
    std::string        key;     // owns the string
    gd::variant        value;   // owns the variant data
};
```

Both support:
- Construction from key/value
- `get_key()`, `get_value()`, `first()`, `second()`
- Equality comparison
- Conversion between owning ↔ non-owning

#### Containers

```cpp
struct args_view : std::vector<arg_view> { ... }  // Non-owning collection
struct args      : std::vector<arg>      { ... }  // Owning collection
```

Both containers provide:
- Full STL-like interface (`begin()/end()`, `push_back()`, `emplace_back()`, `operator[]`, etc.)
- `find(key)`, `contains(key)`, `count(key)`
- Algorithm helpers: `find_if()`, `any_of()`, `all_of()`, `none_of()`
- Easy conversion between owning and non-owning

#### Basic Usage Examples

```cpp
#include "gd_arguments.h"
using namespace gd;

int main() {
    // 1. Simple owning arguments (most common for storage)
    args params = {
        {"name", "Alice"},
        {"age", 30},
        {"active", true},
        {"score", 95.5}
    };

    // Access by index or find by key
    std::string name = params[0].get_value().as<std::string>();
    auto it = params.find("age");
    if (it != params.end()) {
        int age = it->get_value().as<int>();  // 30
    }

    // Modify
    params.find("score")->get_value() = 100.0;

    // 2. Non-owning views — zero copy, perfect for function args
    void process(const args_view& options);

    std::string key1 = "mode", val1 = "fast";
    std::string key2 = "debug", val2 = "true";

    args_view view_options = {
        {key1, variant_view(val1)},
        {key2, variant_view(val2)}
    };

    process(view_options);  // No allocation, no copying!

    // 3. Easy creation with helpers
    auto args1 = make_args({
        make_arg("x", 10),
        make_arg("y", 20.0)
    });

    auto view_args = make_args_view({
        make_arg_view("color", variant_view("red")),
        make_arg_view("size", variant_view(42))
    });

    // 4. Useful utility functions
    variant_view value = find_value(view_options, "color");  // safe lookup
    variant defaulted = get_value_or(params, "timeout", variant(5000));

    bool has_mode = has_key(view_options, "mode");
    auto keys = get_keys(params);  // std::vector<std::string>

    // 5. Convert between owning and views
    args owned = to_args(view_options);           // deep copy
    args_view temp_view = owned;                  // implicit conversion (no copy!)

    return 0;
}
```

#### When to Use Which

| Scenario                                   | Recommended Type         | Why |
|--------------------------------------------|--------------------------|-----|
| Function parameters (read-only)            | `args_view`              | Zero overhead, safe |
| Temporary argument lists                   | `args_view`              | No allocation |
| Parsing config/command line (initial pass) | `args_view`              | Fast, non-owning |
| Storing settings long-term                 | `args`                   | Owns data safely |
| Modifying arguments after creation         | `args`                   | Mutable values |
| Returning arguments from function          | `args`                   | Ownership transfer |

#### Advanced Tips

- Use initializer lists for clean, readable code
- `args_view` can be constructed directly from existing `std::string` and `variant_view` objects without copying
- `args` automatically converts to `args_view` when needed
- All classes are lightweight and exception-safe
- Great for configuration systems, SQL parameter binding, API calls, event data

#### Summary

This argument system gives you the **flexibility of named parameters** with the **performance and safety of modern C++**:

- Write clear, self-documenting code: `func({{"mode", "fast"}, {"count", 100}})`
- Zero-cost abstraction when you don't need ownership
- Full ownership when you do
- Seamless integration with `gd::variant` for any value type

Perfect for configuration, command-line parsing, database queries, event systems, and any API that benefits from named parameters!