// @FILE [tag: html,dom,parser] [description: HTML/XML document model and fast single-pass parser]
//       [type: source] [name: gd_tools_html_document.cpp]

#include "gd_tools_html_document.h"

_GD_TOOLS_HTML_BEGIN

// ============================================================================
// element
// ============================================================================

void element::common_construct( const element& o )
{
   m_stringName         = o.m_stringName;
   m_stringContent      = o.m_stringContent;
   m_pelementParent     = nullptr;                                          // parent is set by the new owner
   m_argumentsAttribute = o.m_argumentsAttribute;

   m_vectorElement.reserve( o.m_vectorElement.size() );
   for( const auto& pelementChild : o.m_vectorElement )
   {
      auto pelement_ = std::make_unique<element>();
      pelement_->common_construct( *pelementChild );
      pelement_->set_parent( this );
      m_vectorElement.push_back( std::move( pelement_ ) );
   }
}

/**  -------------------------------------------------------------------------- append_content
 * @brief Append a text fragment to the element's content
 *        A single space is inserted between consecutive runs.
 * @param stringText  Text fragment to append (ignored if empty)
 */
void element::append_content( std::string_view stringText )
{
   if( stringText.empty() ) { return; }
   if( !m_stringContent.empty() ) { m_stringContent += ' '; }
   m_stringContent.append( stringText );
}

/**  -------------------------------------------------------------------------- add
 * @brief Transfer ownership of a child element into this node
 * @param pelementChild  Child to adopt (must not be nullptr)
 * @return element*      Raw pointer to the newly added child
 */
element* element::add( std::unique_ptr<element> pelementChild )
{
   assert( pelementChild != nullptr );
   pelementChild->set_parent( this );
   m_vectorElement.push_back( std::move( pelementChild ) );
   return m_vectorElement.back().get();
}

/**  -------------------------------------------------------------------------- add_attribute
 * @brief Store a name→value attribute pair on this element
 * @param stringName     Attribute name
 * @param stringContent  Attribute value
 */
void element::add_attribute( std::string_view stringName, std::string_view stringContent )
{
   m_argumentsAttribute[stringName] = stringContent;
}

/// Count self + all descendants ---------------------------------------------
size_t element::size_all() const noexcept
{
   size_t uCount = 1; // Count self
   for( const auto& pChild : m_vectorElement ) { uCount += pChild->size_all(); }
   return uCount;
}

// ## traversal ---------------------------------------------------------------

/**  -------------------------------------------------------------------------- find
 * @brief Depth-first search for the first descendant matching `stringTag`
 * @param stringTag  Tag name (case-insensitive)
 * @return element*  First match or nullptr
 */
element* element::find( std::string_view stringTag )
{
   for( auto& pChild : m_vectorElement )
   {
      if( equal_case_insensitive_s( pChild->m_stringName, stringTag ) ) { return pChild.get(); }
      if( auto* pFound = pChild->find( stringTag ); pFound != nullptr ) { return pFound; }
   }
   return nullptr;
}

const element* element::find( std::string_view stringTag ) const
{
   return const_cast<element*>( this )->find( stringTag );
}

/**  -------------------------------------------------------------------------- find_all
 * @brief Collect every descendant whose tag name matches `stringTag`
 * @param stringTag     Tag name (case-insensitive)
 * @param vectorResult  Accumulator — results are appended
 */
void element::find_all( std::string_view stringTag, std::vector<element*>& vectorResult )
{
   for( auto& pChild : m_vectorElement )
   {
      if( equal_case_insensitive_s( pChild->m_stringName, stringTag ) ) { vectorResult.push_back( pChild.get() ); }
      pChild->find_all( stringTag, vectorResult );
   }
}

void element::find_all( std::string_view stringTag, std::vector<const element*>& vectorResult ) const
{
   for( const auto& pChild : m_vectorElement )
   {
      if( equal_case_insensitive_s( pChild->m_stringName, stringTag ) ) { vectorResult.push_back( pChild.get() ); }
      pChild->find_all( stringTag, vectorResult );
   }
}

