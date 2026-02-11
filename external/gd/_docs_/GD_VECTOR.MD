
[← TOC](TOC.md)

### Short Tutorial: `gd::stack::vector` – Small Vector Optimization with Zero-Heap Performance

The `gd::stack::vector` class is a **hybrid container** that provides the familiar interface of `std::vector` with **small vector optimization (SVO)**. It stores elements inline (on the stack) up to a specified capacity, then seamlessly transitions to heap allocation when needed.

It's perfect for:
- Small containers that typically fit in stack memory (avoiding heap churn)
- Embedded systems and performance-critical code
- Temporaries in hot code paths where allocation overhead matters
- When typical size is known at compile time but growth is still needed
- Replacing `std::vector<T>` when most instances have ≤N elements

#### Key Design Features

- **Zero heap allocation for small containers**: Inline storage with `uCapacityStack` elements
- **Seamless growth**: Automatically switches to heap when exceeding inline capacity
- **Standard vector API**: Drop-in replacement for `std::vector` in most cases
- **Move-optimized**: Efficient move semantics with `noexcept` where possible
- **1.5x growth strategy**: Smart capacity expansion when heap allocation is needed
- **C++20 support**: Three-way comparison, concepts, and modern C++ features
- **Full iterator support**: Forward, const, and reverse iterators

#### Memory Layout

```
┌────────────────────────────────────────────────────────────┐
│  gd::stack::vector<int, 4>  (capacity_inline = 4)          │
├────────────────────────────────────────────────────────────┤
│  Inline Buffer (stack): [int][int][int][int]               │
│                          └───────────────┘                 │
│                          Used when size ≤ 4                │
│                                                            │
│  Heap Buffer (optional): [int][int][int][int]...           │
│                          └───────────────┘                 │
│                          Used when size > 4                │
└────────────────────────────────────────────────────────────┘
```

When `size ≤ 4`: All elements stored in inline stack buffer (zero heap allocation)
When `size > 4`: Heap buffer allocated, elements moved/copied to heap

#### Basic Usage

```cpp
#include "gd_vector.h"
using namespace gd::stack;

int main() {
    // 1. Default constructor - zero elements, inline capacity of 4
    vector<int, 4> v1;
    
    // 2. Add elements (inline storage, no heap)
    v1.push_back(10);
    v1.push_back(20);
    v1.push_back(30);
    v1.push_back(40);  // Still using inline storage
    std::cout << "Capacity: " << v1.capacity() << "\n";  // 4
    std::cout << "Size: " << v1.size() << "\n";          // 4
    
    // 3. Exceed inline capacity - heap allocated automatically
    v1.push_back(50);  // Triggers heap allocation
    std::cout << "Capacity: " << v1.capacity() << "\n";  // 6 (1.5x growth)
    
    // 4. Element access
    std::cout << "First: " << v1.front() << "\n";       // 10
    std::cout << "Last: " << v1.back() << "\n";         // 50
    std::cout << "v1[2]: " << v1[2] << "\n";            // 30
    std::cout << "v1.at(1): " << v1.at(1) << "\n";      // 20
    
    // 5. Iteration
    for (auto& val : v1) {
        val *= 2;  // Double each element
    }
    
    // Const iteration
    for (const auto& val : v1) {
        std::cout << val << " ";
    }
    // Output: 20 40 60 80 100
    
    // 6. Reverse iteration
    for (auto it = v1.rbegin(); it != v1.rend(); ++it) {
        std::cout << *it << " ";
    }
    // Output: 100 80 60 40 20
    
    return 0;
}
```

#### Construction Options

```cpp
// Default constructor
vector<int, 4> v1;

// Copy constructor
vector<int, 4> v2(v1);

// Move constructor (efficient!)
vector<int, 4> v3(std::move(v1));

// Initializer list
vector<std::string, 3> v4 = {"hello", "world", "test"};
vector<std::string, 2> v5 = {"a", "b", "c"};  // Heap allocated (3 > 2)

// Fill constructor
vector<int, 10> v6(5, 100);  // 5 elements, all initialized to 100

// Range constructor
std::array<double, 4> arr = {1.1, 2.2, 3.3, 4.4};
vector<double, 3> v7(arr.begin(), arr.end());  // Heap allocated (4 > 3)
```

