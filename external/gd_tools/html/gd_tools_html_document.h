// @FILE [tag: html,dom,parser] [description: HTML/XML document model and fast single-pass parser] [type: header] [name: gd_tools_html_document.h]

#pragma once

#include <array>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd/gd_types.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#ifndef _GD_TOOLS_HTML_BEGIN
#  define _GD_TOOLS_HTML_BEGIN namespace gd { namespace tools { namespace html {
#  define _GD_TOOLS_HTML_END   } } }
#endif

_GD_TOOLS_HTML_BEGIN

// ============================================================================
// @CLASS [tag: element] [summary: Single node in the HTML/XML DOM tree]
// Stores the tag name, text content, attributes, children, and a back-pointer
// to the parent. Ownership of children flows downward (unique_ptr).
// ============================================================================

/** ------------------------------------------------------------------------- element
 * Represents a tree node for XML/HTML-like document structures, supporting hierarchical elements with attributes and text content.
 */
struct element
{
   // ## types and constants -------------------------------------------------
   using tag_id = gd::types::tag_id; ///< tag for id attribute, or id related logic
   using tag_class = gd::types::tag_class; ///< tag for class attribute, or class related logic

   // ## iterators -----------------------------------------------------------



   /** =======================================================================
    * @brief Iterator types for traversing child elements of an `element` node.
    * 
    * The `iterator` and `const_iterator` types provide forward iteration over 
    * the immediate children of an `element`, while `tree_iterator` and `const_tree_iterator`
    * perform a pre-order depth-first traversal of the entire subtree rooted 
    * at the given element.
    */
   struct iterator
   {
      using iterator_category = std::forward_iterator_tag;
      using value_type        = element;
      using difference_type   = std::ptrdiff_t;
      using pointer           = element*;
      using reference         = element&;

      iterator() = default;
      explicit iterator( std::vector<std::unique_ptr<element>>::iterator itElement )
         : m_itElement( itElement ) {}

      reference  operator*()  const { return **m_itElement; }
      pointer    operator->() const { return m_itElement->get(); }
      iterator&  operator++()       { ++m_itElement; return *this; }
      iterator   operator++(int)    { auto itCopy_ = *this; ++m_itElement; return itCopy_; }
      bool operator==( const iterator& o ) const noexcept { return m_itElement == o.m_itElement; }
      bool operator!=( const iterator& o ) const noexcept { return m_itElement != o.m_itElement; }

   private:
      std::vector<std::unique_ptr<element>>::iterator m_itElement;           ///< Position in child vector
   };

   struct const_iterator
   {
      using iterator_category = std::forward_iterator_tag;
      using value_type        = const element;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const element*;
      using reference         = const element&;

      const_iterator() = default;
      explicit const_iterator( std::vector<std::unique_ptr<element>>::const_iterator itElement )
         : m_itElement( itElement ) {}

      reference       operator*()  const { return **m_itElement; }
      pointer         operator->() const { return m_itElement->get(); }
      const_iterator& operator++()       { ++m_itElement; return *this; }
      const_iterator  operator++(int)    { auto itCopy_ = *this; ++m_itElement; return itCopy_; }
      bool operator==( const const_iterator& o ) const noexcept { return m_itElement == o.m_itElement; }
      bool operator!=( const const_iterator& o ) const noexcept { return m_itElement != o.m_itElement; }

   private:
      std::vector<std::unique_ptr<element>>::const_iterator m_itElement;     ///< Position in child vector
   };


   /** =======================================================================
    * @brief Pre-order depth-first iterators for traversing an entire subtree of `element` nodes.
    * 
    * The `tree_iterator` and `const_tree_iterator` types maintain an explicit 
    * stack of pointers to the next nodes to visit, allowing them to traverse the
    * entire subtree in a single pass without recursion. The children of each node
    * are pushed onto the stack in reverse order so that they are visited in document order.
    */
   struct tree_iterator
   {
      using iterator_category = std::forward_iterator_tag;
      using value_type        = element;
      using difference_type   = std::ptrdiff_t;
      using pointer           = element*;
      using reference         = element&;

