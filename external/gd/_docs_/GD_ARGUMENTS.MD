### Short Tutorial: `gd::argument::arguments` – Ultra-Compact, Stack-Friendly Key-Value Buffer

The `gd::argument::arguments` class is a **highly optimized, contiguous byte buffer** designed to store **named or unnamed values** (key-value pairs) with **minimal memory overhead** and **no internal alignment padding**.

It's perfect for:
- Embedding structured data in limited memory (embedded systems, network packets)
- High-performance serialization/deserialization
- Stack-based temporary argument lists (no heap unless needed)
- Replacing `std::vector<std::pair<std::string, variant>>` when size and speed matter

#### Key Design Features

- **Zero overhead per element**: No per-argument object headers
- **Packed layout**: `[name_length (u16)][name][type (u8)][optional length (u32)][data]`
- **Optional names**: Unnamed values allowed (great for arrays)
- **Supports all `gd::variant` types**: primitives, strings, binary, GUIDs
- **Can use external buffer** (stack array) → zero heap allocation
- **Fast lookup** by name or index
- **Iterators** (basic and named) + structured binding support

#### Memory Layout Example

```text
[name_len:2][name][type:1][len:4?][data...]
```

For `"age" = 30`:
```
03 00 61 67 65 06 1E 00 00 00
     a  g  e     (int32)
```

#### Basic Usage

```cpp
#include "gd_arguments.h"
using namespace gd::argument;

int main() {
    // 1. Stack-based (zero heap!)
    std::array<std::byte, 256> buffer;
    arguments args(buffer);  // uses external buffer

    // 2. Append values
    args.append("name", "Alice");
    args.append("age", 30);
    args.append("active", true);
    args.append("score", 95.5);

    // Also supports unnamed values (like array)
    args.append(1000);                    // unnamed int32
    args.append_binary({1,2,3,4}, 4);     // binary blob

    // 3. Retrieve values
    std::string name = args["name"].as<std::string>();
    int age = args["age"].get<int>();     // or .as<int>()
    double score = args["score"].as<double>();

    // Index access (0-based)
    int first_unnamed = args[4].as<int>(); // 1000

    // Safe with default
    std::string nickname = args.get_argument("nickname", "Anonymous"s);

    // 4. Modify existing (or add if missing)
    args["age"] = 31;                     // overwrites if exists
    args.set("level", 10);                // same as append if not found
    args["old"] = 80;                     // overwrites if exists, adds if not found

    // 5. Iteration (C++17 structured binding!)
    for (auto [key, value] : args.named()) {
        std::cout << key << ": " << value.as_string() << "\n";
    }

    // Simple forward iteration
    for (auto pos = args.next(); pos; pos = args.next(pos)) {
        auto name = arguments::get_name_s(pos);
        auto val = arguments::get_argument_s(pos).as_string();
        std::cout << name << " = " << val << "\n";
    }

    return 0;
}
```

#### Construction Options

```cpp
// Heap-allocated (default)
arguments args1;

// Stack buffer (recommended for performance)
std::array<std::byte, 512> buf;
arguments args2(buf);

// From initializer list (convenient!)
arguments args3 = {
    {"x", 10}, {"y", 20.5}, {"name", "test"}
};

// Non-owning view style (lightweight)
arguments args4 = {
    {"mode", variant_view("fast")},
    {"debug", variant_view(true)}
};
```
#### Accessing Pointer Types

```cpp
   std::string* pstring1 = new std::string( "sample on how to use pointers");
   gd::argument::arguments args;
   args["string"] = pstring1;
   std::string* pstring2 = args["string"].get_pointer<std::string>();
   assert( *pstring2 == "sample on how to use pointers" );
   delete pstring2;
```

#### When to Use `arguments`

| Scenario                                | Use `arguments`? | Why |
|-----------------------------------------|------------------|-----|
| Network packets / binary protocols      | Yes              | Compact, predictable layout |
| Embedded / limited memory               | Yes              | Stack-friendly, minimal overhead |
| High-frequency temporary data           | Yes              | Fast, no heap churn |
| Long-term storage / complex logic       | Maybe            | Consider `gd::args` for easier editing |
| Need named parameters in functions      | Yes              | Clean syntax with zero cost |

#### Comparison with Other Types

| Type                        | Ownership | Memory Use | Speed | Named Access | Stack-Friendly |
|-----------------------------|-----------|------------|-------|--------------|----------------|
| `gd::args` (vector<arg>)    | Owning    | High       | Medium| Yes          | No             |
| `args_view`                 | Non-owning| Medium     | Fast  | Yes          | Partial        |
| `arguments`                 | Optional  | **Lowest** | **Fastest** | Yes     | **Yes**        |

#### Summary

`gd::argument::arguments` is the **most memory-efficient** way to store heterogeneous key-value data in C++. It combines:

- The flexibility of `gd::variant` (any type)
- The convenience of named access
- The performance of raw byte buffers
- The safety of bounds checking and type validation

Use it whenever you care about **size**, **speed**, or **stack usage** — especially in performance-critical or resource-constrained code.

It's the "bare metal" version of argument passing in this library ecosystem: flexible, compact, and fast.