#### Modifying Elements

```cpp
vector<int, 4> v;

// push_back (lvalue and rvalue)
int x = 10;
v.push_back(x);       // Copy
v.push_back(20);      // Move for int (same as copy)
v.push_back(std::move(x));  // Move

// emplace_back - construct in place
v.emplace_back(30);   // Construct int directly

// pop_back
if (!v.empty()) {
    v.pop_back();
}

// resize
v.resize(10, 0);      // Grow to 10 elements, fill new ones with 0
v.resize(5);          // Shrink to 5 elements

// clear
v.clear();            // Remove all elements, capacity preserved

// reserve
v.reserve(20);        // Ensure capacity ≥ 20 (may allocate heap)
```

#### Assignment and Swap

```cpp
vector<int, 4> v1 = {1, 2, 3};
vector<int, 4> v2;

// Copy assignment
v2 = v1;  // v2 now has {1, 2, 3}

// Move assignment
vector<int, 4> v3;
v3 = std::move(v1);  // v3 has {1, 2, 3}, v1 is empty

// Swap (efficient even with mixed inline/heap)
vector<int, 4> small = {1, 2, 3};
vector<int, 2> large(10, 100);  // Heap allocated (10 > 2)
small.swap(large);  // Efficiently swap contents
```

#### std::vector vs gd::stack::vector

```cpp
// ## std::vector APPROACH - Always allocates on heap
std::vector<int> stdVec;
stdVec.push_back(1);
stdVec.push_back(2);
stdVec.push_back(3);
// Problem: Even with just 3 elements, std::vector typically allocates on heap
// Memory: heap allocation overhead + element storage

std::cout << "## std::vector Approach:\n";
std::cout << "   Size: " << stdVec.size() << "\n";
std::cout << "   Capacity: " << stdVec.capacity() << "\n";
for (auto val : stdVec) {
    std::cout << "   " << val << "\n";
}
// std::vector limitations:
// - Always allocates on heap (even for small containers)
// - Heap allocation/deallocation overhead
// - Poor cache locality for small containers
// - Memory fragmentation for many small vectors

// ## gd::stack::vector APPROACH - Zero heap for small containers
gd::stack::vector<int, 4> stackVec;
stackVec.push_back(1);
stackVec.push_back(2);
stackVec.push_back(3);
// Benefit: Elements stored inline on stack (zero heap allocation!)
// Memory: Only stack memory for inline buffer

std::cout << "\n## gd::stack::vector Approach:\n";
std::cout << "   Size: " << stackVec.size() << "\n";
std::cout << "   Capacity: " << stackVec.capacity() << "\n";
std::cout << "   Inline Capacity: " << stackVec.capacity_inline() << "\n";
for (auto val : stackVec) {
    std::cout << "   " << val << "\n";
}

// Add more elements - automatically switches to heap
stackVec.push_back(4);  // Still inline
stackVec.push_back(5);  // Triggers heap allocation!
std::cout << "\nAfter adding 5 elements:\n";
std::cout << "   Size: " << stackVec.size() << "\n";
std::cout << "   Capacity: " << stackVec.capacity() << "\n";
std::cout << "   Is external (heap): " << (stackVec.is_external() ? "Yes" : "No") << "\n";

// gd::stack::vector advantages:
// - Zero heap allocation for small containers
// - Better cache locality (inline storage)
// - No heap fragmentation for small vectors
// - Seamless transition to heap when needed
// - Same API as std::vector (drop-in compatible)

// Performance comparison (conceptual):
std::cout << "\n## Performance Characteristics:\n";
std::cout << "   std::vector (small):  Heap alloc + copy overhead\n";
std::cout << "   gd::stack::vector (≤4): Zero heap, pure stack operations\n";
std::cout << "   gd::stack::vector (>4): Same as std::vector (heap allocated)\n";
```

#### Practical Examples

