[← TOC](TOC.md)

### Overview: `gd_utf8.h` – High-Performance UTF-8 String Utilities

**`gd_utf8.h`** is a lightweight, fast, and modern C++ header for working with UTF-8 encoded text. It provides a comprehensive set of functions for reading, writing, validating, converting, searching, trimming, splitting, and formatting UTF-8 strings — all with **minimal dependencies** and excellent performance.

#### Key Features

| Feature Category       | Highlights                                                                                  |
|------------------------|---------------------------------------------------------------------------------------------|
| **Character Access**   | `character()`, `next()`, `previous()`, `advance()`, `end()`                                 |
| **Size & Count**       | `count()`, `size()`, `strlen()`, `get_character_size()`                                     |
| **Conversion**         | ASCII ↔ UTF-8, UTF-16 ↔ UTF-8, JSON/XML/URI escaping, numbers ↔ text, hex ↔ binary         |
| **Validation**         | `validate()`, `validate_hex()`, `is_text()`                                                 |
| **Search & Compare**   | `find()`, `find_nth()`, `find_character()`, wildcard support                                |
| **Manipulation**       | `trim()`, `trim_left()`, `trim_right()`, `split()`, `split_pair()`, `mid()`, `indent()`     |
| **Formatting**         | `quoted()`, `quoted_if_text()`, `print_hex()`, `pad_left/right()`, `replace()`              |
| **Specialized**        | `uri::`, `json::`, `xml::` namespaces for protocol-specific escaping & validation          |

#### Why Use It?

- **Faster than `std::wstring_convert` / ICU** for most common tasks
- **Zero-allocation** where possible
- **Modern C++** (string_view, initializer_list, templates)
- **No heavy dependencies** — just `<string>`, `<string_view>`, `<vector>`

---

### Simple Usage Tutorials

#### 1. Reading & Iterating UTF-8 Characters

```cpp
#include "gd_utf8.h"
using namespace gd::utf8;

void example_iterate() {
    std::string_view text = "Hello 世界! 😊";

    const uint8_t* p = reinterpret_cast<const uint8_t*>(text.data());
    const uint8_t* end = p + text.size();

    while (p < end) {
        uint32_t ch = character(p);           // Get Unicode code point
        std::cout << "U+" << std::hex << ch << " ";
        p = next(p);                          // Move to next character
    }
    // Output: U+48 U+65 U+6c U+6c U+6f U+20 U+4e16 U+754c U+21 U+20 U+1f60a
}
```

#### 2. Counting Characters & Bytes

```cpp
std::string_view s = "café ☕";

auto [char_count, _] = count(s);           // 6 characters
uint32_t byte_size = size(s);              // 10 bytes

std::cout << "Characters: " << char_count << ", Bytes: " << byte_size << "\n";
```

#### 3. Trimming Whitespace

```cpp
std::string_view dirty = "   Hello World!  \n\t";

std::string_view clean = trim(dirty, tag_view{});   // "Hello World!"

std::cout << "'" << clean << "'\n";                 // 'Hello World!'
```

#### 4. Splitting Strings

```cpp
std::string_view csv = "name,age,city";

auto parts = split(csv, ',');  // returns std::vector<std::string_view>

for (auto& part : parts) {
    std::cout << part << "\n"; // name, age, city
}
```

#### 5. Converting Between Encodings

```cpp
std::wstring wide = L"こんにちは";
std::string utf8;

// UTF-16 → UTF-8
convert_utf16_to_uft8(wide.c_str(), utf8);

// UTF-8 → ASCII (non-ASCII characters dropped)
std::string ascii = convert_utf8_to_ascii(utf8);
```

#### 6. JSON Escaping

```cpp
std::string_view raw = "Hello \"world\"\n\t\\";
std::string escaped = json::convert_utf8_to_json(raw);

std::cout << escaped << "\n"; // Hello \"world\"\n\t\\
```

#### 7. URI/URL Escaping

```cpp
std::string_view unsafe = "hello world & test";
std::string safe;
uri::convert_utf8_to_uri(unsafe, safe);

std::cout << safe << "\n"; // hello%20world%20%26%20test
```

#### 8. Finding Substrings

```cpp
std::string_view text = "The quick brown fox jumps over the lazy dog";
auto pos = find(text, "fox");    // returns string_view starting at "fox"
if (!pos.empty()) {
    std::cout << pos << "\n";    // fox jumps over the lazy dog
}
```

---

### Summary

**`gd_utf8.h`** is your go-to for **fast, safe, and convenient UTF-8 string handling** in modern C++.

**Use the simple wrappers** like `trim()`, `split()`, `count()`, `find()`, `convert_*()` for everyday tasks.

**Use the low-level functions** (`character()`, `next()`, `size()`, etc.) when you need maximum performance or custom iteration logic.

Perfect for:
- Parsing user input / JSON / XML / URLs
- Text processing pipelines
- Cross-platform applications
- Performance-critical string code

UTF-8 without too much headache!