      tree_iterator() = default;                                              ///< End sentinel — empty stack
      explicit tree_iterator( element* pelementStart )
      {
         if( pelementStart != nullptr ) { m_vectorpElement.push_back( pelementStart ); }
      }

      reference      operator*()  const { return *m_vectorpElement.back(); }
      pointer        operator->() const { return  m_vectorpElement.back(); }

      /** ---------------------------------------------------------------------- operator++
       * @brief Advance in pre-order DFS: pop current, push its children in reverse.
       * @return tree_iterator& This iterator, now pointing at the next node
       */
      tree_iterator& operator++()
      {
         element* pelementCurrent = m_vectorpElement.back();
         m_vectorpElement.pop_back();
         for( auto itChild = pelementCurrent->m_vectorElement.rbegin();    // reverse so first child is next
              itChild != pelementCurrent->m_vectorElement.rend(); ++itChild )
         {
            m_vectorpElement.push_back( itChild->get() );
         }
         return *this;
      }

      tree_iterator operator++(int) { auto itCopy_ = *this; ++(*this); return itCopy_; }

      bool operator==( const tree_iterator& o ) const noexcept { return m_vectorpElement == o.m_vectorpElement; }
      bool operator!=( const tree_iterator& o ) const noexcept { return m_vectorpElement != o.m_vectorpElement; }

   private:
      std::vector<element*> m_vectorpElement;                                ///< DFS (Depth-First Search) frontier; empty == end
   };

   struct const_tree_iterator
   {
      using iterator_category = std::forward_iterator_tag;
      using value_type        = const element;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const element*;
      using reference         = const element&;

      const_tree_iterator() = default;
      explicit const_tree_iterator( const element* pelementStart )
      {
         if( pelementStart != nullptr ) { m_vectorpElement.push_back( pelementStart ); }
      }

      reference            operator*()  const { return *m_vectorpElement.back(); }
      pointer              operator->() const { return  m_vectorpElement.back(); }

      const_tree_iterator& operator++()
      {
         const element* pelementCurrent = m_vectorpElement.back();
         m_vectorpElement.pop_back();
         for( auto itChild = pelementCurrent->m_vectorElement.rbegin();
              itChild != pelementCurrent->m_vectorElement.rend(); ++itChild )
         {
            m_vectorpElement.push_back( itChild->get() );
         }
         return *this;
      }

      const_tree_iterator operator++(int) { auto itCopy_ = *this; ++(*this); return itCopy_; }

      bool operator==( const const_tree_iterator& o ) const noexcept { return m_vectorpElement == o.m_vectorpElement; }
      bool operator!=( const const_tree_iterator& o ) const noexcept { return m_vectorpElement != o.m_vectorpElement; }

   private:
      std::vector<const element*> m_vectorpElement;                          ///< DFS (Depth-First Search) frontier; empty == end
   };


   element() = default;
   explicit element( std::string_view stringName ) : m_stringName( stringName ) {}
   explicit element( std::string_view stringName, std::string_view stringContent ): m_stringName( stringName ), m_stringContent( stringContent ) {}

   // copy / move
   element( const element& o )            { common_construct( o ); }
   element( element&& o ) noexcept        { common_construct( std::move( o ) ); }
   element& operator=( const element& o ) { common_construct( o ); return *this; }
   element& operator=( element&& o ) noexcept
   {
      if( this != &o )
      {
         m_stringName         = std::move( o.m_stringName );
         m_stringContent      = std::move( o.m_stringContent );
         m_pelementParent     = o.m_pelementParent;
         m_vectorElement      = std::move( o.m_vectorElement );
         m_argumentsAttribute = std::move( o.m_argumentsAttribute );
         o.m_pelementParent   = nullptr;
         for( auto& pChild : m_vectorElement ) { pChild->m_pelementParent = this; } // update parent pointers in moved children
      }
      return *this;
   }
   ~element() = default;

