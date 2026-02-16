[← TOC](TOC.md)

### Tutorial: `gd::arena::borrow::arena` – Zero-Allocation Stack-Based Memory Allocator

The `gd::arena::borrow::arena` class is a **lightweight, zero-allocation memory allocator** designed for **high-performance scenarios** where avoiding heap allocations is critical. It can "borrow" stack memory or own heap memory, providing a bump-allocator interface that eliminates allocation overhead for STL containers.

It's perfect for:
- Accelerating `std::vector`, `std::string`, and other STL containers
- Performance-critical code paths where allocation overhead matters
- Temporary data structures with predictable size
- Stack-based allocation to avoid heap fragmentation
- Small, short-lived allocations in tight loops
- Real-time systems with deterministic memory behavior

#### Key Design Features

- **Zero heap allocation**: Uses stack memory or pre-allocated buffers
- **Bump allocator allocation**: O(1) allocation with just pointer increment
- **Flexible ownership**: Can borrow external storage OR own heap storage
- **STL allocator adapter**: `arena_allocator` integrates with any STL container
- **Automatic fallback**: Falls back to heap allocation when arena is exhausted
- **Alignment support**: Respects alignment requirements for SIMD and complex types
- **Validation**: Built-in corruption detection with magic numbers
- **No fragmentation**: Linear allocation eliminates fragmentation completely

#### Memory Layout

```
[Borrowed/Owned Buffer]
    |
    +-- bump pointer (current allocation position)
    |
    [Allocation 1][Padding][Allocation 2][Padding]...
          |                                |
          +-- User data                    +-- User data
```

When borrowing external storage:
```
Stack/External Buffer:
    +-------+---------------+---------------+-------+
    | Header| Alloc 1       | Alloc 2       | Free  |
    +-------+---------------+---------------+-------+
    0       ^               ^               capacity
            |               |
            bump--->       bump--->
```

When owning storage:
```
Heap Buffer (owned by arena):
    +-------+---------------+---------------+-------+
    | Header| Alloc 1       | Alloc 2       | Free  |
    +-------+---------------+---------------+-------+
    0       ^               ^               capacity
            |               |
            bump--->       bump--->
```

#### Basic Usage

##### Borrowing Stack Storage (Primary Use Case)

```cpp
#include "gd/gd_arena_borrow.h"
using namespace gd::arena::borrow;

void fast_function() {
    // Allocate buffer on stack (no heap allocation!)
    std::array<std::byte, 1024> buffer;
    
    // Arena borrows the buffer - zero heap allocation
    arena arena(buffer);
    
    // Fast allocations (just pointer increment)
    void* data1 = arena.allocate(256);
    void* data2 = arena.allocate(512);
    
    // All memory is freed when function returns (stack unwinds)
    // No deallocation overhead - buffer is on stack!
}

// Use with large buffers (careful of stack size)
void large_temporary() {
    // Be careful: this uses 256KB of stack space!
    std::array<std::byte, 256 * 1024> buffer;
    arena arena(buffer);
    
    // Process data...
    // No heap allocation, very fast
}
```

##### Owning Heap Storage

```cpp
void own_storage() {
    // Arena allocates its own buffer on heap (one allocation)
    arena arena(nullptr, 2048);  // nullptr = allocate 2048 bytes
    
    REQUIRE( arena.capacity() == 2048 );
    REQUIRE( arena.is_borrowed() == false );
    REQUIRE( arena.owner() == true );
    
    // Use arena...
    void* data = arena.allocate(1000);
    
    // Arena automatically frees its owned buffer on destruction
}
```

#### Integration with STL Containers

This is the **primary use case** for `borrow::arena` - accelerate STL containers by avoiding their heap allocations.

##### std::vector with Arena Allocator

