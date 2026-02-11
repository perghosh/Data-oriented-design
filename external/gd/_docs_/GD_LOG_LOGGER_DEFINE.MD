
[← TOC](TOC.md)

# Logging Macros and Definitions: `gd_log_logger_define.h`

The `gd_log_logger_define.h` header provides a comprehensive set of macros that simplify logging operations and make the logging code more readable and maintainable. These macros act as convenience wrappers around the core logging functionality.

## Conditional Compilation

All logging macros are controlled by the `GD_LOG_SIMPLE` preprocessor directive:

- When `GD_LOG_SIMPLE` is defined: All logging macros are active and output messages
- When `GD_LOG_SIMPLE` is not defined: All macros are replaced with no-op code (`((void)0)`)

This allows for complete removal of logging code from production builds by simply not defining `GD_LOG_SIMPLE`.

## Core Macros

### Basic Logging

| Macro | Purpose | Example |
|-------|---------|---------|
| `LOG_(uLogger, uSeverity, expression)` | Core macro that all others call | `LOG_(0, "ERROR", "Something failed")` |
| `LOG(uSeverity, expression)` | Log to default logger (ID=0) | `LOG("WARNING", "Low disk space")` |
| `LOG_NR(uNumber, uSeverity, expression)` | Log to specific logger | `LOG_NR(1, "DEBUG", "Network packet")` |

### Raw Logging (No Metadata)

| Macro | Purpose | Example |
|-------|---------|---------|
| `LOG_RAW_(uLogger, uSeverity, expression)` | Core raw macro without file/function | `LOG_RAW_(0, "INFO", "Simple message")` |
| `LOG_RAW(uSeverity, expression)` | Raw logging to default logger | `LOG_RAW("INFO", "Simple message")` |

### Conditional Logging

| Macro | Purpose | Example |
|-------|---------|---------|
| `LOG_IF_(uLogger, uSeverity, condition, expression)` | Core conditional macro | `LOG_IF_(0, "ERROR", ptr == nullptr, "Null pointer")` |
| `LOG_IF(uSeverity, condition, expression)` | Conditional log to default logger | `LOG_IF("WARNING", size > limit, "Buffer overflow risk")` |
| `LOG_RAW_IF(uSeverity, condition, expression)` | Conditional raw logging | `LOG_RAW_IF("DEBUG", verbose_mode, "Detail")` |

### Tagged Logging

| Macro | Purpose | Example |
|-------|---------|---------|
| `LOG2_(uLogger, uSeverity, tag, expression)` | Core tagged logging macro | `LOG2_(0, "ERROR", "network", "Connection failed")` |
| `LOG2(uSeverity, tag, expression)` | Tagged logging to default logger | `LOG2("INFO", "database", "Query executed")` |
| `LOG_RAW2(uSeverity, tag, expression)` | Raw tagged logging | `LOG_RAW2("DEBUG", "network", "Packet received")` |

## Severity-Specific Macros

For each severity level, there are specialized macros:

- `LOG_FATAL(expression)` / `LOG_FATAL2(tag, expression)` / `LOG_FATAL_RAW(expression)`
- `LOG_ERROR(expression)` / `LOG_ERROR2(tag, expression)` / `LOG_ERROR_RAW(expression)`
- `LOG_WARNING(expression)` / `LOG_WARNING2(tag, expression)` / `LOG_WARNING_RAW(expression)`
- `LOG_INFORMATION(expression)` / `LOG_INFORMATION2(tag, expression)` / `LOG_INFORMATION_RAW(expression)`
- `LOG_DEBUG(expression)` / `LOG_DEBUG2(tag, expression)` / `LOG_DEBUG_RAW(expression)`
- `LOG_VERBOSE(expression)` / `LOG_VERBOSE2(tag, expression)` / `LOG_VERBOSE_RAW(expression)`

## Conditional Severity Macros

For conditionally logging based on severity:

- `LOG_FATAL_IF(condition, expression)` / `LOG_FATAL_RAW_IF(condition, expression)`
- `LOG_ERROR_IF(condition, expression)` / `LOG_ERROR_RAW_IF(condition, expression)`
- `LOG_WARNING_IF(condition, expression)` / `LOG_WARNING_RAW_IF(condition, expression)`
- `LOG_INFORMATION_IF(condition, expression)` / `LOG_INFORMATION_RAW_IF(condition, expression)`
- `LOG_DEBUG_IF(condition, expression)` / `LOG_DEBUG_RAW_IF(condition, expression)`
- `LOG_VERBOSE_IF(condition, expression)` / `LOG_VERBOSE_RAW_IF(condition, expression)`

## Multi-Logger Support

For applications using multiple logger instances:

- `LOG_FATAL_NR(uNumber, expression)` / `LOG_ERROR_NR(uNumber, expression)` etc.
- `LOG_FATAL_IF_NR(uNumber, condition, expression)` / `LOG_ERROR_IF_NR(uNumber, condition, expression)` etc.

## Configuration Macros

| Macro | Purpose |
|-------|---------|
| `LOG_SET_SEVERITY(uSeverity)` | Set severity filter for default logger |
| `LOG_GET_SEVERITY()` | Get current severity filter from default logger |

## Usage Examples

### Basic Logging
```cpp
LOG_ERROR("Failed to open file: " << filename);
LOG_INFORMATION("Processing " << count << " items");
```

### Conditional Logging
```cpp
LOG_WARNING_IF(size > MAX_SIZE, "Buffer size exceeded: " << size);
LOG_FATAL_IF(ptr == nullptr, "Null pointer dereference prevented");
```

### Tagged Logging
```cpp
LOG_DEBUG2("network", "Connecting to " << host);
LOG_ERROR2("database", "Query failed: " << error);
```

### Multiple Loggers
```cpp
// Logger 0 for general logs
LOG_ERROR("General error occurred");

// Logger 1 for network-specific logs
LOG_ERROR_NR(1, "Network connection lost");
```

## Best Practices

1. Use severity-specific macros (`LOG_ERROR`, `LOG_WARNING`, etc.) rather than generic `LOG` when the severity is known
2. Use conditional logging (`LOG_IF`, `LOG_ERROR_IF`) when logging depends on runtime conditions
3. Use tagged logging (`LOG2`, `LOG_ERROR2`) to organize logs by component or feature
4. Use raw logging (`LOG_RAW`, `LOG_DEBUG_RAW`) when you only want to log the message without metadata
5. Remember that all logging can be disabled in production by not defining `GD_LOG_SIMPLE`

These macros provide a clean, readable interface to the logging system while maintaining the flexibility and power of the underlying implementation.