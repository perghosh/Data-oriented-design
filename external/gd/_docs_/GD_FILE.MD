### Overview: `gd::file` – Cross-Platform File & Path Utilities

The `gd::file` header provides a **lightweight, OS-independent** set of tools for common file and path operations. It wraps and extends `std::filesystem` with simpler APIs, better error handling, and useful utilities missing from the standard library.

#### Key Components

| Component         | Description                                                                 | Best For |
|-------------------|-----------------------------------------------------------------------------|----------|
| `gd::file::path`  | Simplified, chainable path builder (like `std::filesystem::path` but smarter normalization) | Building/modifying paths |
| File I/O          | `read_file_g`, `write_file_g`, `delete_file_g`, `file_open_g`, `file_write_g` | Basic file operations |
| Known Folders     | `get_known_folder_path_g`, `get_known_folder_wpath_g`                      | User/AppData/Documents/Desktop |
| Directory Listing | `list_files_g` with filters                                                | Directory enumeration |
| Path Utils        | `fix_path_g`, `normalize_path_for_os_g`, `parent_g`, `closest_having_file_g` | Path cleaning & traversal |
| Error Handling    | All functions return `std::pair<bool, std::string>` (success + error message) | Robust scripts/tools |

#### Comparison with `std::filesystem`

| Feature                     | `std::filesystem`                     | `gd::file`                                      |
|-----------------------------|----------------------------------------|-------------------------------------------------|
| Cross-platform separator    | Yes                                    | Yes + auto-normalization                        |
| Error handling              | `std::error_code`                      | `pair<bool, string>` (simpler)                  |
| Known folders (AppData etc.)| No                                     | Yes                                             |
| Path builder chaining       | Clumsy                                 | Fluent (`+=`, `/`, `add()`)                     |
| Closest ancestor search     | No                                     | Yes (`closest_having_file_g`)                   |
| Directory listing with filter| Manual                                 | Built-in (`list_files_g`)                       |

---

### Tutorial 1: Building & Manipulating Paths

```cpp
#include "gd_file.h"
using namespace gd::file;

int main() {
    // Start with a base path
    path p = "C:/Projects/MyApp";

    // Add folders/files (automatically adds separators)
    p += "data";
    p += "logs";
    p += "error.log";

    // Or chain with initializer list
    p.add({"temp", "cache", "debug.txt"});

    // Add separator only if needed
    p.add_separator(); // no-op if already ends with /

    // Normalize (removes double separators, converts / to \ on Windows)
    std::string final = p.string(); // "C:\\Projects\\MyApp\\data\\logs\\error.log"

    // Query properties
    if (p.has_filename()) {
        std::cout << "File: " << p.filename() << "\n";          // "error.log"
        std::cout << "Extension: " << p.extension() << "\n";    // ".log"
        std::cout << "Stem: " << p.stem() << "\n";              // "error"
    }

    // Get parent folder
    path parent = p.parent_path(); // "C:\\Projects\\MyApp\\data\\logs"

    // Remove last component
    p.erase_end(); // removes "error.log"

    return 0;
}
```

---

### Tutorial 2: Reading & Writing Files

```cpp
#include "gd_file.h"
using namespace gd::file;

void example_read_write() {
    // Write text
    auto [ok_write, err_write] = write_file_g("config.txt", "user=alice\nlevel=42");
    if (!ok_write) std::cerr << "Write failed: " << err_write << "\n";

    // Read entire file
    std::string content;
    auto [ok_read, err_read] = read_file_g("config.txt", content);
    if (ok_read) {
        std::cout << "File content:\n" << content << "\n";
    }

    // Append to existing file
    int fd;
    auto [fd_ok, fd_err] = file_open_g("log.txt", true); // true = append mode
    if (fd_ok >= 0) {
        file_write_g(fd, "New log entry\n");
        file_close_g(fd);
    }
}
```

---

### Tutorial 3: Known Folders & Directory Traversal

```cpp
#include "gd_file.h"
using namespace gd::file;

void known_folders() {
    // Get standard folders
    auto [ok, path] = get_known_folder_path_g("Documents");
    if (ok) std::cout << "Documents: " << path << "\n";

    // Wide-string version
    auto [okw, wpath] = get_known_folder_wpath_g("AppData");
}

void find_config() {
    // Find nearest config file in parent hierarchy
    auto [found, config_path] = closest_having_file_g(".", "config.ini");
    if (found) {
        std::cout << "Found config at: " << config_path << "\n";
    }

    // List all .txt files in current directory
    auto files = list_files_g(".", {{"extension", ".txt"}});
    for (const auto& f : files) std::cout << f << "\n";
}
```

---

### Tutorial 4: Path Comparison & Normalization

```cpp
#include "gd_file.h"
using namespace gd::file;

void path_comparison() {
    path p1 = "C:/data/logs/../backup";
    path p2 = "C:\\data\\backup";

    // After normalization, they are equal
    assert(p1 == p2);           // true (after internal normalization)

    // Compare with std::string_view
    assert(p1 == "C:\\data\\backup"_sv);

    // Compare with filesystem::path
    assert(p1 == std::filesystem::path("C:/data/backup"));

    // Case-insensitive comparison
    path p3 = "c:\\DATA\\BACKUP";
    assert(p1 == p3);           // true (gd::file normalizes case on Windows)
}
```

---

### Quick Reference: Most Used Functions

```cpp
// Paths
path p = "base";
p += "folder/file.txt";
std::string str = p.string();           // normalized

// File I/O
read_file_g("data.txt", str_content);
write_file_g("output.log", "Hello");

// Known folders
get_known_folder_path_g("Desktop");

// Directory listing
auto txt_files = list_files_g(".", {{"extension", ".txt"}});

// Find nearest ancestor file
closest_having_file_g(".", "CMakeLists.txt");
```

---

### Summary

`gd::file` is your **Swiss Army knife** for file and path operations in C++:

- Cross-platform (Windows/Linux/macOS)
- Simple error handling
- Fluent path building
- Useful extras: known folders, closest file search, safe file I/O

Use it whenever you need to work with files/paths and want code that **just works** across operating systems without boilerplate.