```cpp
#include "gd/gd_arena_borrow.h"
#include <vector>

void vector_with_arena() {
    // Stack-allocated buffer (no heap!)
    std::array<std::byte, 4096> buffer;
    arena arena(buffer);
    
    // Create vector using arena allocator
    using Allocator = arena_allocator<int>;
    std::vector<int, Allocator> vec(Allocator(&arena));
    
    // Push back - NO heap allocation, just pointer increment!
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);  // Extremely fast
    }
    
    // Verify data is in arena
    REQUIRE( arena.contains(vec.data()) );
    REQUIRE( arena.used() > 0 );
}

// Compare: standard vector vs arena-accelerated vector
void benchmark_comparison() {
    // Standard vector - each push_back may allocate
    std::vector<int> stdVec;
    for (int i = 0; i < 100; ++i) {
        stdVec.push_back(i);  // May trigger heap allocation
    }
    
    // Arena vector - zero allocation during push_back
    std::array<std::byte, 4096> buffer;
    arena arena(buffer);
    std::vector<int, arena_allocator<int>> fastVec(arena_allocator<int>(&arena));
    for (int i = 0; i < 100; ++i) {
        fastVec.push_back(i);  // Never allocates
    }
}
```

##### std::string with Arena Allocator

```cpp
#include "gd/gd_arena_borrow.h"
#include <string>

void string_with_arena() {
    std::array<std::byte, 2048> buffer;
    arena arena(buffer);
    
    // Arena-allocated string
    using Allocator = arena_allocator<char>;
    using ArenaString = std::basic_string<char, std::char_traits<char>, Allocator>;
    
    ArenaString str(Allocator(&arena));
    
    // Concatenation - NO heap allocation!
    str = "Hello";
    str += ", ";
    str += "World!";
    str += " This is a long string that would normally cause allocation";
    
    // Verify string data is in arena
    REQUIRE( arena.contains(const_cast<char*>(str.data())) );
    
    // Use string normally...
    std::cout << str << std::endl;
}
```

##### std::map and Other Containers

```cpp
#include "gd/gd_arena_borrow.h"
#include <map>
#include <unordered_map>

void map_with_arena() {
    std::array<std::byte, 8192> buffer;
    arena arena(buffer);
    
    // Map with arena allocator
    using Allocator = arena_allocator<std::pair<const int, std::string>>;
    std::map<int, std::string, std::less<int>, Allocator> myMap(
        std::less<int>(), 
        Allocator(&arena)
    );
    
    // Insertions use arena memory (still some overhead for tree nodes)
    myMap[1] = "First";
    myMap[2] = "Second";
    myMap[3] = "Third";
    
    // All nodes are in arena
    REQUIRE( arena.used() > 0 );
}

void unordered_map_with_arena() {
    std::array<std::byte, 16384> buffer;
    arena arena(buffer);
    
    // Unordered_map with arena allocator
    using Allocator = arena_allocator<std::pair<const int, int>>;
    std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, Allocator> myMap(
        10,  // bucket count
        std::hash<int>(),
        std::equal_to<int>(),
        Allocator(&arena)
    );
    
    // Insertions - reduced allocation overhead
    for (int i = 0; i < 50; ++i) {
        myMap[i] = i * 2;
    }
}
```

#### Allocator Fallback Mechanism

One of the most powerful features of `arena_allocator` is automatic fallback to heap allocation when the arena is exhausted.

```cpp
void automatic_fallback() {
    // Small arena to demonstrate fallback
    std::array<std::byte, 128> smallBuffer;
    arena arena(smallBuffer);
    
    std::vector<int, arena_allocator<int>> vec(arena_allocator<int>(&arena));
    
    // This fits in arena (10 * 4 = 40 bytes)
    vec.reserve(10);
    REQUIRE( arena.contains(vec.data()) );  // Data is in arena
    std::cout << "Used: " << arena.used() << " bytes\n";
    
    // This WON'T fit (1000 * 4 = 4000 bytes, but only 128 in arena)
    vec.resize(1000);
    
    // Automatically fell back to heap!
    REQUIRE_FALSE( arena.contains(vec.data()) );  // Data is on heap
    std::cout << "Used: " << arena.used() << " bytes (unchanged)\n";
    
    // But vector still works perfectly!
    vec[500] = 42;
    REQUIRE( vec[500] == 42 );
}
```