   void common_construct( const element& o );

// ## index operators ---------------------------------------------------------
   element*       operator[]( size_t uIndex )       { return m_vectorElement[uIndex].get(); }
   const element* operator[]( size_t uIndex ) const { return m_vectorElement[uIndex].get(); }

// ## getters / setters -------------------------------------------------------
   std::string_view  name()    const noexcept { return m_stringName; }
   std::string_view  content() const noexcept { return m_stringContent; }
   element*          parent()  const noexcept { return m_pelementParent; }

   void set_parent( element* pelementParent )         { m_pelementParent = pelementParent; }
   void set_content( std::string_view stringContent ) { m_stringContent  = stringContent;  }
   void append_content( std::string_view stringText );

// ## child management --------------------------------------------------------
   element* add( std::unique_ptr<element> pelementChild );

   /// Reserve space for expected child count to avoid reallocations
   void reserve( size_t uChildCount ) { m_vectorElement.reserve( uChildCount ); }

   element*       at( size_t uIndex )       { return m_vectorElement.at( uIndex ).get(); }
   const element* at( size_t uIndex ) const { return m_vectorElement.at( uIndex ).get(); }
   element*       front()                   { return m_vectorElement.front().get(); }
   const element* front() const             { return m_vectorElement.front().get(); }
   element*       back()                    { return m_vectorElement.back().get(); }
   const element* back()  const             { return m_vectorElement.back().get(); }
   size_t         size()  const noexcept    { return m_vectorElement.size(); }
   bool           empty() const noexcept    { return m_vectorElement.empty(); }

// ## attribute management ----------------------------------------------------
   void             add_attribute( std::string_view stringName, std::string_view stringContent );
   std::string_view get_attribute( std::string_view stringName ) const { return m_argumentsAttribute[stringName].as_string_view(); }
   bool             has_attribute( std::string_view stringName ) const { return m_argumentsAttribute.exists( stringName ); }
   void             remove_attribute( std::string_view stringName ) { m_argumentsAttribute.remove( stringName ); }
   size_t           size_attribute() const noexcept { return m_argumentsAttribute.size(); }

   auto             attributes() const noexcept { return m_argumentsAttribute.named(); }

// ## traversal -------------------------------------------------------------

   size_t size_parents() const noexcept;
   size_t size_all() const noexcept;
   std::vector<element*> parents() const;  ///< Ancestors from parent up to root, in order
   const element* closest( std::string_view stringTag ) const;


   /** ----------------------------------------------------------------------- find
    * @brief First depth-first descendant whose tag name matches (case-insensitive)
    * @param stringTag Tag name to search for
    * @return element* First matching element, or nullptr
    */
   element*       find( std::string_view stringTag );
   const element* find( std::string_view stringTag ) const;

   /** -------------------------------------------------------------------------- find_all
    * @brief Collect every descendant whose tag name matches (case-insensitive)
    * @param stringTag        Tag name to search for
    * @param vectorResult     Accumulator — matching pointers are appended here
    */
   void find_all( std::string_view stringTag, std::vector<element*>& vectorResult );
   void find_all( std::string_view stringTag, std::vector<const element*>& vectorResult ) const;

   
   element*       find( std::string_view stringIdValue, tag_id );            /// @brief First descendant (or self) where attribute `id` equals id in elment id attribute
   const element* find( std::string_view stringIdValue, tag_id ) const;      /// @brief First descendant (or self) where attribute `id` equals id in elment id attribute

   /// @brief Collect descendants whose `class` attribute contains the token
   void find( std::string_view stringClassName, std::vector<element*>& vectorResult, tag_class );
   void find( std::string_view stringClassName, std::vector<const element*>& vectorResult, tag_class ) const;

   /** -------------------------------------------------------------------------- walk
    * @brief Visit every descendant in document order
    * @param callbackVisitor  Called with (element&); return false to stop traversal
    * @return bool  True if the full tree was visited, false if cut short
    */
   bool walk( const std::function<bool(const element&)>& callbac_ ) const;

// ## range helpers -----------------------------------------------------------
   /// Direct-child range — use in `for( auto& elementChild : elementParent )` --- begin / end
   iterator       begin()        { return iterator( m_vectorElement.begin() ); }
   iterator       end()          { return iterator( m_vectorElement.end() );   }
   const_iterator begin()  const { return const_iterator( m_vectorElement.cbegin() ); }
   const_iterator end()    const { return const_iterator( m_vectorElement.cend() );   }
   const_iterator cbegin() const { return const_iterator( m_vectorElement.cbegin() ); }
   const_iterator cend()   const { return const_iterator( m_vectorElement.cend() );   }

