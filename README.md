DOD or data oriented design development, what is it and how to do it

## Data Oriented Design - Pros and cons
**Pros**
- **Performance:** Faster execution times, reduced memory consumption
- **Efficiency:** Faster development, reusable code
- **Simplicity:** Less code to manage, easier bug identification.
- **Stability:** Fewer bugs, improved system reliability
- **Developer:** Reduced stress, decreased cognitive load

**Cons**
- Initial development time may be longer as developers adapt to a new paradigm.
- Requires a deeper understanding of computer architecture and low-level programming concepts.
- Very different from how humans think.

### Data Oriented Design in one sentence = "Format data to maximize CPU efficiency"

# Data Oriented Design - why do it

**Ultimately, computers only understand machine code and binary data. This is the foundation of data-oriented design.**

Data Oriented Design = build logic around patterns, patterns that works well for the CPU.  
  
A computer's central processing unit (CPU) can only execute machine code, a binary language of 0s and 1s. Programming languages like C++, C# or Java, must be converted into this machine code before the computer can understand and process it. Tools like compilers and interpreters, are used to convert programming code to machine code, the computer's native language.  
While computers may seem limited, understanding only a few simple instructions represented by 0s and 1s, their speed and precision are extreme. This makes them usefull in a wide range of tasks.  
  
The challenge for developers is translating complicated human information into a computer-processable format, ensuring that the computer's output is usefull, accurate and user-friendly. This is not as easy as you might think.  
  
The amount of code needed to achieve this can vary significantly and may sometimes be massive. Beyond just being massive, the code must also be correct, manageable, and often easy to extend or modify to accommodate new functionality. Unfortunately, the world humans live in is constantly changing, making adaptability a crucial aspect of software design.  
You wouldn’t want to spend thousands of hours creating computer programs for a specific task, only to discover later that circumstances have changed.  


### Developer challenges
The challenging part of software development isn’t learning a programming language to write code—almost anyone can do that if they’re interested. The real difficulty lies in learning how to manage code effectively: ensuring the compiled machine code is error-free, maintainable, and adaptable for future modifications.

### Keep the house in order and clean - a metaphor
A blank slate of a home is a designer's dream. In the beginning, everything has its place, what to add and finding it is a breeze. Before you know it, a house can become overwhelmed with possessions. Maintaining order is key to easily finding what you need.  
A home needs order and flexibility to remain pleasant and functional. Mess up the home and even the nicest house is not as pleasant any more.  

Same with code. A new codebase is a dream, easy to add stuff and easy to understand. Surprisingly quickly, you may find yourself spending time in locating specific code segments or understanding their intended behavior.
  
### Keep the code in order and clean
Just as homes have designated spaces for specific functions—kitchens for cooking, bedrooms for sleeping, and bathrooms for personal hygiene—computers have core functionalities and operating systems that govern their behavior. Developers must understand these areas to maintain clean and organized code.  
What software often do is to collect information from users, store it in internal memory, process it as needed, provide feedback to the user, and potentially save it for future use.  
A significant portion of a developer's work involves writing code for reading and writing data to various locations, often converting it into formats that are easily understood and utilized by users.

### Code explodes in size
When data is close to users, it should also be presented in a format and structure that they can easily understand. Human-readable data formats are fundamentally different from what works well for computers to process.  
On the positive side this code is simple to read because you can describe it in terms that is easy to understand but a lot more code is needed when data is in Human-readable data format.  
Code that directly interacts with users, often referred to as the User Interface (UI), typically consists of, or should strive for, declarative programming paradigms (declarative programming contrasting sharply with imperative programming).  
  
Early transformation when data leaves user or late when it reaches user into or away from a machine-friendly format is crucial for streamlining code and improving efficiency. 
This suggests that when a web browser acts as the user interface, the separation of concerns between domain logic and data structures optimized for computer processing can be effectively implemented within the browser environment.

**In scenarios where domain logic is defined within the code...**  
Keeping human-readable data formats for inter-system communication will necessitate extensive conversion logic, leading to increased code size, potential performance bottlenecks and code that are difficult to adapt to changes.  
Experienced developers can often spot the maintainability of a codebase by simply reading portions of it and evaluating how deeply the domain logic is intertwined with the underlying technology. This evaluation provides a strong indication of the challenges that may arise when working with the code.
Domain logic going deep is not good. Try to avoid it