The fallback algorithm:
1. Try to allocate from arena first
2. If arena is full or allocation too large, fall back to `::operator new`
3. Store allocation header before heap allocation
4. On deallocation, check if pointer is in arena
5. If in arena: bump pointer back (no-op for linear allocator)
6. If not in arena: call `::operator delete` with adjusted pointer

#### Advanced Patterns

##### Nested Containers

```cpp
void nested_containers() {
    std::array<std::byte, 16384> buffer;
    arena arena(buffer);
    
    using Allocator = arena_allocator<int>;
    using VecAllocator = arena_allocator<std::vector<int, Allocator>>;
    
    // Vector of vectors
    std::vector<std::vector<int, Allocator>, VecAllocator> matrix(
        VecAllocator(&arena)
    );
    
    // Each inner vector also uses arena allocator
    matrix.resize(5);
    for (auto& row : matrix) {
        row.push_back(1);
        row.push_back(2);
        row.push_back(3);
    }
    
    // All allocations are in arena
    REQUIRE( arena.used() > 0 );
}
```

##### String Building with Arena

```cpp
void fast_string_building() {
    std::array<std::byte, 4096> buffer;
    arena arena(buffer);
    
    using Allocator = arena_allocator<char>;
    using ArenaString = std::basic_string<char, std::char_traits<char>, Allocator>;
    
    ArenaString result(Allocator(&arena));
    
    // Build string efficiently
    result.reserve(100);
    
    result += "Processing ";
    result += std::to_string(42);
    result += " items...\n";
    
    result += "Status: ";
    result += "OK\n";
    
    result += "Time: ";
    result += std::to_string(123.456);
    result += "ms\n";
    
    // Zero heap allocation for all concatenations!
    std::cout << result;
}
```

##### Temporary Buffer Pattern

```cpp
void process_data_temporary() {
    // Arena lifetime = function scope
    std::array<std::byte, 8192> buffer;
    arena arena(buffer);
    
    // Use arena for all temporary allocations
    std::vector<int, arena_allocator<int>> tempData(arena_allocator<int>(&arena));
    std::string tempString(arena_allocator<char>(&arena));
    
    // Load temporary data
    tempData = {1, 2, 3, 4, 5};
    tempString = "Temporary work string";
    
    // Process data
    for (auto val : tempData) {
        // Do work...
    }
    
    // Everything automatically freed when function returns
    // No manual cleanup needed!
}
```

##### Scope-Based Arena for Performance

```cpp
class FastScope {
    std::array<std::byte, 4096> buffer_;
    arena arena_;
    
public:
    FastScope() : arena_(buffer_) {}
    
    template<typename T>
    std::vector<T, arena_allocator<T>> create_vector() {
        return std::vector<T, arena_allocator<T>>(arena_allocator<T>(&arena_));
    }
    
    template<typename T>
    std::basic_string<T, std::char_traits<T>, arena_allocator<T>> create_string() {
        return std::basic_string<T, std::char_traits<T>, arena_allocator<T>>(
            arena_allocator<T>(&arena_)
        );
    }
    
    size_t used() const { return arena_.used(); }
};

void scope_usage() {
    FastScope scope;
    
    // All containers in this scope use arena
    auto vec = scope.create_vector<int>();
    auto str = scope.create_string<char>();
    
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    str = "Fast string";
    
    std::cout << "Used: " << scope.used() << " bytes\n";
}
```

#### Memory Management Methods

```cpp
std::array<std::byte, 1024> buffer;
arena arena(buffer);

// Capacity methods
size_t capacity = arena.capacity();     // Total buffer size
size_t used = arena.used();             // Currently used bytes
size_t available = arena.available();   // Available bytes = capacity - used

// Ownership methods
bool isBorrowed = arena.is_borrowed();  // True if borrowing external buffer
bool isOwner = arena.owner();           // True if arena owns its buffer

// Allocation methods
void* p1 = arena.allocate(size);                              // Default alignment
void* p2 = arena.allocate(size, alignment);                  // Custom alignment

// Containment check
bool inArena = arena.contains(ptr);  // Check if pointer is within arena

// Reset (clears allocations, keeps buffer)
arena.reset();  // Used becomes 0, all allocations invalidated

// Destruction (if owning)
arena.destroy();  // Frees owned buffer if any
```