```cpp
// Example 1: Temporary buffer in function
void process_data() {
    // Typical size: 0-10 elements, max rarely exceeded
    gd::stack::vector<int, 16> temp_buffer;
    
    // Add some values
    temp_buffer.push_back(1);
    temp_buffer.push_back(2);
    temp_buffer.push_back(3);
    
    // Process without heap allocation overhead
    for (auto& val : temp_buffer) {
        val *= 10;
    }
    
}  // Automatic cleanup, no heap freed if capacity not exceeded

// Example 2: Path components (strings)
gd::stack::vector<std::string, 8> path_components;
path_components.push_back("home");
path_components.push_back("user");
path_components.push_back("documents");
// Zero heap allocation (3 ≤ 8)

// Example 3: Small cache in class
class AnimationSystem {
public:
    void update(float dt) {
        // Add recent frames for smoothing
        recent_frames_.push_back(dt);
        if (recent_frames_.size() > recent_frames_.capacity_inline()) {
            recent_frames_.pop_back();  // Keep within inline capacity
        }
    }
    
private:
    // Store last 5 frame times inline
    gd::stack::vector<float, 5> recent_frames_;
};

// Example 4: Key-value pairs for configuration
struct ConfigEntry {
    std::string key;
    int value;
};

gd::stack::vector<ConfigEntry, 10> config;
config.push_back({"max_connections", 100});
config.push_back({"timeout", 30});
config.push_back({"retry_count", 3});
// Zero heap allocation, perfect for small config sets

// Example 5: JSON conversion with owning variants (AsVariant)
// Demonstrates how gd::variant_array (using stack::vector) efficiently handles JSON parsing
gd::variant AsVariant( const jsonpath::json& json )
{
    if( json.is_null() )            return gd::variant();
    else if( json.is_boolean() )    return gd::variant( json.as_bool() );
    else if( json.is_number() )     return gd::variant( json.as_double() );
    else if( json.is_string() )
        return gd::variant( json.as_string() );
    else if( json.is_array() )
    {
        gd::variant_array array;
        for ( const auto& item : json )
            array.push_back( AsVariant( item ) );
        return gd::variant( std::move( array ) );
    }
    else if( json.is_object() )
    {
        gd::variant_object object;
        for ( const auto& [key, value] : json )
            object[key] = AsVariant( value );
        return gd::variant( std::move( object ) );
    }
    else
        return gd::variant();
}

// Example 6: JSON conversion with view variants (AsVariantView)
// Similar to AsVariant but optimized for view-based variant semantics
// Use when you need non-owning references to the underlying JSON data
gd::variant AsVariantView( const jsonpath::json& json )
{
    if( json.is_null() )            return gd::variant();
    else if( json.is_boolean() )    return gd::variant( json.as_bool() );
    else if( json.is_number() )     return gd::variant( json.as_double() );
    else if( json.is_string() )
        return gd::variant( json.as_string() );
    else if( json.is_array() )
    {
        gd::variant_array array;
        for ( const auto& item : json )
            array.push_back( AsVariantView( item ) );
        return gd::variant( std::move( array ) );
    }
    else if( json.is_object() )
    {
        gd::variant_object object;
        for ( const auto& [key, value] : json )
            object[key] = AsVariantView( value );
        return gd::variant( std::move( object ) );
    }
    else
        return gd::variant();
}

// When parsing typical JSON with small arrays:
// - { "name": "test", "values": [1, 2, 3] }
// The array [1, 2, 3] uses inline storage (3 ≤ typical capacity)
// - No heap allocation for small nested arrays
// - Efficient JSON-to-variant conversion in hot paths
// - Perfect for API responses, configuration files, and message parsing
// - AsVariant: Creates owning copies (safe for temporary JSON objects)
// - AsVariantView: Creates lightweight views (use with persistent JSON objects)
```

#### Comparison with Other Containers

| Type                          | Heap for Small | Inline Storage | Growth Strategy | Move Efficient | Cache Friendly |
|-------------------------------|----------------|----------------|-----------------|----------------|----------------|
| `std::vector<T>`              | Yes            | No             | 2x              | Yes            | Medium         |
| `std::array<T, N>`            | No             | Yes (fixed)    | None            | Yes            | High           |
| `gd::stack::vector<T, N>`     | **No**         | **Yes**        | **1.5x**        | **Yes**        | **High**       |
| `std::deque<T>`               | Yes            | Partial        | Variable        | Medium         | Low            |

#### When to Use `gd::stack::vector`