/**  -------------------------------------------------------------------------- find_by_id
 * @brief First descendant (or self) where attribute `id` equals `stringIdValue`
 * @param stringIdValue  Exact id value (case-sensitive)
 * @return element*      First match or nullptr
 */
element* element::find_by_id( std::string_view stringIdValue )
{
   if( has_attribute( "id" ) && get_attribute( "id" ) == stringIdValue ) { return this; }
   for( auto& pChild : m_vectorElement )
   {
      if( auto* pFound = pChild->find_by_id( stringIdValue ); pFound != nullptr ) { return pFound; }
   }
   return nullptr;
}

const element* element::find_by_id( std::string_view stringIdValue ) const
{
   return const_cast<element*>( this )->find_by_id( stringIdValue );
}

/**  -------------------------------------------------------------------------- find_by_class
 * @brief Collect descendants whose `class` attribute contains `stringClassName`
 * @param stringClassName  Single class token
 * @param vectorResult     Accumulator
 */
void element::find_by_class( std::string_view stringClassName, std::vector<element*>& vectorResult )
{
   for( auto& pelement : m_vectorElement )
   {
      if( has_class_token_s( pelement->get_attribute( "class" ), stringClassName ) ) { vectorResult.push_back( pelement.get() ); }
      pelement->find_by_class( stringClassName, vectorResult );
   }
}

void element::find_by_class( std::string_view stringClassName, std::vector<const element*>& vectorResult ) const
{
   for( const auto& pelement : m_vectorElement )
   {
      if( has_class_token_s( pelement->get_attribute( "class" ), stringClassName ) )
      {
         vectorResult.push_back( pelement.get() );
      }
      pelement->find_by_class( stringClassName, vectorResult );
   }
}

/**  -------------------------------------------------------------------------- walk
 * @brief Visit every descendant in document order
 * @param callbackVisitor  Callable(element&) → bool; returning false stops early
 * @return bool  True if the entire tree was visited
 */
 /*
bool element::walk( const std::function<bool(element&)>& callbackVisitor )
{
   for( auto& pChild : m_vectorElement )
   {
      if( !callbackVisitor( *pChild ) )           { return false; }
      if( !pChild->walk( callbackVisitor ) )       { return false; }
   }
   return true;
}
*/

/**  -------------------------------------------------------------------------- walk
 * @brief Visit every descendant in document order
 * @param callback_  Callable(element&) → bool; returning false stops early
 * @return bool True if the entire tree was visited
 */
bool element::walk( const std::function<bool(const element&)>& callback_ ) const
{
   for( const auto& pelement : m_vectorElement )
   {
      if( callback_( *pelement ) == false )           { return false; }
      if( pelement->walk( callback_ ) == false )      { return false; }
   }
   return true;
}

// ## private helpers ---------------------------------------------------------

/**  -------------------------------------------------------------------------- equal_case_insensitive_s
 * @brief Case-insensitive string equality check
 * @param stringA   First string
 * @param stringB   Second string
 * @return bool     True if strings are equal ignoring case
 */
bool element::equal_case_insensitive_s( std::string_view stringA, std::string_view stringB ) noexcept
{
   if( stringA.size() != stringB.size() ) { return false; }
   for( size_t u = 0; u < stringA.size(); ++u )
   {
      if( std::tolower( (unsigned char)stringA[u] ) != std::tolower( (unsigned char)stringB[u] ) ) { return false; }
   }
   return true;
}

/**  -------------------------------------------------------------------------- has_class_token_s
 * @brief Check if a class list string contains a specific class token (token word must match exactly, not just as a substring)
 * 
 * @code
 * // Sample usage of has_class_token_s to check if an element has a specific class token
 * std::string stringClassList = "class1 class2 class3";
 * std::string stringToken = "class2";
 * bool hasToken = element::has_class_token_s( stringClassList, stringToken );
 * @endcode
 * 
 * @param stringClassList  Space-separated list of class names
 * @param stringToken      Class token to search for
 * @return bool            True if the class token is found
 */
