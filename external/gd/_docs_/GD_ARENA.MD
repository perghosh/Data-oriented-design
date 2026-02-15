[← TOC](TOC.md)

### Tutorial: `gd::arena::arena` – High-Performance Stack-Based Memory Allocator

The `gd::arena::arena` class is a **fast, stack-friendly memory allocator** designed for **temporary allocations** with **bulk deallocation**. It allocates memory in blocks and frees all allocations with a single operation.

It's perfect for:
- Temporary data structures that have a known lifetime
- High-performance scenarios with many small allocations
- Reducing memory fragmentation in long-running processes
- Game development (frame-based allocations)
- Network protocol parsing (packet lifetime)
- Replacing `new/delete` in performance-critical code

#### Key Design Features

- **O(1) deallocation**: Free all allocations at once with `reset()` or when arena goes out of scope
- **Contiguous blocks**: Memory is allocated in large blocks for better cache locality
- **No per-allocation overhead**: Minimal overhead per allocation
- **Alignment support**: Can allocate with specific alignment requirements
- **Block-based growth**: Automatically creates new blocks when current is full
- **Iterator support**: Iterate over blocks and allocations for debugging
- **Custom allocators**: Template parameter allows custom allocation strategies
- **Validation**: Built-in validation for corruption detection

#### Memory Layout

```
[Block Header][Allocation Header][User Data][Allocation Header][User Data]...
    |            |
    |            +-- Each allocation has a header (size, alignment, magic)
    +-- Each block has a header (size, used, alloc count, magic)
```

Block Header:
- Magic number for validation
- Total block size
- Used bytes in block
- Allocation count

Allocation Header:
- Magic number for validation
- Allocation size
- Alignment requirements
- Padding bytes

#### Basic Usage

```cpp
#include "gd/gd_arena.h"
using namespace gd::arena;

int main() {
    // 1. Default arena (default 4KB blocks)
    arena<> arena1;

    // 2. Custom block size
    arena<> arena2(1024);  // 1KB blocks

    // 3. Allocate raw bytes
    void* buffer = arena1.allocate(256);

    // 4. Allocate arrays of objects
    int* intArray = arena1.allocate_objects<int>(10);
    for (int i = 0; i < 10; ++i) {
        intArray[i] = i * 10;
    }

    // 5. Allocate and get span (C++20)
    auto doubleSpan = arena1.allocate_span<double>(5);
    for (size_t i = 0; i < doubleSpan.size(); ++i) {
        doubleSpan[i] = 3.14 * i;
    }

    // 6. Allocated memory is automatically freed when arena goes out of scope
    return 0;
}
```

#### Using Arena with Arguments

```cpp
#include "gd/gd_arena.h"
#include "gd/gd_arguments.h"

void process_request() {
    // Create arena for request lifetime
    arena<> arena_;

    // Allocate buffer for arguments
    auto buffer = arena_.allocate_span<std::byte>(2048);

    // Create arguments using arena memory
    gd::argument::arguments args(buffer);

    // Add data - all stored in arena memory
    args.append("user", "Alice");
    args.append("age", 30);
    args.append("active", true);

    // Access data
    std::string name = args["user"].as_string();
    int age = args["age"].as_int();

    // Everything is freed automatically
}

// Multiple allocations in one arena
void batch_process() {
    arena<> arena_;

    // First allocation
    auto span1 = arena_.allocate_span<std::byte>(512);
    gd::argument::arguments args1(span1);
    args1.append("batch", 1);

    // Second allocation
    auto span2 = arena_.allocate_span<std::byte>(512);
    gd::argument::arguments args2(span2);
    args2.append("batch", 2);

    // Both are alive until arena goes out of scope
}
```

#### Using Arena with Vector

```cpp
#include "gd/gd_arena.h"
#include "gd/gd_vector.h"

void vector_operations() {
    arena<> arena_;

    // Allocate vector of custom struct
    struct UserData {
        int id;
        std::string name;
        double score;
    };

    UserData* userData = arena_.allocate_objects<UserData>(100);
    
    // Use as raw array
    for (int i = 0; i < 100; ++i) {
        userData[i].id = i;
        userData[i].name = "User" + std::to_string(i);
        userData[i].score = 50.0 + (rand() % 50);
    }

    // Or use span for safer access
    auto userSpan = arena_.allocate_span<UserData>(100);
    for (auto& user : userSpan) {
        user.id = 0;
        user.name = "";
        user.score = 0.0;
    }
}

// Borrow pattern: Vector borrows arena memory
void borrow_pattern() {
    arena<> arena_;
    
    // Allocate large buffer
    auto buffer = arena_.allocate_span<std::byte>(4096);
    
    // Create vector that borrows the buffer
    gd::borrow::vector<int> vec;
    vec.assign(buffer.data(), buffer.size() / sizeof(int));
    
    // Vector uses arena memory but doesn't own it
    vec.push_back(1);
    vec.push_back(2);
    
    // Memory is still freed when arena is destroyed
}
```

