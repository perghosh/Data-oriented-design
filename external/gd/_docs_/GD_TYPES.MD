# Core Type System: `gd::types` – Foundation for GD Library

The `gd_types.h` header defines the fundamental type system that underpins the entire GD library. This system provides a unified way to identify, classify, and work with different data types throughout the library components.

## Overview

The GD type system is built around the concept of assigning unique numeric identifiers to data types and providing comprehensive metadata about their properties. This enables:

- Type-agnostic operations that work correctly regardless of the actual data type
- Zero-cost abstractions through tag dispatching
- Efficient type checking and validation
- Seamless data exchange between different components

## Key Concepts

### Type Numbers

Each type in the system has a unique number assigned through `enumTypeNumber`:

```cpp
enum enumTypeNumber
{
   eTypeNumberUnknown = 0,
   eTypeNumberBool = 1,
   eTypeNumberInt8 = 2,
   eTypeNumberUInt8 = 3,
   // ... more types
   eTypeNumberString = 14,
   eTypeNumberUtf8String = 15,
   // ... up to eTypeNumberMAX
};
```

### Type Groups

Types are organized into groups using bit flags in `enumTypeGroup`:

```cpp
enum enumTypeGroup
{
   eTypeGroupNumber = 0x0000'0100,     // Number types
   eTypeGroupInteger = 0x0000'0200,    // Integer types
   eTypeGroupDecimal = 0x0000'0400,    // Floating point types
   eTypeGroupString = 0x0000'1000,     // String types
   eTypeGroupBinary = 0x0000'4000,     // Binary data
   eTypeGroupBoolean = 0x0000'8000,     // Boolean types
   // Size groups
   eTypeGroupSize08 = 0x0001'0000,     // 8-bit types
   eTypeGroupSize16 = 0x0002'0000,     // 16-bit types
   // ... etc
};
```

### Combined Type Information

The complete type information is stored in `enumType`, which combines type number, group, and size flags:

```cpp
enum enumType
{
   eTypeUnknown = eTypeNumberUnknown,
   eTypeBool = eTypeNumberBool | eTypeGroupBoolean | eTypeGroupSize08,
   eTypeInt8 = eTypeNumberInt8 | (eTypeGroupInteger|eTypeGroupNumber) | eTypeGroupSize08 | eTypeGroupSigned,
   // ... more type definitions
};
```

## Tag Dispatchers

The system uses tag dispatching for compile-time type resolution without runtime overhead:

```cpp
// Command related tags
struct tag_command_add {};
struct tag_command_remove {};
// ... more command tags

// Type related tags
struct tag_type_unsigned {};
struct tag_type_signed {};
// ... more type tags

// State related tags
struct tag_state_active {};
struct tag_state_canceled {};
// ... more state tags

// ... many more tag types for different categories
```

## Type Helper Functions

### Extracting Type Information

```cpp
// Extract specific parts from a combined type value
constexpr uint32_t type_number_g(unsigned uType);     // Get type number
constexpr uint32_t type_group_g(unsigned uType);      // Get type group
constexpr uint32_t type_size_g(unsigned uType);       // Get type size

// Type checking functions
constexpr bool is_boolean(unsigned uType);             // Check if boolean type
constexpr bool is_number(unsigned uType);              // Check if number type
constexpr bool is_integer(unsigned uType);             // Check if integer type
constexpr bool is_decimal(unsigned uType);             // Check if decimal type
constexpr bool is_string(unsigned uType);              // Check if string type
// ... more type checking functions
```

### Character Type Classification

```cpp
// Character type lookup tables
extern const uint8_t puCharType_g[0x100];      // Character types
extern const uint16_t puCharGroup_g[0x100];    // Character groups

// Get character type by name
uint8_t ctype_g(const std::string_view& stringCType, tag_main_type);
uint16_t ctype_g(const std::string_view& stringCType);

// Detect if text is integer, decimal, or text
unsigned detect_ctypegroup_g(const uint8_t* puText, unsigned uLength);
```

### Type Name Conversions

```cpp
// Get type name by type number
constexpr std::string_view type_name_g(unsigned uType);

// Convert type number to type
constexpr enumType typenumber_to_type_g(unsigned uTypeNumber);

// Get all types that match certain criteria
std::vector<unsigned> types_g(unsigned uGroupFilter);
```

## Special Utility Types

### Character Name Type

```cpp
struct cname {
   std::string_view m_stringCName;
   // ... constructors and operators
};

// Literal operator for creating character names
cname operator ""_ctype(const char* psz, size_t uLength);
```

### UUID Wrapper

```cpp
struct uuid {
   uint8_t m_puData[16];
   // ... methods for comparison and access
};
```

### Binary Data Wrapper

```cpp
struct binary {
   const uint8_t* m_puData;
   size_t m_uLength;
   // ... methods for comparison and access
};
```

## Type Traits

The header also provides type traits for STL containers:

```cpp
template<typename T>
struct is_list : std::false_type {};

template<typename T, typename Alloc>
struct is_list<std::list<T, Alloc>> : std::true_type {};

template<typename T>
struct is_vector : std::false_type {};

template<typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

template<typename T>
struct is_variant : std::false_type {};

template<typename... Types>
struct is_variant<std::variant<Types...>> : std::true_type {};
```

## Usage Examples

### Basic Type Operations

```cpp
// Check if a type is a number
if (gd::types::is_number(eTypeInt32)) {
   // Handle integer value
}

// Extract type information
uint32_t uTypeNumber = gd::types::type_number_g(eTypeInt32);  // 6
uint32_t uTypeGroup = gd::types::type_group_g(eTypeInt32);    // eTypeGroupInteger | eTypeGroupNumber
uint32_t uTypeSize = gd::types::type_size_g(eTypeInt32);      // eTypeGroupSize32

// Get type name
std::string_view stringTypeName = gd::types::type_name_g(eTypeInt32);  // "int32_t"
```

### Character Type Detection

```cpp
// Detect if a string represents a number
std::string_view stringText = "12345";
unsigned uTypeGroup = gd::types::detect_ctypegroup_g(stringText);

if (uTypeGroup & gd::types::eTypeGroupNumber) {
   // Text represents a number
}
```

### Tag-Based Method Overloading

```cpp
// Using tag dispatchers for compile-time method selection
template<typename T>
void process_value(const T& value, gd::types::tag_type_integer) {
   // Integer-specific processing
}

template<typename T>
void process_value(const T& value, gd::types::tag_type_decimal) {
   // Decimal-specific processing
}

template<typename T>
void process_value(const T& value) {
   if constexpr (gd::types::is_integer(gd::types::type_g<T>())) {
      process_value(value, gd::types::tag_type_integer{});
   } else if constexpr (gd::types::is_decimal(gd::types::type_g<T>())) {
      process_value(value, gd::types::tag_type_decimal{});
   }
   // ... handle other types
}
```

## Alignment Support

The system also provides alignment helpers:

```cpp
enum enumAlign {
   enumAlignLeft = 0x0001,
   enumAlignRight = 0x0002,
   enumAlignCenter = 0x0004,
   // ... other alignment options
};

// Get default alignment for a type
unsigned align_g(unsigned uType);
```

## Summary

The GD type system is a comprehensive foundation that enables type-agnostic operations throughout the library while maintaining type safety. By combining numeric type identifiers with rich metadata, it provides:

- Zero-cost abstractions through tag dispatching
- Efficient type checking and validation
- Flexible data exchange between components
- Compile-time type resolution

Understanding this type system is essential for working with the GD library, as it's used consistently across all components for type identification, validation, and processing.