| Scenario                                | Use `gd::stack::vector`? | Why |
|-----------------------------------------|--------------------------|-----|
| Temporary small buffers in functions    | **Yes**                  | Zero heap, automatic cleanup |
| Hot code paths with known small size    | **Yes**                  | Cache-friendly, no allocation |
| Embedded systems / limited heap         | **Yes**                  | Reduces heap pressure |
| Many instances with ≤N elements         | **Yes**                  | Dramatically reduces heap usage |
| Large data structures (>1000 elements)  | Maybe                    | Use std::vector if always large |
| Need fixed-size array                   | No                       | Use std::array instead |
| Need specialized data structures        | No                       | Use appropriate container |

#### Capacity and Storage Mode

```cpp
gd::stack::vector<int, 4> v = {1, 2, 3};

// Query capacity
std::cout << "Inline capacity: " << v.capacity_inline() << "\n";  // 4
std::cout << "Current capacity: " << v.capacity() << "\n";        // 4
std::cout << "Size: " << v.size() << "\n";                        // 3

// Check storage mode
std::cout << "Using heap: " << (v.is_external() ? "Yes" : "No") << "\n";  // No

// Force heap allocation by exceeding inline capacity
v.push_back(4);  // Still inline (4 = 4)
v.push_back(5);  // Triggers heap!

std::cout << "\nAfter adding 2 more:\n";
std::cout << "Current capacity: " << v.capacity() << "\n";        // 6 (1.5x growth)
std::cout << "Size: " << v.size() << "\n";                        // 5
std::cout << "Using heap: " << (v.is_external() ? "Yes" : "No") << "\n";  // Yes

// Reserve can force heap allocation
v.reserve(20);
std::cout << "\nAfter reserve(20):\n";
std::cout << "Current capacity: " << v.capacity() << "\n";        // 20
std::cout << "Using heap: " << (v.is_external() ? "Yes" : "No") << "\n";  // Yes
```

#### Best Practices

```cpp
// ✅ GOOD: Choose inline capacity based on typical usage
// If 95% of cases have ≤4 elements, use capacity 4
gd::stack::vector<int, 4> typical_vector;

// ✅ GOOD: Use emplace_back for complex types
struct Point { float x, y, z; };
gd::stack::vector<Point, 10> points;
points.emplace_back(1.0f, 2.0f, 3.0f);  // Construct in place

// ✅ GOOD: Move vectors when passing to functions
void process(gd::stack::vector<int, 8> data);
auto v = gd::stack::vector<int, 8>{1, 2, 3};
process(std::move(v));  // Efficient move

// ⚠️ AVOID: Too large inline capacity wastes stack memory
// gd::stack::vector<int, 10000> huge_stack;  // 40KB on stack!
// Better: Use smaller capacity, let it grow to heap
gd::stack::vector<int, 100> reasonable_stack;

// ✅ GOOD: Use for return values from functions
auto get_ids() -> gd::stack::vector<int, 16> {
    gd::stack::vector<int, 16> result;
    result.push_back(1);
    result.push_back(2);
    return result;  // NRVO or move optimization
}

// ✅ GOOD: Store in classes for member variables
class GameObject {
public:
    void add_child(GameObject* child) {
        children_.push_back(child);
    }
private:
    // Typical object has 0-5 children
    gd::stack::vector<GameObject*, 8> children_;
};
```

#### Summary

`gd::stack::vector` is the **high-performance alternative to `std::vector`** when you care about:
- **Eliminating heap allocations** for small containers
- **Improving cache locality** with inline storage
- **Reducing memory fragmentation** in performance-critical code
- **Maintaining std::vector compatibility** with familiar API

It combines:
- The **zero-allocation efficiency** of fixed-size arrays (`std::array`)
- The **flexibility and growth** of dynamic arrays (`std::vector`)
- The **automatic optimization** of small vector optimization (SVO)

Use it whenever you have **many small containers** or **hot code paths** where allocation overhead matters. It's especially valuable in:
- Game engines and real-time systems
- Embedded and resource-constrained environments
- High-frequency trading or networking code
- Any scenario where avoiding heap churn improves performance

It's the "smart vector" that knows how to stay fast when small and stay flexible when large.

---

### Short Tutorial: `gd::borrow::vector` – Zero-Heap Performance with External Storage

The `gd::borrow::vector` class is a **flexible container** that provides the familiar interface of `std::vector` with the ability to **borrow external storage** initially, then seamlessly transition to heap ownership when needed.