   /// Whole-subtree range — use in `for( auto& e : element.tree() )` ----------- tree_begin / tree_end
   tree_iterator       tree_begin()        { return tree_iterator( this ); }
   tree_iterator       tree_end()          { return tree_iterator();        }
   const_tree_iterator tree_begin()  const { return const_tree_iterator( this ); }
   const_tree_iterator tree_end()    const { return const_tree_iterator();        }
   const_tree_iterator tree_cbegin() const { return const_tree_iterator( this ); }
   const_tree_iterator tree_cend()   const { return const_tree_iterator();        }


// ## member variables --------------------------------------------------------
   std::string                             m_stringName;               ///< Tag name e.g. "div", "record"
   std::string                             m_stringContent;            ///< Accumulated text content
   element*                                m_pelementParent = nullptr; ///< Non-owning back-pointer
   std::vector<std::unique_ptr<element>>   m_vectorElement;            ///< Owned child nodes
   gd::argument::shared::arguments         m_argumentsAttribute;       ///< Attribute key→value store

public:
   /// Case-insensitive ASCII equality ---------------------------------------- equal_case_insensitive_s
   static bool equal_case_insensitive_s( std::string_view stringA, std::string_view stringB ) noexcept;

   /// True if `stringClassList` contains `stringToken` as a whole word ------- has_class_token_s
   static bool has_class_token_s( std::string_view stringClassList, std::string_view stringToken ) noexcept;

   /// Join a vector of element pointers into a string of their tag names ------- to_string_s
   static std::string to_string_s( const std::vector<element*>& vectorElements, std::string_view stringSplit = " / ");
};


// ============================================================================
// @CLASS [tag: document] [summary: Owns the root element and exposes query API]
// ============================================================================

/** -------------------------------------------------------------------------- document
 * @brief Container for the parsed HTML/XML tree structure
 * 
 * The `document` owns the synthetic root `element` which contains the entire 
 * parsed document tree. Provides convenience query methods that delegate to 
 * the root element for finding elements by tag name or ID. Move-only to 
 * ensure single ownership of the tree.
 * 
 * After parsing, call `is_valid()` to confirm the document has a valid root 
 * before querying. An invalid document indicates a parse failure or an empty 
 * source.
 * 
 * @code
// Sample HTML source ----------------------------------------------------
string stringHtml = R"(
   <html>
   <body>
      <div id="main" class="content highlight">Hello <span>World</span></div>
      <img src="logo.png" alt="logo" />
   </body>
   </html>
)";

// Parse into a document -------------------------------------------------
parser parserParser;
parserParser.set_mode( enumParseMode::eParseModeHtml );
document documentDocument = parserParser.parse( stringHtml );

if( documentDocument.is_valid() == false ) { std::cerr << "Parse failed\n"; return;}

// Root element and simple queries --------------------------------------
element* pelementRoot = documentDocument.root();                         // non-owning pointer to synthetic root
element* pelementDiv  = documentDocument.find( "div" );                  // first <div> in DFS order

if( pelementDiv != nullptr )
{
   if( pelementDiv->has_attribute( "id" ) )
   {
      std::string stringId = std::string( pelementDiv->get_attribute( "id" ) );
      std::cout << "Found div id='" << stringId << "' content='" << pelementDiv->content() << "'\n";
   }

   // Walk direct children of the div
   for( auto& elementChild : *pelementDiv )
   {
      std::cout << "  child tag: " << elementChild.name() << " content='" << elementChild.content() << "'\n";
   }

   // Append some text content
   pelementDiv->append_content( " -- appended text" );
   std::cout << "Div new content: '" << pelementDiv->content() << "'\n";
}

// Find by id using tag-dispatch overload --------------------------------
// Note: passing `element::tag_id{}` selects the id-based find overload
const element* pelementById = documentDocument.find( "main", element::tag_id{} );
if( pelementById != nullptr ) { std::cout << "find(id=main) -> " << pelementById->name() << "\n"; }

// Collect all <img> elements -------------------------------------------
std::vector<element*> vectorImg = documentDocument.find_all( "img" );
std::cout << "Found " << vectorImg.size() << " <img> elements\n";
for( element* pelementImg : vectorImg )
{
   if( pelementImg->has_attribute( "src" ) )
   {
      std::cout << " img src = " << pelementImg->get_attribute( "src" ) << "\n";
   }
}
 * @endcode
 * 
 */
