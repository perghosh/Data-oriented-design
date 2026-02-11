[← TOC](TOC.md)

# Overview: `gd::cli::options` – Command-Line Interface Parser

`gd::cli::options` is a **comprehensive command-line interface (CLI) parsing library** that allows developers to define, parse, and retrieve command-line arguments in a structured way.

It's perfect for:
- Building CLI applications with multiple subcommands
- Parsing and validating command-line arguments
- Generating help documentation automatically
- Creating hierarchical command structures
- Applications requiring complex argument handling

## Key Features

- **Subcommand support** → hierarchical command structures
- **Type-safe argument parsing** → automatic type conversion
- **Help documentation generation** → multiple output formats
- **Global options** → shared across subcommands
- **Flexible input parsing** → from various sources
- **Alias support** → shortcuts for common commands

## Architecture Overview

The library organizes CLI options in a hierarchical structure:
```
Main Options
├── Global Options (shared across subcommands)
├── Subcommand 1
│   ├── Options specific to Subcommand 1
│   └── ...
├── Subcommand 2
│   ├── Options specific to Subcommand 2
│   └── ...
└── ...
```

## Basic Usage

```cpp
#include "gd_cli_options.h"
using namespace gd::cli;  // options = cli::options

int main(int argc, const char* argv[]) {
    // 1. Create main options object
    options cli("myapp", "A sample application");

    // 2. Add options
    cli.add({ "help", 'h', "Display help information" });
    cli.add({ "verbose", 'v', "Enable verbose output" });
    cli.add({ "input", 'i', "Input file" });
    cli.add({ "output", 'o', "Output file" });

    // 3. Parse command-line arguments
    auto [success, error] = cli.parse(argc, argv);
    if (!success) {
        std::cerr << "Error: " << error << std::endl;
        return 1;
    }

    // 4. Handle options
    if (cli.exists("help")) {
        std::string doc;
        cli.print_documentation(doc, options::tag_documentation_table{});
        std::cout << doc << std::endl;
        return 0;
    }

    // 5. Access option values
    if (cli.exists("input")) {
        std::string file = cli.get_variant("input").as_string();
        std::cout << "Input file: " << file << std::endl;
    }

    return 0;
}
```

## Advanced Usage with Subcommands

```cpp
// Create subcommands
options process_cmd("process", "Process data files");
process_cmd.add({ "input", 'i', "Input file" });
process_cmd.add({ "format", 'f', "Output format" });

options analyze_cmd("analyze", "Analyze data files");
analyze_cmd.add({ "input", 'i', "Input file" });
analyze_cmd.add({ "report", 'r', "Generate report" });

// Add to main options
options cli("myapp", "Data processing tool");
cli.add({ "verbose", 'v', "Enable verbose output" });
cli.sub_add(process_cmd);
cli.sub_add(analyze_cmd);

// Parse and handle
auto [success, error] = cli.parse(argc, argv);
const options* active = cli.sub_find_active();

if (active && active->name() == "process") {
    // Handle process command
    std::string input = active->get_variant("input").as_string();
    std::cout << "Processing " << input << std::endl;
}
```

## Option Types and Flags

```cpp
// Basic flag (boolean option)
options.add({ "verbose", 'v', "Enable verbose output" });

// Option with value
options.add({ "input", 'i', "Input file" });

// Option with explicit type
options.add({ "count", gd::types::type_g("uint32"), "Number of items" });

// Global option (valid for all subcommands)
options.add({ "debug", 'd', "Enable debug mode" }.flags(options::eOptionFlagGlobal));

// Option that can only be used alone
options.add({ "version", 'V', "Show version" }.flags(options::eOptionFlagSingle));
```

## Parsing Different Input Formats

```cpp
// From argc/argv (standard main function parameters)
auto [success, error] = options.parse(argc, argv);

// From a single command string
auto [success, error] = options.parse("process --input file.txt --output result.txt");

// From a vector of strings
std::vector<std::string> args = {"process", "--input", "file.txt"};
auto [success, error] = options.parse(args);

// From a terminal-style command string
auto [success, error] = options.parse_terminal("myapp process -i file.txt -o result.txt");
```

## Documentation Generation

```cpp
// Table format
std::string doc_table;
options.print_documentation(doc_table, options::tag_documentation_table{});

// Dense format
std::string doc_dense;
options.print_documentation(doc_dense, options::tag_documentation_dense{});

// Verbose format
std::string doc_verbose;
options.print_documentation(doc_verbose, options::tag_documentation_verbose{});
```

## When to Use `gd::cli::options`

| Scenario                              | Recommended? | Why |
|---------------------------------------|--------------|-----|
| Simple CLI apps with few options      | Yes          | Quick setup and parsing |
| Complex CLI with subcommands          | Yes          | Hierarchical structure support |
| Applications requiring auto-help       | Yes          | Built-in documentation generation |
| Type-safe argument parsing            | Yes          | Automatic type conversion |
| Very simple argument parsing (1-2 args) | Maybe        | Might be overkill |
| GUI applications                       | No           | Designed for CLI only |

## Summary

`gd::cli::options` is the **go-to choice** for building command-line interfaces in C++ applications. It combines:

- Subcommand support for complex CLIs
- Type-safe argument parsing
- Automatic documentation generation
- Flexible input parsing from various sources
- Global options and aliases

Use it whenever you need to build a robust CLI application — it's the "Command-Line Interface" pattern done right for modern C++.
