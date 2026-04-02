[← TOC](TOC.md)

# GD Expression Engine – Lightweight, High-Performance Scripting for C++

The **GD Expression Engine** is a blazing-fast, lightweight expression parser and evaluator designed to bring the power of scripting directly into C++ applications. With minimal overhead and maximum flexibility, it enables you to execute expressions read from files, databases, command-line arguments, or any string source—making your C++ code more dynamic and configurable without sacrificing performance.

## Key Features

- **Ultra-Fast Parsing**: Optimized tokenization and evaluation pipeline built for speed
- **Zero-Bloat Design**: Minimal memory footprint, perfect for embedded and performance-critical applications
- **Easy Extension**: Simple namespace-based method injection for custom logic
- **Thread-Safe**: Stateless design enables safe concurrent execution in multiple threads
- **Flexible Input**: Read expressions from files, databases, strings, or user input
- **Rich Type System**: Integrates seamlessly with `gd::variant` and `gd::argument::arguments`
- **Operator Precedence**: Full support for mathematical and logical operators with correct precedence
- **Infix to Postfix**: Automatic conversion for efficient evaluation using stack-based computation

## Architecture

The expression engine is built around three core components:

### 1. Tokenizer (`gd_expression_token`)
Converts raw expression strings into structured tokens:
- Operators: `+`, `-`, `*`, `/`, `&&`, `||`, `==`, `!=`, etc.
- Values: integers, doubles, strings, booleans
- Identifiers: variable names and function calls
- Keywords: `if`, `else`, `while`, `for`, `return`, etc.
- Separators: parentheses, commas, semicolons

### 2. Parser (`gd_expression_runtime`)
Manages the runtime environment:
- **Variables**: Named values that can be accessed and modified during evaluation
- **Methods**: Custom functions that extend the expression language
- **Globals**: Named objects available to all methods
- **Error Handling**: Comprehensive error reporting with detailed messages

### 3. Value System (`gd_expression_value`)
A lightweight variant type that holds expression values:
- `int64_t` for integers
- `double` for floating-point numbers
- `std::string` for text
- `bool` for logical values
- Pointers for custom objects
- Type-safe conversions between types

## Quick Start

### Basic Expression Evaluation

```cpp
#include "gd/expression/gd_expression_runtime.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_token.h"

using namespace gd::expression;

int main() {
    // 1. Create runtime
    runtime runtime_;

    // 2. Add standard mathematical methods (abs, max, min, sum, etc.)
    runtime_.add({ uMethodDefaultSize_g, pmethodDefault_g, "" });

    // 3. Add variables
    runtime_.add("x", 10);
    runtime_.add("y", 20);
    runtime_.add("name", "Alice");

    // 4. Parse expression to tokens
    std::vector<token> vectorToken;
    auto result = token::parse_s("x + y * 2", vectorToken);
    if (!result.first) {
        std::cerr << "Parse error: " << result.second << "\n";
        return 1;
    }

    // 5. Convert to postfix notation for efficient evaluation
    std::vector<token> vectorPostfix;
    result = token::compile_s(vectorToken, vectorPostfix);
    if (!result.first) {
        std::cerr << "Compile error: " << result.second << "\n";
        return 1;
    }

    // 6. Evaluate the expression
    value valueResult;
    result = runtime_.calculate_s(vectorPostfix, &valueResult);
    if (!result.first) {
        std::cerr << "Evaluation error: " << result.second << "\n";
        return 1;
    }

    std::cout << "Result: " << valueResult.as_integer() << "\n";  // Output: 50
    return 0;
}
```

### Using with `gd::argument::arguments`

```cpp
#include "gd/gd_arguments_shared.h"
#include "gd/expression/gd_expression_glue_to_gd.h"

// Parse arguments from command line or config
std::array<std::byte, 1024> buffer;
gd::argument::arguments args(buffer);
args["threshold"] = 100;
args["ratio"] = 0.5;
args["enabled"] = true;

// Create runtime with callback to find values
auto callback = [&args](const std::string_view& name, value::variant_t* variant) {
    if (auto it = args.find(name); it != args.end()) {
        *variant = to_value_g(it->as_variant_view());
        return true;
    }
    return false;
};

runtime runtime_(callback);

// Now expressions can access those variables!
// "threshold * ratio" → 50
```

### Custom Method Injection

```cpp
// Define a custom method
static std::pair<bool, std::string> CalculateDistance_s(
    const std::vector<value>& vectorArgument, 
    value* pvalueReturn
) {
    if (vectorArgument.size() < 4) {
        return { false, "distance requires 4 arguments: x1, y1, x2, y2" };
    }
    
    double x1 = vectorArgument[0].as_double();
    double y1 = vectorArgument[1].as_double();
    double x2 = vectorArgument[2].as_double();
    double y2 = vectorArgument[3].as_double();
    
    double distance = std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    *pvalueReturn = value(distance);
    
    return { true, "" };
}

// Add method array (must be sorted!)
const method pmethodCustom_g[] = {
    { (void*)&CalculateDistance_s, "distance", 4, 1 },
};

// Register with runtime
runtime_.add({ 1, pmethodCustom_g, "geometry" });

// Now use in expression: "distance(0, 0, 3, 4)" → 5.0
```