## Let's get practical. How about using "string"?
C++ string objects, such as `std::string`, are designed to efficiently manage text within code.
The `std::string` class exemplifies data-oriented design, offering a clear interface for string manipulation. It leverages a well-known data pattern that most C++ developers are familiar with.
A sequence of bytes terminated by a zero byte. `std::string` and many other string implementations are based on this zero-terminated byte sequence pattern.  
This pattern is CPU-friendly, cache-efficient, and easy to manipulate. Its extensive usage demonstrates the ease of reusing data-oriented code in various contexts. 
And it's easy to see why separating domain logic from data pattern logic is crucial. Injecting domain logic into data-oriented code designed for specific patterns immediately destroys it.

There are numerous implementations for common data patterns like JSON and XML, making it easy to find shared code solutions. Developing custom implementations for internal data patterns used within applications is often easier than one might think.
By investing time upfront to create well-structured, reusable code, you can preserve its value even if the domain changes. While this requires more initial thought and effort, the long-term gains are significant.

Programming languages like C++, JavaScript, C#, Python, HTML, CSS, SQL, and others exemplify common data patterns for developers.
While a pattern can take many forms, its effectiveness for internal application logic hinges on how easily a computer can process it and how hard it is to implement for developers. This makes it relatively straightforward to determine whether a pattern is well-suited for a given task.

----

## Styleguide used in code is based on Hungarian Notation
*Why write code with this style*

- Improved Code Search and Navigation:
  -  Consistent naming conventions make it incredibly easy to search for specific variables across the entire codebase. This significantly speeds up debugging, refactoring, and understanding existing code.

- Enhanced Code Readability and Collaboration:
  -  When all developers adhere to the same naming conventions, the code becomes much easier to read and understand, even for developers who are unfamiliar with specific parts of the project. This fosters better collaboration and knowledge sharing within the team.

- Reduced Cognitive Load and Improved Flow:
  - When developers can quickly grasp what variables are.  it significantly reduces the mental effort required to understand code to work with it. Not needing to remember variables frees up cognitive resources. When variable names clearly indicate their type or purpose, developers can more easily identify the relevant parts of the code that require their attention, without being distracted by irrelevant details.
  
- Increased Development Speed:
  - With a well-defined set of naming conventions, developers can quickly select appropriate names for variables, reducing the time spent on trivial decisions. This can lead to a noticeable increase in development speed and overall productivity.

- Forced Reflection on Object Purpose:
  - If a variable containing the object name is difficult to understand, it often signals an issue with the object's name itself. Developers are forced to critically evaluate the object's purpose and consider if a more suitable name exists.


**Types**

| Postfix | Description | Sample |
| ------------ | ----------- | ------ |
| `b`* | **boolean** | `bool bOk, bIsOk, bIsEof, bResult;` |
| `i`* | **signed integer** (all sizes) | `int iCount;` `int64_t iBigValue;` `int16_t iPosition; char iCharacter;` |
| `u`* | **unsigned integer** (all sizes) | `unsigned uCount;` `uint64_t uBigValue;` `uint8_t uCharacter;` `size_t uLength;` |
| `d`* | **decimal values** (double, float) | `double dSalary;` `float dXAxis;` `double dMaxValue;` |
| `p`* | **pointer** (all, including smart pointers) | `int* piNumber;` `int piNumber[20];` `void* pUnknown;` `std::unique_ptr<std::atomic<uint64_t>[]> pThreadResult;` |
| `e`* | **enum values** | `enum enumBodyType { eUnknown, eXml, eJson };`  `enumBodyType eType = eJson;` |
| `it`* | **iterator** | `for( auto it : vectorValue ) {...}` `for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {...}` |
| `m_`* | **member variables** | `uint64_t m_uRowCount;`  `std::vector<column> m_vectorColumn;` `uint8_t* m_puTableData = nullptr;` |
| `string`* | **all string objects** | `std::string_view stringName;`  `std::string stringName;` `std::wstring stringName;` |
| *`_` | **view declaration** | `boost::beast::http::file_body::value_type body_;` |

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
Note that in situations where type is so "strange" that is difficult to select proper name then end name with `_`. Ending with _ (underscore) means that you can name it to whatever you want.

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

### Learn while reading code
Developers read a lot of code. To improve their skills, they should write code that's easy to learn from. 

This coding style uses abbreviations for primitive C++ types and common names for frequently used objects. Object names are prefixed with class name to make it clear what it is. Other than that, code is not abbreviated. This approach helps developers focus on domain objects and key variables, rather than getting bogged down with reading EVERYTHING.
It reduces cognitive load, as our short-term memory can typically hold about 5-8 names for about 20 seconds and impedes the cognitive resources necessary for process what code does. 
By minimizing cognitive overhead on temporary or irrelevant details, this coding style prioritizes skill in development and efficient comprehension.

If you write 100 lines of code, maybe 10 lines are important to understand what it does. If it is easy to find the important code, the rest can be skipped. Code is easer to read.