struct document
{
   document()  = default;
   ~document() = default;

   document( document&& o ) noexcept : m_pelementRoot( std::move( o.m_pelementRoot ) ) {}
   document& operator=( document&& o ) noexcept
   {
      if( this != &o ) { m_pelementRoot = std::move( o.m_pelementRoot ); }
      return *this;
   }

   bool     is_valid() const noexcept { return m_pelementRoot != nullptr; }
   element* root()     const noexcept { return m_pelementRoot.get(); }

   // @API [tag: find] 

   element*       find( std::string_view stringTag );
   const element* find( std::string_view stringTag ) const;

   std::vector<element*>       find_all( std::string_view stringTag );
   std::vector<const element*> find_all( std::string_view stringTag ) const;

   element*       find( std::string_view stringIdValue, element::tag_id );
   const element* find( std::string_view stringIdValue, element::tag_id ) const;

// ## member variables --------------------------------------------------------
   std::unique_ptr<element> m_pelementRoot;  ///< Synthetic root that owns the tree
};


// ============================================================================
// @CLASS [tag: parse_mode] [summary: Controls parser strictness and void-element behaviour]
//
// eParseModeHtml — HTML5 mode. The built-in void-element table is consulted so
//                 `<br>`, `<img>`, `<input>` etc. never push onto the open-
//                 element stack. Lenient close-tag matching is always active.
//
// eParseModeXmlLenient — For any XML dialect where end-tags may be absent. The void-
//                 element table is entirely ignored; every element without an
//                 explicit `/>` or matching close-tag remains open and becomes
//                 the parent of whatever follows. EOF closes all open elements
//                 silently, leaving a valid tree.
//
// eParseModeXmlStrict — Reserved for future use. Currently behaves like
//                 eParseModeXmlLenient. A later version will populate a diagnostic
//                 list for mismatched or missing close-tags.
// ============================================================================
enum class enumParseMode : uint8_t
{
   eParseModeHtml       = 0,  ///< HTML5: void elements self-close, lenient close-tag matching
   eParseModeXmlLenient = 1,  ///< XML: end-tags may be absent — void-element table is skipped
   eParseModeXmlStrict  = 2,  ///< Well-formed XML — reserved; today behaves like e_xml_lenient
   eParseModeError      = 3   ///< Invalid mode value; reserved for diagnostics
};


/** -------------------------------------------------------------------------- parser
 * Implements a single-pass tokeniser for HTML/XML source text, building a
 * document tree of `element` nodes. The parsing mode controls how tags are
 * treated and how strict the parser is about mismatched or missing close-tags:
 *
 * @NOTE Tag names are interned for the ~80 most common HTML tags to avoid
 *       a heap allocation per node.
 */
class parser
{
public:
   parser() { m_vectorElementStack.reserve( 64 ); }

   /// Set the parsing mode before calling parse() ---------------------------- set_mode
   void set_mode( enumParseMode eMode ) noexcept { m_eParseMode = eMode; }

   /// Return the currently active mode --------------------------------------- mode
   enumParseMode mode() const noexcept { return m_eParseMode; }

   /** -------------------------------------------------------------------------- parse
    * @brief Parse raw HTML/XML text and return a populated document
    * @param stringSource  Full source text (must remain valid for the duration of this call)
    * @return document     Populated tree; check is_valid() before use
    */
   document parse( std::string_view stringSource, std::pair<bool, std::string>* ppairError = nullptr );


// ## helpers – position / character access -----------------------------------