### Variable Arguments Method

```cpp
// Method that accepts any number of arguments
static std::pair<bool, std::string> AverageAll_s(
    const std::vector<value>& vectorArgument,
    value* pvalueReturn
) {
    if (vectorArgument.empty()) {
        return { false, "average requires at least 1 argument" };
    }
    
    double sum = 0;
    for (const auto& val : vectorArgument) {
        sum += val.as_double();
    }
    
    *pvalueReturn = value(sum / vectorArgument.size());
    return { true, "" };
}

const method pmethodCustom_g[] = {
    { (void*)&AverageAll_s, "average_all", 1, 1, method::eFlagVarArgs },
};
```

## Thread Safety

The expression engine is designed for **thread-safe concurrent execution**:

- **Stateless Evaluation**: Each runtime instance maintains its own state
- **No Global Mutable State**: Methods receive all data through arguments
- **Copyable Runtime**: Easy to create per-thread runtime instances

```cpp
// Safe parallel processing
std::vector<std::thread> threads;
std::vector<runtime> runtimes;

for (int i = 0; i < 4; ++i) {
    runtimes.emplace_back();
    auto& rt = runtimes.back();
    rt.add({ uMethodDefaultSize_g, pmethodDefault_g, "" });
    rt.add("index", i);
    
    threads.emplace_back([&rt]() {
        // Each thread has its own runtime, no race conditions!
        value result;
        auto expr = "index * 10";
        // ... parse and evaluate ...
    });
}

for (auto& t : threads) t.join();
```

## Integration with GD Ecosystem

The expression engine seamlessly integrates with other GD components:

### With `gd::variant`

```cpp
gd::variant v = 42;
value expr_value = to_value_g(v.as_variant_view());
// Now usable in expressions
```

### With `gd::argument::arguments`

```cpp
gd::argument::arguments args;
args["temperature"] = 23.5;
args["unit"] = "C";

runtime rt([&args](auto name, auto* variant) {
    auto view = args[name].as_variant_view();
    *variant = to_value_g(view);
    return true;
});
```

## Built-in Methods

The standard library (`gd_expression_method_01`) includes:

| Method | Description | Example |
|--------|-------------|---------|
| `abs(x)` | Absolute value | `abs(-5)` → `5` |
| `max(a, b, ...)` | Maximum value | `max(1, 5, 3)` → `5` |
| `min(a, b, ...)` | Minimum value | `min(1, 5, 3)` → `1` |
| `sum(a, b, ...)` | Sum of values | `sum(1, 2, 3)` → `6` |
| `average(a, b, ...)` | Average of values | `average(10, 20)` → `15` |
| `ceil(x)` | Round up | `ceil(3.2)` → `4` |
| `floor(x)` | Round down | `floor(3.8)` → `3` |
| `round(x)` | Round to nearest | `round(3.5)` → `4` |
| `if(cond, t, f)` | Conditional | `if(x > 0, "pos", "neg")` |
| `is_null(x)` | Check null | `is_null(value)` |
| `is_not_null(x)` | Check not null | `is_not_null(value)` |
| `all_true(a, b, ...)` | All true? | `all_true(1, 1, 0)` → `false` |
| `any_true(a, b, ...)` | Any true? | `any_true(0, 0, 1)` → `true` |
| `stddev(a, b, ...)` | Standard deviation | `stddev(1, 2, 3)` |
| `variance(a, b, ...)` | Variance | `variance(1, 2, 3)` |

## Error Handling

All operations return a `std::pair<bool, std::string>`:

```cpp
auto result = token::parse_s(expression, tokens);
if (!result.first) {
    std::cerr << "Error: " << result.second << "\n";
    // Handle error
}
```

## Performance Considerations

- **Parse Once, Evaluate Many**: Parse to tokens once, then reuse for multiple evaluations
- **Postfix Evaluation**: Postfix notation enables stack-based evaluation (O(n))
- **Minimal Allocations**: Value types use stack storage when possible
- **Binary Search**: Method lookup uses sorted arrays for O(log n) complexity

## When to Use

| Scenario | Use Expression Engine? | Why |
|----------|------------------------|-----|
| Configuration files | ✅ Yes | Flexible, human-readable |
| Database queries | ✅ Yes | Dynamic filtering |
| User-defined formulas | ✅ Yes | Safe sandboxed evaluation |
| Hot-reload logic | ✅ Yes | No recompilation needed |
| Simple math | ❌ Maybe | Native C++ faster |
| Complex business logic | ❌ No | Use C++ directly |

## See Also

- [`GD_VARIANT.MD`](GD_VARIANT.MD) - Type-safe value container
- [`GD_ARGUMENTS.MD`](GD_ARGUMENTS.MD) - Efficient key-value storage
- [`expression/README.MD`](../expression/README.MD) - Implementation details