It's perfect for:
- Using stack-allocated or existing buffers as temporary storage
- APIs that need optional buffer ownership
- Performance-critical code where heap allocation can be avoided
- Scenarios where storage might be pre-allocated elsewhere
- Temporary processing with known buffer sizes

#### Key Design Features

- **External storage support**: Can borrow from `std::array`, C arrays, or other buffers
- **Automatic ownership transition**: Switches from borrowed to owned when capacity is exceeded
- **Standard vector API**: Drop-in replacement for `std::vector` in most cases
- **Move-optimized**: Efficient move semantics with `noexcept` where possible
- **Ownership tracking**: `owner()` and `is_borrowed()` methods to query storage state
- **Multi-argument emplace_back**: Add multiple values with one call (e.g., `emplace_back(1,2,3,4,5)`)
- **Full iterator support**: Forward, const, and reverse iterators
- **C++20 support**: Three-way comparison, concepts, and modern C++ features

#### Memory Layout

```
┌──────────────────────────────────────────────────────────────┐
│  gd::borrow::vector<int> with borrowed std::array            │
├──────────────────────────────────────────────────────────────┤
│  External Buffer: [int][int][int][int]... (borrowed)         │
│                     └───────────────┘                        │
│                     Used when borrowed and size ≤ capacity   │
│                                                              │
│  Heap Buffer (optional): [int][int][int][int]...             │
│                     └───────────────┘                        │
│                     Used when exceeded or default-constructed│
└──────────────────────────────────────────────────────────────┘
```

**Borrowed Mode**: Uses external buffer provided at construction (zero heap allocation)
**Owned Mode**: Uses heap-allocated buffer (when exceeded or default-constructed)

#### Basic Usage

```cpp
#include "gd_vector.h"
#include <array>
using namespace gd::borrow;

int main() {
    // 1. Borrow from std::array (zero heap allocation)
    std::array<int, 10> buffer;
    vector<int> v1(buffer);
    
    // 2. Add elements (using borrowed storage)
    v1.push_back(10);
    v1.push_back(20);
    v1.push_back(30);
    std::cout << "Size: " << v1.size() << "\n";          // 3
    std::cout << "Is borrowed: " << v1.is_borrowed() << "\n";  // true
    std::cout << "Is owner: " << v1.owner() << "\n";     // false
    
    // 3. Element access
    std::cout << "First: " << v1.front() << "\n";       // 10
    std::cout << "Last: " << v1.back() << "\n";         // 30
    std::cout << "v1[1]: " << v1[1] << "\n";            // 20
    std::cout << "v1.at(2): " << v1.at(2) << "\n";      // 30
    
    // 4. Exceed borrowed capacity - automatically switches to owned
    for (int i = 0; i < 10; ++i) {
        v1.push_back(i);
    }
    std::cout << "\nAfter exceeding capacity:\n";
    std::cout << "Is borrowed: " << v1.is_borrowed() << "\n";  // false
    std::cout << "Is owner: " << v1.owner() << "\n";     // true
    
    // 5. Iteration
    for (auto& val : v1) {
        std::cout << val << " ";
    }
    
    // 6. Multi-argument emplace_back (unique feature!)
    vector<int> v2;
    v2.emplace_back(1, 2, 3, 4, 5);  // Adds 5 separate elements!
    std::cout << "\n\nMulti-emplace: " << v2.size() << "\n";  // 5
    std::cout << "Elements: " << v2[0] << " " << v2[4] << "\n";  // 1 5
    
    return 0;
}
```

#### Construction Options

```cpp
// Default constructor - starts empty, will allocate on heap
vector<int> v1;
v1.push_back(1);  // Heap allocated

// Borrow from std::array
std::array<int, 10> buffer;
vector<int> v2(buffer);  // Borrows buffer

// Borrow from C array
int c_array[10];
vector<int> v3(c_array, 10);  // Borrows C array

// Initializer list (always owns)
vector<int> v4 = {1, 2, 3, 4, 5};
std::cout << "Is owner: " << v4.owner() << "\n";  // true

// Copy constructor
vector<int> v5(v4);  // v5 owns its own storage

// Move constructor (efficient!)
vector<int> v6(std::move(v4));
// v4 is now empty, v6 owns the storage
```

#### Ownership and Borrowing

