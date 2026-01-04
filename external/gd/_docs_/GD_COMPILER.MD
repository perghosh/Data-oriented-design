### Description: `gd_compiler.h`

**Lightweight compiler & C++ standard detection header**

Detects:
- Compiler: GCC, Clang, MSVC
- C++ standard support: C++17, C++20, C++23
- Platform: Windows, Linux, macOS, iOS, PlayStation

Provides clean, global macros:
```cpp
GD_COMPILER_HAS_CPP20_SUPPORT
GD_COMPILER_HAS_CPP23_SUPPORT
```

Perfect for writing portable code that adapts to available C++ features without complex build system checks.