   /// True when the cursor has reached the end of the source ----------------- at_end
   bool at_end() const noexcept { return m_uPosition >= m_stringSource.size(); }
   bool is_eof() const noexcept { return at_end(); }
   bool is_error() const noexcept { return m_eParseMode == enumParseMode::eParseModeError; }
   char offset( int iOffset = 0 ) const noexcept;

   /// Current character — call at_end() first -------------------------------- current_char
   char current_char() const noexcept { assert( at_end() == false ); return m_stringSource[m_uPosition]; }

   /// Peek up to `uLength` characters ahead without advancing --------------- peek
   std::string_view peek( size_t uLength ) const noexcept
   {
      return m_stringSource.substr( m_uPosition, std::min( uLength, m_stringSource.size() - m_uPosition ) );
   }

   void             skip_whitespace() noexcept;
   void             skip_until( char iStop ) noexcept;
   std::string_view read_name_view() noexcept;    ///< Returns a view into m_stringSource — zero copy
   std::string      read_attribute_value();

// ## tag dispatch ------------------------------------------------------------
   bool parse_tag();
   void parse_opening_tag();
   void parse_closing_tag();
   void parse_comment();
   void parse_declaration();
   void parse_processing_instruction();
   void parse_text();
   void parse_attributes( element& elementTarget );

// ## state management for error handling and backtracking --------------------------------

   void error_prepare() noexcept { m_uPositionError = m_uPosition; }
   void error_set( std::string_view stringMessage );

private:
// ## members ------------------------------------------------------------
   std::string_view      m_stringSource;                                   ///< View into the source being parsed
   size_t                m_uPosition       = 0;                            ///< Current read cursor
   size_t                m_uPositionError  = 0;                            ///< if error occurs, position of the error in the source text
   element*              m_pelementCurrent = nullptr;                      ///< Node currently being populated
   std::vector<element*> m_vectorElementStack;                             ///< Open-element stack (non-owning views)
   enumParseMode         m_eParseMode      = enumParseMode::eParseModeHtml;///< Active parsing mode
   std::string           m_stringError;                                    ///< Last error message; reserved for future diagnostics

// ## utilities ---------------------------------------------------------------

   /// Intern common tag names to avoid per-element heap allocations ---------- intern_tag_s
   static std::string_view intern_tag_s( std::string_view stringRaw ) noexcept;

   /// True for HTML void elements — only called when mode is e_html ---------- is_void_element_s
   static bool is_void_element_s( std::string_view stringTag ) noexcept;

// ## static tables -----------------------------------------------------------

   // @NOTE Both arrays must remain sorted — lower_bound depends on it
   static constexpr std::array<std::string_view, 14> m_arrayVoidElements = { {
      "area","base","br","col","embed","hr","img","input", "link","meta","param","source","track","wbr"
   } };

   static const std::array<std::string_view, 94>& interned_tag_table() noexcept;
};

/// Get the character at the current position plus `iOffset` without advancing. Returns '\0' if the peek position is out of bounds (before start or after end).
inline char parser::offset( int iOffset ) const noexcept
{                                                                                                  assert( ((int64_t)m_uPosition + iOffset) >= 0 );
   size_t uPeekPosition = m_uPosition + iOffset;
   return uPeekPosition < m_stringSource.size() ? m_stringSource[uPeekPosition] : '\0'; // Note that if peek position goes before start the value will be huge and peek will return '\0' 
}


/// Set the parser into error mode, preventing further parsing and rewinding to the position where the error was detected. The provided message is stored for diagnostics.
inline void parser::error_set( std::string_view stringMessage ) 
{
   m_eParseMode = enumParseMode::eParseModeError;                          // switch to error mode to prevent further parsing
   m_uPosition = m_uPositionError;                                         // rewind to the position where the error was detected
   if( m_stringError.empty() == false ) m_stringError += ", ";
   m_stringError += stringMessage;                                         // store the error message for diagnostics
}


_GD_TOOLS_HTML_END