```cpp
std::array<int, 5> buffer;
vector<int> v(buffer);

std::cout << "Initial state:\n";
std::cout << "  Is borrowed: " << v.is_borrowed() << "\n";  // true
std::cout << "  Is owner: " << v.owner() << "\n";            // false

// Use borrowed storage
for (int i = 0; i < 5; ++i) {
    v.push_back(i * 10);
}
std::cout << "  Size: " << v.size() << "\n";  // 5
std::cout << "  Still borrowed: " << v.is_borrowed() << "\n";  // true

// Trigger ownership transition
v.push_back(50);  // Exceeds borrowed capacity!
std::cout << "\nAfter exceeding capacity:\n";
std::cout << "  Is borrowed: " << v.is_borrowed() << "\n";  // false
std::cout << "  Is owner: " << v.owner() << "\n";            // true
std::cout << "  Elements moved to heap\n";

// Force ownership with reserve
vector<int> v2(buffer);
v2.reserve(20);  // Forces allocation, now owned
std::cout << "\nAfter reserve(20):\n";
std::cout << "  Is owner: " << v2.owner() << "\n";  // true
```

#### Modifying Elements

```cpp
vector<int> v;

// push_back (lvalue and rvalue)
int x = 10;
v.push_back(x);       // Copy
v.push_back(20);      // Move for int
v.push_back(std::move(x));  // Move

// emplace_back - construct in place
v.emplace_back(30);   // Construct int directly

// Multi-argument emplace_back - add multiple values!
v.emplace_back(40, 50, 60);  // Adds 3 separate elements
std::cout << "Size after multi-emplace: " << v.size() << "\n";  // 6

// pop_back
if (!v.empty()) {
    v.pop_back();
}

// resize
v.resize(10, 0);      // Grow to 10 elements
v.resize(5);          // Shrink to 5 elements

// clear
v.clear();            // Remove all elements, ownership preserved

// reserve (may force ownership if borrowed)
v.reserve(20);
```

#### Multi-Argument emplace_back (Unique Feature)

```cpp
// Standard emplace_back - adds ONE element
vector<int> v1;
v1.emplace_back(10);  // Add single element: [10]

// Multi-argument emplace_back - adds MULTIPLE elements
vector<int> v2;
v2.emplace_back(10, 20, 30, 40, 50);  // Add 5 elements: [10, 20, 30, 40, 50]
std::cout << "Size: " << v2.size() << "\n";  // 5
std::cout << "First: " << v2.front() << "\n";  // 10
std::cout << "Last: " << v2.back() << "\n";  // 50

// Works with any type
vector<double> v3;
v3.emplace_back(1.1, 2.2, 3.3, 4.4);
// [1.1, 2.2, 3.3, 4.4]

// Useful for initialization
vector<long long> v4;
v4.emplace_back(1LL, 2LL, 3LL, 4LL, 5LL, 6LL);
// [1, 2, 3, 4, 5, 6]

// Combines borrowed storage with multi-emplace
std::array<int, 20> buffer;
vector<int> v5(buffer);
v5.emplace_back(100, 200, 300, 400, 500);  // Adds 5 elements to borrowed storage
std::cout << "Still borrowed: " << v5.is_borrowed() << "\n";  // true
```

#### Assignment and Swap

```cpp
// Copy assignment
vector<int> v1 = {1, 2, 3};
vector<int> v2;
v2 = v1;  // v2 now has its own copy

// Move assignment
vector<int> v3;
v3 = std::move(v1);  // v3 has the data, v1 is empty

// Swap (efficient even with mixed borrowed/owned)
std::array<int, 5> buffer;
vector<int> borrowed(buffer);
vector<int> owned = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

std::cout << "Before swap:\n";
std::cout << "  Borrowed is borrowed: " << borrowed.is_borrowed() << "\n";
std::cout << "  Owned is owner: " << owned.owner() << "\n";

borrowed.swap(owned);

std::cout << "After swap:\n";
std::cout << "  Borrowed is now owner: " << borrowed.owner() << "\n";
std::cout << "  Owned is now borrowed: " << owned.is_borrowed() << "\n";
```

#### Comparison with Other Containers