#### Alignment Support

```cpp
void aligned_allocations() {
    std::array<std::byte, 1024> buffer;
    arena arena(buffer);
    
    // Allocate with specific alignment
    void* alignedPtr = arena.allocate(64, 64);  // 64 bytes, 64-byte aligned
    
    // Verify alignment
    REQUIRE( (reinterpret_cast<std::uintptr_t>(alignedPtr) & 63) == 0 );
    
    // Good for SIMD data
    float* simdData = static_cast<float*>(
        arena.allocate(256 * sizeof(float), alignof(float) * 4)
    );
    
    // 16-byte aligned for SSE
    REQUIRE( (reinterpret_cast<std::uintptr_t>(simdData) & 15) == 0 );
}
```

#### Comparison with Other Allocators

| Approach                    | Allocation Time | Heap Allocs | Stack Usage | Max Size      | Use Case |
|-----------------------------|----------------|-------------|-------------|---------------|----------|
| `std::allocator`            | O(1)           | Many        | Low         | Unlimited     | General use |
| `std::pmr::memory_resource` | O(1)           | Few         | Variable    | Unlimited     | Custom memory mgmt |
| **`borrow::arena` (stack)** | **O(1)**       | **None**    | **Fixed**   | **Buffer size** | **Performance-critical** |
| **`borrow::arena` (owned)** | **O(1)**       | **One**     | **Low**     | **Buffer size** | **Predictable size** |

#### When to Use borrow::arena

| Scenario                                          | Use borrow::arena? | Why |
|---------------------------------------------------|-------------------|-----|
| `std::vector` in tight loop (predictable size)    | Yes               | Eliminates heap allocation |
| `std::string` concatenation in performance path   | Yes               | No allocation during concat |
| Temporary parsing buffers (<1MB)                  | Yes               | Stack-allocated, very fast |
| Real-time audio processing                        | Yes               | Deterministic memory behavior |
| Game frame data (predictable size)               | Yes               | Zero allocation during frame |
| Large data structures (>1MB)                     | Maybe             | Watch stack size |
| Unpredictable size allocations                    | No                | Better use heap allocator |
| Long-lived objects                                | No                | Arena typically short-lived |
| Thread-shared data                                | No                | Arena not thread-safe |

#### Best Practices

1. **Choose appropriate buffer size**:
   - Stack buffer: 1KB-256KB (watch stack limits)
   - Owned buffer: Match expected usage

2. **Prefer stack buffers for small, predictable usage**:
   ```cpp
   std::array<std::byte, 4096> buffer;  // Stack, no heap
   arena arena(buffer);
   ```

3. **Use owned buffers for larger or dynamic requirements**:
   ```cpp
   arena arena(nullptr, 65536);  // Heap, one allocation
   ```

4. **Take advantage of fallback for safety**:
   ```cpp
   // Start with small arena, fall back to heap if needed
   std::array<std::byte, 1024> buffer;
   arena arena(buffer);
   std::vector<int, arena_allocator<int>> vec(arena_allocator<int>(&arena));
   ```

5. **Monitor arena usage**:
   ```cpp
   if (arena.used() > arena.capacity() * 0.8) {
       // Consider increasing buffer size
   }
   ```

6. **Reset arenas for reuse**:
   ```cpp
   arena.reset();  // Faster than destroying and recreating
   ```

7. **Combine with RAII for scope management**:
   ```cpp
   class ArenaScope {
       std::array<std::byte, 4096> buffer_;
       arena arena_;
   public:
       ArenaScope() : arena_(buffer_) {}
       arena& get() { return arena_; }
   };
   ```

#### Performance Characteristics

```
Allocation (in arena):     ~5-20 ns (just pointer increment)
Allocation (fallback):     ~50-200 ns (heap allocation)
Deallocation:              ~0 ns (stack) or ~50-200 ns (heap fallback)
Memory overhead:           ~0 bytes (no allocation headers)
Cache locality:             Excellent (stack buffer)
Fragmentation:             None (linear allocation)
```