bool element::has_class_token_s( std::string_view stringClassList, std::string_view stringToken ) noexcept
{
   size_t uPosition = 0;
   while( uPosition < stringClassList.size() )
   {
      while( uPosition < stringClassList.size() && std::isspace( (unsigned char)stringClassList[uPosition] ) ) { ++uPosition; } // skip leading whitespace

      size_t uTokenStart = uPosition;
      while( uPosition < stringClassList.size() && false == std::isspace( (unsigned char)stringClassList[uPosition] ) )
      {
         ++uPosition;
      }

#ifndef NDEBUG
      [[maybe_unused]] std::string_view stringCurrentToken_d = stringClassList.substr( uTokenStart, uPosition - uTokenStart );
#endif // NDEBUG
      
      if( stringClassList.substr( uTokenStart, uPosition - uTokenStart ) == stringToken ) { return true; }
   }
   return false;
}


// ============================================================================
// document
// ============================================================================

element* document::find( std::string_view stringTag )
{
   if( !m_pelementRoot ) { return nullptr; }
   return m_pelementRoot->find( stringTag );
}

const element* document::find( std::string_view stringTag ) const
{
   return const_cast<document*>( this )->find( stringTag );
}

/// Collect every descendant whose tag name matches `stringTag` (case-insensitive)
std::vector<element*> document::find_all( std::string_view stringTag )
{
   std::vector<element*> vectorResult;
   if( m_pelementRoot ) { m_pelementRoot->find_all( stringTag, vectorResult ); }
   return vectorResult;
}

std::vector<const element*> document::find_all( std::string_view stringTag ) const
{
   std::vector<const element*> vectorResult;
   if( m_pelementRoot ) { m_pelementRoot->find_all( stringTag, vectorResult ); }
   return vectorResult;
}

element* document::find_by_id( std::string_view stringIdValue )
{
   if( !m_pelementRoot ) { return nullptr; }
   return m_pelementRoot->find_by_id( stringIdValue );
}

const element* document::find_by_id( std::string_view stringIdValue ) const
{
   return const_cast<document*>( this )->find_by_id( stringIdValue );
}


// ============================================================================
// parser – static tables
// ============================================================================

// @NOTE Sorted alphabetically — lower_bound depends on this ordering
const std::array<std::string_view, 94>& parser::interned_tag_table() noexcept
{
   static constexpr std::array<std::string_view, 94> arrayInternedTags_s = { {
      "a","abbr","address","article","aside","audio",
      "b","blockquote","body","br","button",
      "canvas","caption","cite","code","col","colgroup",
      "data","datalist","dd","del","details","dfn","dialog","div","dl","dt",
      "em","embed",
      "fieldset","figcaption","figure","footer","form",
      "h1","h2","h3","h4","h5","h6","head","header","hr","html",
      "i","iframe","img","input","ins",
      "kbd",
      "label","legend","li","link",
      "main","map","mark","menu","meta",
      "nav",
      "ol","option","output",
      "p","picture","pre","progress",
      "q",
      "s","script","section","select","small","source","span","strong","style","sub","summary","sup",
      "table","tbody","td","textarea","tfoot","th","thead","time","title","tr",
      "u","ul",
      "video",
      "wbr"
   } };
   return arrayInternedTags_s;
}


// ============================================================================
// parser – implementation
// ============================================================================

/**  -------------------------------------------------------------------------- parse
 * @brief Parse full HTML/XML source text into a document tree
 *
 * A synthetic `__root__` element owns all top-level nodes so multi-root
 * documents are handled correctly. The open-element stack is reset at the
 * start of each call so the same parser instance can be reused.
 *
 * @param stringSource  View of the complete source text
 * @return document     Populated document; call is_valid() before use
 */