| Type                          | Can Borrow | Auto Switch to Heap | Zero-Heap Mode | Multi-Emplace |
|-------------------------------|------------|---------------------|----------------|---------------|
| `std::vector<T>`              | No         | N/A                 | No             | No            |
| `std::array<T, N>`            | No         | N/A                 | Yes (fixed)    | No            |
| `gd::stack::vector<T, N>`     | No         | Yes                 | Yes            | No            |
| `gd::borrow::vector<T>`       | **Yes**    | **Yes**             | **Yes**        | **Yes**       |
| `std::span<T>`                | Yes        | No                  | Yes            | No            |

#### When to Use `gd::borrow::vector`

| Scenario                                | Use `gd::borrow::vector`? | Why |
|-----------------------------------------|---------------------------|-----|
| API accepts optional buffer             | **Yes**                   | Can borrow or own as needed |
| Temporary processing with known buffer   | **Yes**                   | Zero heap if size fits |
| Hot code with pre-allocated storage      | **Yes**                   | Avoid allocation overhead |
| Need multi-value initialization         | **Yes**                   | `emplace_back(1,2,3)` is convenient |
| Fixed-size array only                   | No                        | Use std::array |
| Always heap-allocated                   | Maybe                     | Use std::vector |
| Need only view semantics                | No                        | Use std::span |

#### Best Practices

```cpp
// ✅ GOOD: Borrow when external buffer is available
std::array<int, 16> buffer;
void process(gd::borrow::vector<int>& data);
{
    gd::borrow::vector<int> temp(buffer);
    // Use temp, zero heap if size ≤ 16
    process(temp);
}

// ✅ GOOD: Use multi-argument emplace_back for bulk initialization
gd::borrow::vector<int> ids;
ids.emplace_back(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
// Clear and efficient

// ✅ GOOD: Check ownership before assuming buffer lifetime
void save_data(const gd::borrow::vector<int>& data) {
    if (data.is_borrowed()) {
        // Buffer lifetime managed externally
        std::cout << "Using borrowed storage\n";
    } else {
        // Vector owns the data
        std::cout << "Using owned storage\n";
    }
}

// ⚠️ AVOID: Keep reference to borrowed buffer after ownership change
std::array<int, 5> buffer;
gd::borrow::vector<int> v(buffer);
int* p = v.data();  // OK for now
v.push_back(1, 2, 3, 4, 5, 6);  // Exceeds capacity!
// p is now invalid - ownership changed!

// ✅ GOOD: Reserve early if you know final size
std::array<int, 10> buffer;
gd::borrow::vector<int> v(buffer);
v.reserve(20);  // Force ownership early
// Now we can grow without worrying about invalidating references

// ✅ GOOD: Use in classes for flexible storage
class NetworkBuffer {
public:
    NetworkBuffer() : vector_(stack_buffer_) {}
    
    void add_data(int data) {
        vector_.push_back(data);
        // Uses stack buffer first, heap if needed
    }
    
private:
    std::array<int, 32> stack_buffer_;
    gd::borrow::vector<int> vector_;
};

// ✅ GOOD: Use for return values with flexible ownership
auto get_ids() -> gd::borrow::vector<int> {
    std::array<int, 16> static_buffer;
    static int counter = 0;
    
    if (counter++ % 2 == 0) {
        // Return borrowed (fast path)
        return gd::borrow::vector<int>(static_buffer);
    } else {
        // Return owned (fallback)
        return gd::borrow::vector<int>({1, 2, 3, 4, 5});
    }
}
```

#### Summary

`gd::borrow::vector` is the **flexible alternative to `std::vector`** when you need:
- **External buffer support** for zero-allocation performance
- **Automatic ownership transition** when exceeding borrowed capacity
- **Multi-value initialization** with `emplace_back(1,2,3,...)`
- **API flexibility** to accept optional buffers
- **Standard vector compatibility** with familiar API

It combines:
- The **zero-allocation efficiency** of using pre-allocated buffers
- The **flexibility** of automatic heap allocation when needed
- The **convenience** of multi-argument initialization
- The **safety** of ownership tracking

Use it whenever you have:
- **Pre-allocated buffers** that could be borrowed
- **APIs** that need optional storage ownership
- **Performance-critical code** where allocation overhead matters
- **Bulk initialization** needs with multiple values

It's the "flexible vector" that can borrow, own, or adapt based on your needs.