#### Common Pitfalls

❌ **Don't**: Use stack buffers that are too large
```cpp
std::array<std::byte, 10 * 1024 * 1024> buffer;  // 10MB on stack!
arena arena(buffer);
// Stack overflow risk!
```

✅ **Do**: Use owned storage for large buffers
```cpp
arena arena(nullptr, 10 * 1024 * 1024);  // 10MB on heap
```

❌ **Don't**: Store pointers after arena destruction
```cpp
int* ptr = nullptr;
{
    std::array<std::byte, 1024> buffer;
    arena arena(buffer);
    ptr = static_cast<int*>(arena.allocate(sizeof(int)));
    *ptr = 42;
}  // arena destroyed, ptr is dangling!
*ptr = 100;  // UB!
```

✅ **Do**: Keep pointers within arena scope
```cpp
{
    std::array<std::byte, 1024> buffer;
    arena arena(buffer);
    int* ptr = static_cast<int*>(arena.allocate(sizeof(int)));
    *ptr = 42;
    // Use ptr here
}  // Safe: arena lifetime matches pointer usage
```

❌ **Don't**: Assume arena will have enough space
```cpp
std::array<std::byte, 128> buffer;
arena arena(buffer);
std::vector<int, arena_allocator<int>> vec(arena_allocator<int>(&arena));
vec.resize(10000);  // Won't fit! Falls back to heap (which is OK)
// But if you need it in arena specifically, handle the failure
```

✅ **Do**: Check arena usage if space is critical
```cpp
if (arena.available() < required) {
    // Handle appropriately
}
```

#### Integration Example: Complete System

```cpp
class FastJsonParser {
    std::array<std::byte, 16384> buffer_;  // 16KB stack buffer
    arena arena_;
    
    using Allocator = arena_allocator<char>;
    using ArenaString = std::basic_string<char, std::char_traits<char>, Allocator>;
    
public:
    FastJsonParser() : arena_(buffer_) {}
    
    ArenaString parse(const std::string& json) {
        // Reset for new parse
        arena_.reset();
        
        // All temporary strings use arena
        ArenaString result(Allocator(&arena_));
        
        // Parse JSON (using arena for all temporary strings)
        size_t pos = 0;
        while (pos < json.size()) {
            // Extract field name
            ArenaString fieldName = extract_string(json, pos, Allocator(&arena_));
            
            // Extract value
            ArenaString value = extract_string(json, pos, Allocator(&arena_));
            
            // Build result (still using arena!)
            result += fieldName;
            result += ": ";
            result += value;
            result += "\n";
        }
        
        return result;  // May trigger heap allocation if too large
        // Arena allocations are freed here (stack unwinds)
    }
    
private:
    ArenaString extract_string(const std::string& json, size_t& pos, Allocator alloc) {
        ArenaString str(alloc);
        // ... parsing logic ...
        return str;
    }
};

void usage_example() {
    FastJsonParser parser;
    
    std::string json = R"({"name":"John","age":30,"active":true})";
    
    // Parse uses arena for all temporary strings
    // Zero heap allocation during parsing!
    std::string result = parser.parse(json);
    
    std::cout << result;
}
```

#### Summary

`gd::arena::borrow::arena` is the **ultimate tool for zero-allocation performance** in C++. It provides:

- **Zero heap allocation** (when using stack buffers)
- **Bump-allocator speed**: 5-20 ns per allocation
- **STL integration**: Works with any standard container
- **Automatic fallback**: Graceful degradation to heap
- **Predictable performance**: No allocation surprises
- **Excellent cache locality**: Stack memory is hot
- **Simple API**: Easy to use correctly
- **Memory efficiency**: Zero overhead per allocation

Use it whenever you need **maximum performance** with **predictable memory usage** and **minimal allocation overhead**.

It's the secret weapon for high-performance C++: avoid the heap, stay on the stack, move fast.