#### Advanced Patterns

##### Scoped Arena (RAII)

```cpp
class ScopedArena {
    arena<> arena_;
public:
    template<typename T>
    T* allocate(size_t count) {
        return arena_.allocate_objects<T>(count);
    }
    
    template<typename T>
    std::span<T> allocate_span(size_t count) {
        return arena_.allocate_span<T>(count);
    }
    
    void reset() { arena_.reset(); }
};

void scoped_usage() {
    ScopedArena scope;
    
    auto data = scope.allocate_span<double>(1000);
    auto names = scope.allocate_span<char>(256);
    
    // All freed when scope goes out of scope
}
```

##### Frame-Based Allocation (Game Development)

```cpp
class FrameAllocator {
    arena<> arena_;
public:
    FrameAllocator(size_t frameSize = 1024 * 1024) : arena_(frameSize) {}
    
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        T* ptr = arena_.allocate_objects<T>(1);
        new(ptr) T(std::forward<Args>(args)...);
        return ptr;
    }
    
    void newFrame() {
        arena_.reset();
    }
    
    void* allocate(size_t size) {
        return arena_.allocate(size);
    }
};

void game_loop() {
    FrameAllocator allocator(2 * 1024 * 1024);  // 2MB per frame
    
    while (running) {
        allocator.newFrame();  // Clear previous frame
        
        // Allocate frame data
        auto particles = allocator.allocate_span<Particle>(1000);
        auto UIElements = allocator.allocate_span<UIElement>(50);
        
        render(particles, UIElements);
        
        // Everything automatically freed on next newFrame()
    }
}
```

##### Temporary String Building

```cpp
std::string build_temporary(const std::vector<std::string>& parts) {
    arena<> arena_;  // Arena lifetime only for this function
    
    // Calculate total size needed
    size_t total = 0;
    for (const auto& part : parts) {
        total += part.size() + 1;  // +1 for space or null
    }
    
    // Allocate contiguous buffer
    char* buffer = arena_.allocate_objects<char>(total + 1);
    char* pos = buffer;
    
    // Build string
    for (const auto& part : parts) {
        std::memcpy(pos, part.data(), part.size());
        pos += part.size();
        *pos++ = ' ';
    }
    *pos = '\0';
    
    return std::string(buffer, total);  // Copy to std::string for return
    // Arena memory is freed here
}
```

#### Memory Management Methods

```cpp
arena<> arena_;

// Query methods
size_t blockSize = arena_.block_size();        // Size of each block
size_t blockCount = arena_.block_count();      // Number of blocks
size_t allocated = arena_.total_allocated();  // Total bytes allocated
size_t capacity = arena_.total_capacity();    // Total capacity
double frag = arena_.fragmentation();         // Fragmentation ratio

// Reset methods
arena_.clear();          // Deallocate all blocks
arena_.reset();          // Reset allocations but keep blocks
arena_.shrink_to_fit();  // Free unused blocks

// Iteration
for (auto it = arena_.begin_blocks(); it != arena_.end_blocks(); ++it) {
    // Iterate over blocks
}

for (auto it = arena_.begin_allocations(); it != arena_.end_allocations(); ++it) {
    // Iterate over allocations
    void* data = it.data();
}

// Validation
bool isValid = arena_.validate();  // Check for corruption
arena_.dump_blocks();            // Print block information
arena_.dump_allocations();        // Print allocation information
```

#### Aligned Allocation

```cpp
arena<> arena_;

// Allocate with specific alignment
void* alignedPtr = arena_.allocate_aligned(256, 64);  // 256 bytes, 64-byte aligned

// Good for SIMD data
float* simdData = static_cast<float*>(
    arena_.allocate_aligned(256 * sizeof(float), alignof(float) * 4)
);

// Allocated with 16-byte alignment for SSE
```

#### Comparison with Standard Allocators