document parser::parse( std::string_view stringSource )
{
   m_stringSource    = stringSource;
   m_uPosition       = 0;
   m_vectorElementStack.clear();

   document documentResult;
   auto pelementRoot_ = std::make_unique<element>( "__root__" );
   pelementRoot_->reserve( 8 );

   m_pelementCurrent = pelementRoot_.get();
   m_vectorElementStack.push_back( m_pelementCurrent );

   while( !at_end() )
   {
      skip_whitespace();
      if( at_end() ) { break; }

      if( current_char() == '<' ) { parse_tag();  }
      else                        { parse_text(); }
   }

   documentResult.m_pelementRoot = std::move( pelementRoot_ );
   return documentResult;
}

// ## private – position helpers ----------------------------------------------

void parser::skip_whitespace() noexcept
{
   while( !at_end() && std::isspace( (unsigned char)current_char() ) ) { ++m_uPosition; }
}

void parser::skip_until( char iStop ) noexcept
{
   while( !at_end() && current_char() != iStop ) { ++m_uPosition; }
   if( !at_end() ) { ++m_uPosition; }                                     // consume the stop char
}

/// Returns a view directly into the source buffer — zero heap allocation
std::string_view parser::read_name_view() noexcept
{
   size_t uStart = m_uPosition;
   while( !at_end() )
   {
      char iChar = current_char();
      if( std::isalnum( (unsigned char)iChar ) ||
          iChar == '_' || iChar == '-' || iChar == ':' || iChar == '.' )
      {
         ++m_uPosition;
      }
      else { break; }
   }
   return m_stringSource.substr( uStart, m_uPosition - uStart );
}

std::string parser::read_attribute_value()
{
   if( at_end() ) { return {}; }

   char iQuote = current_char();
   if( iQuote == '"' || iQuote == '\'' )
   {
      ++m_uPosition;                                                        // consume open quote
      size_t uStart = m_uPosition;
      while( !at_end() && current_char() != iQuote ) { ++m_uPosition; }
      std::string stringValue( m_stringSource.substr( uStart, m_uPosition - uStart ) );
      if( !at_end() ) { ++m_uPosition; }                                   // consume close quote
      return stringValue;
   }

   // Unquoted value — stop at whitespace or '>'
   size_t uStart = m_uPosition;
   while( !at_end() )
   {
      char iChar = current_char();
      if( std::isspace( (unsigned char)iChar ) || iChar == '>' || iChar == '/' ) { break; }
      ++m_uPosition;
   }
   return std::string( m_stringSource.substr( uStart, m_uPosition - uStart ) );
}

// ## private – tag dispatch --------------------------------------------------

/**  -------------------------------------------------------------------------- parse_tag
 * @brief Dispatch to the appropriate tag handler based on the syntax following '<'
 *
 * Handles opening tags, closing tags, comments, declarations, and processing
 * instructions. If the syntax is unrecognized, the tag is skipped.
 */
void parser::parse_tag()
{                                                                                                  assert( !at_end() && current_char() == '<' );
   ++m_uPosition;                                                           // consume '<'
   if( at_end() == true ) { return; }

   if( current_char() == '/' )      { parse_closing_tag();            }
   else if( peek(3) == "!--" )      { parse_comment();                }
   else if( current_char() == '!' ) { parse_declaration();            }
   else if( current_char() == '?' ) { parse_processing_instruction(); }
   else                             { parse_opening_tag();            }
}

/**  -------------------------------------------------------------------------- parse_opening_tag
 * @brief Parse `tagname attr="val" … >` or self-closing `… />`
 *
 * Whether an element without `/>` is treated as self-closing depends on
 * `m_eParseMode`:
 *   - e_html        → void-element table is consulted; matching tags never push
 *   - e_xml_lenient → void-element table is skipped; every element can have children
 *   - e_xml_strict  → same as e_xml_lenient today (future: emit diagnostics)
 *
 * @NOTE Tag names are interned for the ~80 most common HTML tags to avoid
 *       a heap allocation per node.
 */
