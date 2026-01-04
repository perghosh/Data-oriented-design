# Introduction: `gd::log` – Flexible, Thread-Safe Logging Framework

The `gd::log` framework is a comprehensive, highly configurable logging system designed for C++ applications that need flexible output with fine-grained control over message filtering, formatting, and destination. It provides a simple yet powerful API for logging messages to various outputs (console, file, etc.) with thread safety and minimal performance overhead.

## Core Components

### Key Classes

| Class | Purpose |
|-------|---------|
| `logger` | Central singleton instance that manages printers and filters messages |
| `message` | Container for log messages with severity, type, and content |
| `i_printer` | Abstract interface for output destinations (console, file, etc.) |

### Severity Levels

The framework supports hierarchical severity levels from most to least important:

- `eSeverityFatal` - Application-critical errors
- `eSeverityError` - Recoverable errors
- `eSeverityWarning` - Potentially problematic situations
- `eSeverityInformation` - General informational messages
- `eSeverityDebug` - Debug-specific information
- `eSeverityVerbose` - Highly detailed tracing information

## Basic Usage

### Getting Started

```cpp
#include "gd_log_logger.h"
using namespace gd::log;

// Get default logger (instance ID 0)
logger<0>* plogger = get_s();

// Add a console printer to output logs
plogger->append(std::make_unique<printer_console>());

// Set severity filter to show warnings and above
plogger->set_severity(eSeverityWarning);

// Print a simple message
plogger->print(message(eSeverityWarning) << "This is a warning message");
plogger->flush();
```

### Using the Convenience Macros

For even simpler logging, you can use macros:

```cpp
#define LOG_(uLogger, uSeverity, expression) \
    gd::log::get_g<uLogger>()->print( \
        gd::log::message(gd::log::severity_get_g(uSeverity), gd::log::eMessageTypeAll) \
        << __FILE__ << __func__ << expression)

#define LOG(uSeverity, expression) LOG_(0, uSeverity, expression)

// Usage
LOG("WARNING", "This is a warning message");
LOG(eSeverityError, "This is an error with severity enum");
```

## Advanced Features

### Multiple Loggers

The framework supports multiple isolated logger instances using template parameters:

```cpp
// Get different logger instances
logger<0>* pDefaultLogger = get_s();     // Default logger
logger<1>* pNetworkLogger = get_g<1>();  // Network-specific logger

// Each can have different printers and settings
pDefaultLogger->append(std::make_unique<printer_console>());
pNetworkLogger->append(std::make_unique<printer_file>(L"network.log"));
```

### Message Formatting

The `message` class provides various ways to format log content:

```cpp
// Simple streaming
message(eSeverityInformation) << "Value: " << 42;

// Using printf-style formatting
message(eSeverityDebug).printf("Processing %d items in %s seconds", count, time_str);

// Include automatic metadata
message(eSeverityInformation, eMessageTypeTime | eMessageTypeFileName) << "Processing complete";
```

### Tagged Logging

Use tags to filter logs for specific components or features:

```cpp
// Add a tag to a message
plogger->print(message(eSeverityInformation) << tag("network") << "Connected to server");

// Configure logger to only show messages with specific tags
plogger->set_flags(eLoggerFlagOnlyTag);
plogger->tag_add("network");

// Now only messages with "network" tag will be shown
```

### Conditional Logging

```cpp
#define LOG_IF(uSeverity, condition, expression) \
    if(!(condition)) {;} else LOG(uSeverity, expression)

// Usage
LOG_IF(eSeverityError, connection_failed, "Unable to connect to server");
```

### Thread Safety

The logger supports thread-safe operations through its template parameter:

```cpp
// Thread-safe logger (default for most operations)
logger<0, true>* pThreadSafeLogger = get_s();

// Non-thread-safe logger (slightly faster for single-threaded apps)
logger<1, false>* pSingleThreadLogger = get_g<1>();
```

## Printer Implementations

The framework includes various printer implementations:

- `printer_console` - Outputs to the standard console
- `printer_file` - Writes to a file with support for rotation

```cpp
// Add file printer with rotation
plogger->append(std::make_unique<printer_file>(L"app.log", 1024 * 1024)); // 1MB max size
```

## Error Handling

The framework provides error handling for printer failures:

```cpp
// Check for printer errors
plogger->append(std::make_unique<printer_file>(L"invalid/path.txt"));
plogger->print(message(eSeverityInformation) << "This might fail");

while (plogger->error_size() != 0) {
    auto error = plogger->error_pop();
    std::cout << "Printer error: " << error << std::endl;
}
```

## Performance Considerations

1. The logging system is designed to minimize overhead when messages are filtered out
2. Severity filtering happens before expensive string operations
3. Thread-safe logging uses mutexes only when necessary
4. Consider using the non-thread-safe logger in single-threaded contexts for better performance

## Best Practices

1. Use appropriate severity levels to enable meaningful filtering
2. Consider tagged logging for component-specific debugging
3. Use printf-style formatting for complex messages
4. Handle printer errors in production code
5. Consider separate loggers for different subsystems in large applications

The `gd::log` framework provides a robust foundation for logging in C++ applications, offering flexibility for both simple and complex logging scenarios while maintaining good performance characteristics.