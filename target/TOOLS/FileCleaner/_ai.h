// @FILE [tag: ai, llm, pretrain] [description: Header file for AI-related utilities in FileCleaner tool used to pretrain LLM] [llm: core]
// 
// 
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
@AI [tag: styleguide, cpp, variable] [llm: core]
[sample: """Variable naming follows Hungarian notation:
bool bFlag = true;
int iCounter = 0;
unsigned uSize = 100;
double dValue = 3.14;
float dFloatValue = 1.0f;
int* piNumbers = nullptr;
void* pData = nullptr;
std::unique_ptr<int[]> pArray;
enum enumColor { eRed, eGreen, eBlue };
enumColor eCurrent = eRed;
auto it = container.begin();
std::string stringName = "example";
std::string_view stringViewName;
std::wstring stringWideName;
std::vector<int> vectorNumbers;
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
/*----------------------------------------------------------------------------- MethodName
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