void parser::parse_opening_tag()
{
   std::string_view stringTagRaw = read_name_view();
   if( stringTagRaw.empty() ) { skip_until( '>' ); return; }

   // Intern the tag name — avoids a heap allocation for the vast majority of HTML nodes
   std::string_view stringInterned = intern_tag_s( stringTagRaw );
   auto pelementNew_ = std::make_unique<element>( stringInterned.empty() ? std::string( stringTagRaw ) : std::string( stringInterned ) );
   element* pelementNewRaw = pelementNew_.get();

   parse_attributes( *pelementNew_ );
   skip_whitespace();

   bool bIsSelfClosing = false;
   if( !at_end() && current_char() == '/' )
   {
      bIsSelfClosing = true;
      ++m_uPosition;                                                        // consume '/'
   }
   if( !at_end() && current_char() == '>' ) { ++m_uPosition; }            // consume '>'

   m_pelementCurrent->add( std::move( pelementNew_ ) );

   // @NOTE In e_html mode the void-element table prevents pushing known self-closing
   //       tags (br, img, input …) onto the stack.
   //       In XML modes the table is ignored — every element without `/>` can have
   //       children, which is the "forgiving" behaviour requested.
   bool bTreatAsVoid = bIsSelfClosing || ( m_eParseMode == enumParseMode::eParseModeHtml && is_void_element_s( stringTagRaw ) );

   if( bTreatAsVoid == false )
   {
      m_pelementCurrent = pelementNewRaw;
      m_vectorElementStack.push_back( pelementNewRaw );
   }
}

/**  -------------------------------------------------------------------------- parse_closing_tag
 * @brief Consume `/ tagname >` and pop the element stack
 *
 * Walks backward through the open stack to find the nearest ancestor whose
 * tag matches. Elements that were never closed are simply left as valid
 * children of their parent — no crash, no data loss.
 */
void parser::parse_closing_tag()
{
   assert( !at_end() && current_char() == '/' );
   ++m_uPosition;                                                           // consume '/'
   skip_whitespace();

   std::string_view stringClosingTag = read_name_view();
   skip_until( '>' );

   for( int i = (int)m_vectorElementStack.size() - 1; i >= 0; --i )          // walk backward through the stack to find a matching open tag
   {
      if( equal_case_insensitive_s( m_vectorElementStack[(size_t)i]->name(), stringClosingTag ) )
      {
         m_vectorElementStack.resize( (size_t)i );
         m_pelementCurrent = m_vectorElementStack.empty() ? nullptr : m_vectorElementStack.back();
         return;
      }
   }
   // @NOTE Orphan close-tag — silently ignored (common in HTML and lenient XML)
}

void parser::parse_comment()
{
   m_uPosition += 3;                                                        // consume "!--"
   while( m_uPosition + 2 < m_stringSource.size() )
   {
      if( m_stringSource.substr( m_uPosition, 3 ) == "-->" ) { m_uPosition += 3; return; }
      ++m_uPosition;
   }
}

/**  -------------------------------------------------------------------------- parse_declaration
 * @brief Skip `<! … >` declarations (e.g. `<!DOCTYPE html>`)
 */
void parser::parse_declaration()
{
   skip_until( '>' );                                                       // skip <!DOCTYPE …> and similar
}

/**  -------------------------------------------------------------------------- parse_processing_instruction
 * @brief Skip `<? … ?>` processing instructions (XML) or similar constructs
 */
void parser::parse_processing_instruction()
{
   while( m_uPosition + 1 < m_stringSource.size() )
   {
      if( m_stringSource.substr( m_uPosition, 2 ) == "?>" ) { m_uPosition += 2; return; }
      ++m_uPosition;
   }
}

/**  -------------------------------------------------------------------------- parse_text
 * @brief Consume text until the next '<' and append it to the current element's content
 *
 * Leading and trailing whitespace is trimmed from the text fragment before
 * appending. If the resulting string is empty, it is ignored.
 */
