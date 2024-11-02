# Data-oriented-design
DOD or data oriented design development, what is it and how to do it

## Why DOD?
- **Performance:** Faster execution times, Reduced memory consumption
- **Efficiency:** Faster development, reusable code
- **Simplicity:** Less code to manage, easier bug identification.
- **Stability:** Fewer bugs, improved system reliability
- **Developer:** Reduced stress, decreased cognitive load 


## Styleguide used in code

**Types**

| Postfix | Description | Sample |
| ------------ | ----------- | ------ |
| `b`* | **boolean** | `bool bOk, bIsOk, bIsEof, bResult;` |
| `i`* | **signed integer** (all sizes) | `int iCount;` `int64_t iBigValue;` `int16_t iPosition;` |
| `u`* | **unsigned integer** (all sizes) | `unsigned uCount;` `uint64_t uBigValue;` `uint8_t uCharacter;` `size_t uLength;` |
| `d`* | **decimal values** (double, float) | `double dSalary;` `float dXAxis;` `double dMaxValue;` |
| `p`* | **pointer** (all, including smart pointers) | `int* piNumber;` `int piNumber[20];` `void* pUnknown;` `std::unique_ptr<std::atomic<uint64_t>[]> pThreadResult;` |
| `e`* | **enum values** | `enum enumBodyType { eUnknown, eXml, eJson };`  `enumBodyType eType = eJson;` |
| `it`* | **iterator** | `for( auto it : vectorValue ) {...}` `for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {...}` |
| `m_`* | **member variables** | `uint64_t m_uRowCount;`  `std::vector<column> m_vectorColumn;` `uint8_t* m_puTableData = nullptr;` |
| `string`* | **all string objects** | `std::string_view stringName;`  `std::string stringName;` `std::wstring stringName;` |

Objects get full name in lowercase (or first character for each name in uppercase if abbreviated)  
Sample:
```cpp
std::pair<std::string_view,std::string_view> pairSelect;
std::vector<std::pair<std::string, gd::variant>> vectorNameValue;
std::vector< detail::row > vectorBody;
pugi::xml_node xmlnodeQueries;
gd::sql::query queryInsert;
CDocument* m_pdocument;
CApplication* m_papplicationMain;
CThisIsAVeryLongClassNameToShowAbbriviation TIAVLCNTSA;
```

**Scope**
| Prefix | Description | Sample |
| ------------ | ----------- | ------ |
| *`_g` | **global reach**, global methods and variables | `CApplication* papplication_g;` |
| *`_s` | **static**, like free functions and static variables within objects and methods with file scope | `static std::string m_stringCity_s;` |
| *`_d` | **debug names**, names that are used for debugging | `std::string stringCommand_d;` |

Sample:
```cpp
#ifndef NDEBUG
   std::string stringCommand_d;
   const char* pbszCommand_d = nullptr;
   stringCommand_d = name(); 
   pbszCommand_d = stringCommand_d.c_str();
#endif
```


**Code layers** 
| Type | Description |
| ------------ | ----------- |
| `general code` | Similar to stl, general code is written in lower case leters |
| `source code` | Source code can be used by any target. Each part in source is placed in some sort of namespace. Style is in PascalCase and each class starts with `C` |
| `target code` | Code in each separate target are only used in that target and isn't placed in a namespace, other than that style is similar to source code |
| `play code` | no rules, do as you wish |
| `test code` | no rules, do as you wish |

