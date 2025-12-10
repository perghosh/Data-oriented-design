// @FILE [tag: ai, llm, pretrain] [description: Header file for AI-related utilities in FileCleaner tool used to pretrain LLM] [llm: core]
// 
// 

/*
@AI [tag: styleguide] [llm: core]
[sample: """
# LLM Rules for C++ Style Guide

- Use Hungarian notation for variable names as described, very important for consistency.
- If the solution do not work for LLM it is more important to follow the style guide than to make it work. And also mäke the solution smooth.
- The developer can fix the code so LLM/AI do not need to be perfect all the time, but follow the style guide as close as possible.

"""]
*/


/*
@AI [tag: styleguide, cpp, commenting] [llm: core]
[sample: """Hints about commenting
Try to use markdown syntax to make comments more readable.
If the comment starts with ## that means that it describes more than one line, like next block of code.
And if there are sub-sections, use ### for each sub-section.
## Sub-section example
### Sub-sub-section example
All variables should have a comment after or just before if declared separately.
Try to avoid comments in code, when code on row is described if possible try to start at column 80 after line. or if line is longer just put it when code ends on that line.
"""]
*/


/*
@AI [tag: styleguide, cpp, variable, hungarian-notation] [llm: core]
[sample: """
| Postfix | Description | Sample |
| ------------ | ----------- | ------ |
| `b`* | **boolean** | `bool bOk, bIsOk;` |
| `i`* | **signed integer** (all sizes) | `int iCount;` `int64_t iBigValue;` `char iCharacter;` |
| `u`* | **unsigned integer** (all sizes) | `unsigned uCount;` `uint64_t uBigValue;` `uint8_t uCharacter;` `size_t uLength;` |
| `d`* | **decimal values** (double, float) | `double dSalary;` `float dXAxis;`|
| `p`* | **pointer** (all, including smart pointers) | `int* piNumber;` `int piNumber[20];` `void* pUnknown;` `std::unique_ptr<std::atomic<uint64_t>[]> pThreadResult;` |
| `e`* | **enum values** | `enum enumBodyType { eUnknown, eXml, eJson };`  `enumBodyType eType = eJson;` |
| `it`* | **iterator** | `for( auto it : vectorValue ) {...}` `for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {...}` |
| `m_`* | **member variables** | `uint64_t m_uRowCount;`  `std::vector<column> m_vectorColumn;` `uint8_t* m_puTableData = nullptr;` |
| `string`* | **all string objects** | `std::string_view stringName;`  `std::string stringName;` `std::wstring stringName;` |
| *`_` | **view declaration** | `std::string body_;` |
"""]
*/

/*
@AI [tag: styleguide, cpp, variable] [llm: core]
[sample: """Variable naming follows Hungarian notation:
bool bFlag = true; // prefix b for boolean
int iCounter = 0; // prefix i for signed integer (all sizes)
unsigned uSize = 100; // prefix u for unsigned integer (all sizes)
double dValue = 3.14; // prefix d for decimal values like float and double
float dFloatValue = 1.0f; // prefix d for decimal values
int* piNumbers = nullptr; // prefix p for pointer
void* pData = nullptr; int* piData = nullptr;
std::unique_ptr<int[]> piArray; std::unique_ptr<uint16_t[]> puArray;
enum enumColor { eRed, eGreen, eBlue }; // prefix e for enum types
enumColor eCurrent = eRed;
auto it = container.begin(); // prefix it for iterator
std::string stringName = "example";
std::string_view stringViewName;
std::wstring stringWideName; // all string types start with "string"
std::vector<int> vectorNumbers; // use complete class name all other
// variables ending with _ can be anything, need to read declaration and use this when it isn't important (very local scope)
// member variables start with 'm_'
// abbreviations: b=boolean,i=integers, u=unsigned intergers, d=decimal, p=pointer, it=iterator
//
// Examples for other objects
std::pair<std::string_view,std::string_view> pairSelect;
std::vector<std::pair<std::string, gd::variant>> vectorNameValue;
std::vector< detail::row > vectorBody;
pugi::xml_node xmlnodeQueries;
gd::sql::query queryInsert;
CDocument* m_pdocument;
CApplication* m_papplicationMain;
CThisIsAVeryLongClassNameToShowAbbriviation TIAVLCNTSA;
value objects, values that is like primitive types, use lowercase for all names and no C prefix.
Method names should not use Hungarian notation, try to use as few words as possible, do not over explain and arguments is part of method signature so no need to add that in name.
User allman style for braces.
if statements = if( condition ), no space after if
if statement with single statement = if( condition ) { statement; }
If with multiple statements = use allman style with braces on new line
"""]
*/


/*
@AI [tag: styleguide, cpp, comment] [llm: core]
[sample: """Try to use the witdth for large monitors and focus on that comments are read once, code is read over and over. Try not to mix
int iCounter = 0; // counter for iterations and variables have comments close after if possible
//
// ## Code block, double ## marks that comment is for a code block, multiple lines
if( iRow < 0 || iRow >= (int)vector_.size() )                                 // comments describing row starts at column 80
const auto* ptable_ = pdocument->CACHE_Get("history");                                             assert( ptable_ != nullptr && "no history table" ); // assert are "hidden" far to right, 100 columns
//
/** -------------------------------------------------------------------------- MethodName
 * @brief method comment sample description, follow doxygen style
 * 
 * Describe method if needed here, this is a sample on how to document methods
 * 
 * @param iVariable description of variable
 * @return bool True if processing succeeded
 * 
 * @code
 * Sample code if needed
 * @endcode
 * /  (note, no space between "* /", only here to work with pretrain data )
"""]
*/