void parser::parse_text()
{
   size_t uStart = m_uPosition;
   while( !at_end() && current_char() != '<' ) { ++m_uPosition; }             // stop at next tag

   std::string_view stringRaw = m_stringSource.substr( uStart, m_uPosition - uStart ); // raw text including any whitespace

   // ## Trim without allocating — find bounds inside the raw view
   size_t uFirst = 0;
   while( uFirst < stringRaw.size() && std::isspace( (unsigned char)stringRaw[uFirst] ) ) { ++uFirst; }
   size_t uLast = stringRaw.size();
   while( uLast > uFirst && std::isspace( (unsigned char)stringRaw[uLast - 1] ) ) { --uLast; }

   std::string_view stringTrimmed = stringRaw.substr( uFirst, uLast - uFirst );
   if( !stringTrimmed.empty() && m_pelementCurrent != nullptr )
   {
      m_pelementCurrent->append_content( stringTrimmed );
   }
}

/**  -------------------------------------------------------------------------- parse_attributes
 * @brief Read zero or more attribute pairs and store them on `elementTarget`
 * @param elementTarget  Element to populate with the parsed attributes
 */
void parser::parse_attributes( element& elementTarget )
{
   while( !at_end() )
   {
      skip_whitespace();
      if( at_end() ) { break; }

      char iChar = current_char();
      if( iChar == '>' || iChar == '/' ) { break; }

      std::string_view stringAttrName = read_name_view();
      if( stringAttrName.empty() ) { ++m_uPosition; continue; }            // skip unrecognised char

      skip_whitespace();
      std::string stringAttrValue;

      if( !at_end() && current_char() == '=' )
      {
         ++m_uPosition;                                                     // consume '='
         skip_whitespace();
         stringAttrValue = read_attribute_value();
      }

      elementTarget.add_attribute( stringAttrName, stringAttrValue );
   }
}

// ## private – static utilities ----------------------------------------------

std::string_view parser::intern_tag_s( std::string_view stringRaw ) noexcept
{
   const auto& arrayTable = interned_tag_table();

   // Lower-case the raw name on the stack — tag names are always short
   char   arrayLower[64] = {};
   size_t uLength = std::min( stringRaw.size(), sizeof(arrayLower) - 1 );
   for( size_t u = 0; u < uLength; ++u )
   {
      arrayLower[u] = (char)std::tolower( (unsigned char)stringRaw[u] );
   }
   std::string_view stringLower( arrayLower, uLength );

   auto it = std::lower_bound( arrayTable.begin(), arrayTable.end(), stringLower );
   if( ;it != arrayTable.end() && *it == stringLower ) { return *it; }
   return {};                                                               // unknown tag — caller allocates
}

bool parser::is_void_element_s( std::string_view stringTag ) noexcept
{
   // @NOTE Only called in e_html mode — see parse_opening_tag
   char   arrayLower[32] = {};
   size_t uLen = std::min( stringTag.size(), sizeof(arrayLower) - 1 );
   for( size_t u = 0; u < uLen; ++u )
   {
      arrayLower[u] = (char)std::tolower( (unsigned char)stringTag[u] );
   }
   std::string_view stringLower( arrayLower, uLen );

   auto it = std::lower_bound( m_arrayVoidElements.begin(), m_arrayVoidElements.end(), stringLower );
   return ( it != m_arrayVoidElements.end() && *it == stringLower );
}

bool parser::equal_case_insensitive_s( std::string_view stringA, std::string_view stringB ) noexcept
{
   if( stringA.size() != stringB.size() ) { return false; }
   for( size_t u = 0; u < stringA.size(); ++u )
   {
      if( std::tolower( (unsigned char)stringA[u] ) != std::tolower( (unsigned char)stringB[u] ) )
      {
         return false;
      }
   }
   return true;
}

_GD_TOOLS_HTML_END