### Short Tutorial: `gd::argument::shared::arguments` – High-Performance, Reference-Counted Key-Value Buffer

The `gd::argument::shared::arguments` class is the **high-performance variant** of the arguments buffer system. It uses **reference-counted shared buffers** with **aligned 32-bit headers** for faster access and better cache locality, at the cost of slightly higher memory usage and no stack-only mode.

It's ideal for:
- Scenarios with many copies of large argument sets (reference counting avoids deep copies)
- Performance-critical code where alignment and fast traversal matter
- Long-lived or frequently shared data structures
- When you need copy-on-write semantics

#### Key Differences from `gd::argument::arguments`

| Feature                          | `arguments` (compact)               | `shared::arguments` (this one)              |
|----------------------------------|-------------------------------------|---------------------------------------------|
| Buffer layout                    | Packed, minimal padding             | 32-bit aligned headers                      |
| Memory overhead                  | Lowest possible                     | Slightly higher (better alignment)          |
| Stack-only support               | Yes (external buffer)               | No (always heap + refcount)                 |
| Copy semantics                   | Deep copy                           | Reference-counted (CoW)                     |
| Access speed                     | Good                                | **Faster** (aligned reads)                  |
| Best for                         | Embedded, tight memory              | General high-performance use                |

#### Core Concepts

- **Shared buffer with reference counting**: Multiple `arguments` objects can share the same data until one modifies it (then it clones).
- **Aligned layout**: `[name_len+type (u32)][name][value_len+type (u32)][value]` → faster pointer arithmetic and memory access.
- **Copy-on-write**: Cheap copies, expensive only on mutation.

#### Basic Usage

```cpp
#include "gd_arguments_shared.h"
using namespace gd::argument::shared;

int main() {
    // Create and populate
    arguments args;
    args.append("name", "Alice");
    args.append("age", 30);
    args.append("score", 95.5);
    args.append("active", true);

    // Cheap copy — shares underlying buffer!
    arguments args2 = args;           // refcount = 2, no data copy
    arguments args3 = std::move(args); // args now empty

    // Modify → triggers clone (copy-on-write)
    args2["age"] = 31;                // args2 now has its own buffer

    // Access
    std::string name = args["name"].as<std::string>();
    int age = args2["age"].get<int>();

    // Safe lookup with default
    double level = args.get_argument("level", 1.0);

    // Iteration with structured binding (C++17+)
    for (auto [key, value] : args.named()) {
        std::cout << key << ": " << value.as_string() << "\n";
    }

    // Raw pointer iteration (fast)
    for (auto pos = args.next(); pos; pos = args.next(pos)) {
        auto name = arguments::get_name_s(pos);
        auto val = arguments::get_argument_s(pos);
        std::cout << name << " = " << val.as_string() << "\n";
    }

    return 0;
}
```

#### Construction & Copy Behavior

```cpp
arguments a1 = { {"x", 10}, {"y", 20} };  // heap + refcount

arguments a2 = a1;        // shares buffer (fast!)
arguments a3 = std::move(a1); // a1 becomes empty

a2.append("z", 30);       // triggers clone — a2 now unique
// a3 still shares original data unchanged
```

#### When to Use `shared::arguments`

| Scenario                                 | Use `shared::arguments`? | Why |
|------------------------------------------|--------------------------|-----|
| Many copies of same data                 | Yes                      | Reference counting saves memory/time |
| Frequent sharing between objects/functions | Yes                      | CoW semantics |
| Need maximum traversal speed             | Yes                      | Aligned access |
| Memory extremely tight / embedded        | No                       | Use compact version |
| Want stack-only storage                  | No                       | Not supported |

#### Summary

`gd::argument::shared::arguments` is the **performance-optimized** sibling in the arguments family:

- Same powerful API as the compact version
- Reference-counted sharing for efficient copying
- Aligned layout for faster access
- Automatic copy-on-write

Use it when you have **multiple instances** of argument data or need **maximum speed** in traversal and lookup — it's the default choice for most general-purpose applications.

Together with the compact version, it gives you full control over the memory/performance trade-off in heterogeneous data handling.