| Approach              | Allocation Time | Deallocation Time | Memory Overhead | Fragmentation | Use Case |
|----------------------|----------------|------------------|----------------|---------------|----------|
| `new/delete`         | O(1)           | O(1)            | High per alloc | High          | Long-lived objects |
| `std::allocator`     | O(1)           | O(1)            | Low            | Medium        | Generic containers |
| `pool allocator`     | O(1)           | O(1)            | Very low       | Low           | Fixed-size objects |
| **`arena::arena`**   | **O(1)**       | **O(1) bulk**    | **Very low**   | **None**      | **Temporary data** |

#### When to Use Arena

| Scenario                                    | Use `arena`? | Why |
|---------------------------------------------|--------------|-----|
| Temporary calculations in a function           | Yes          | Auto-cleanup, fast |
| Frame-based game data                        | Yes          | Bulk deallocation per frame |
| Network packet parsing                       | Yes          | Packet lifetime is known |
| Data processing pipelines                    | Yes          | Stage-based allocation |
| Small objects with short lifetime            | Yes          | Reduces fragmentation |
| Large objects with long lifetime             | No           | Use standard allocation |
| Objects that need individual deallocation     | No           | Arena frees everything |
| Shared data across threads                   | No           | Arena is not thread-safe |

#### Best Practices

1. **Scope arenas tightly**: Create arena at the point where allocations begin and destroy when done
2. **Choose appropriate block size**: Too small = many blocks, too large = wasted memory
3. **Use `reset()` for reuse**: Instead of destroying and recreating arenas
4. **Monitor fragmentation**: Call `fragmentation()` if memory usage is critical
5. **Validate in debug mode**: Use `validate()` to catch corruption early
6. **Align when needed**: Use `allocate_aligned()` for SIMD or special requirements

#### Performance Characteristics

```
Allocation:         ~10-50 ns (amortized)
Deallocation:       ~0-5 ns (bulk via reset)
Memory overhead:    ~16 bytes per allocation + block headers
Cache locality:     Excellent (contiguous blocks)
Fragmentation:      None (linear allocation)
```

#### Common Pitfalls

❌ **Don't**: Store arena-allocated pointers beyond arena lifetime
```cpp
int* bad_pointer = nullptr;
{
    arena<> arena_;
    bad_pointer = arena_.allocate_objects<int>(1);
    *bad_pointer = 42;
}  // arena destroyed, bad_pointer is now dangling!
*bad_pointer = 100;  // UB!
```

✅ **Do**: Use arena only within its scope
```cpp
{
    arena<> arena_;
    int* data = arena_.allocate_objects<int>(10);
    // Use data here
}
// Memory is safely freed
```

❌ **Don't**: Use arena for objects that need individual deallocation
```cpp
arena<> arena_;
auto items = arena_.allocate_objects<Item>(1000);
process_items(items);  // What if we want to free item 500 early?
```

✅ **Do**: Use arena when you know all allocations end together
```cpp
arena<> arena_;
auto items = arena_.allocate_objects<Item>(1000);
process_all_items(items);  // All freed together at end
```

#### Integration Example: Complete System

```cpp
class RequestProcessor {
    arena<> arena_;  // Per-request arena
    
public:
    void handle_request(const std::string& request) {
        arena_.reset();  // Clear previous request
        
        // Allocate all request-related data
        auto buffer = arena_.allocate_span<std::byte>(4096);
        gd::argument::arguments args(buffer);
        
        // Parse request into arguments
        parse_request(request, args);
        
        // Allocate processing data
        auto workingSet = arena_.allocate_span<float>(1024);
        
        // Process
        auto result = process(args, workingSet);
        
        // Send response
        send_response(result);
        
        // Everything automatically freed on next request or destruction
    }
    
private:
    void parse_request(const std::string& req, gd::argument::arguments& args) {
        // Parse into arena-allocated arguments
        args.append("method", extract_method(req));
        args.append("path", extract_path(req));
        // ... more fields
    }
    
    std::string process(const gd::argument::arguments& args, 
                      std::span<float> workingSet) {
        // Use working set for computation
        // All intermediate data can be allocated from arena_
        return "result";
    }
};
```

#### Summary

`gd::arena::arena` is the **ultimate tool for temporary allocation** in C++. It provides:

- **Zero-overhead allocation**: Faster than `new/delete` for many small objects
- **Bulk deallocation**: Free everything in O(1) time
- **No fragmentation**: Linear allocation eliminates fragmentation
- **Excellent cache locality**: Contiguous blocks improve performance
- **Simple API**: Easy to use correctly, hard to misuse
- **Debug-friendly**: Built-in validation and diagnostics

Use it whenever you have a **clear allocation lifetime** and want **maximum performance** with **minimal complexity**.

It's the foundation of high-performance C++ code: allocate fast, free fast, move on to the next task.