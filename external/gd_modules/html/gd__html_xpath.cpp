// @FILE [tag: html,dom,xpath] [description: Basic XPath 1.0 subset evaluator for the html::element tree]
//       [type: source] [name: gd_tools_html_xpath.cpp]

#include "gd_tools_html_xpath.h"

#include <cctype>

_GD_MODULES_HTML_BEGIN

// ============================================================================
// xpath  —  public API
// ============================================================================

/**  -------------------------------------------------------------------------- evaluate
 * @brief  Evaluate XPath expression and return the first matching element
 * @param  elementRoot       Root of the subtree to search
 * @param  stringExpression  XPath expression
 * @return element*          First match in document order, or nullptr
 */
element* xpath::evaluate( element& elementRoot, std::string_view stringExpression )
{
   auto vectorResult = evaluate_all( elementRoot, stringExpression );
   return vectorResult.empty() ? nullptr : vectorResult.front();
}

const element* xpath::evaluate( const element& elementRoot, std::string_view stringExpression )
{
   auto vectorResult = evaluate_all( elementRoot, stringExpression );
   return vectorResult.empty() ? nullptr : vectorResult.front();
}

/**  -------------------------------------------------------------------------- evaluate_all
 * @brief  Evaluate XPath expression and return every matching element
 *
 * The algorithm maintains a *context set* — the set of nodes that survive
 * each step.  For each `xpath_step`:
 *   - if `m_bIsDescendant` is true  → expand each context node into its full subtree
 *   - otherwise                     → expand each context node into its direct children
 * In both cases only nodes that pass the step's tag-test and predicates are kept.
 *
 * @param  elementRoot       Root of the subtree to search
 * @param  stringExpression  XPath expression
 * @return vector<element*>  All matches in document order
 */
std::vector<element*> xpath::evaluate_all( element& elementRoot, std::string_view stringExpression )
{
   std::vector<xpath_step> vectorStep = parse_expression_s( stringExpression );
   if( vectorStep.empty() ) { return {}; }

   std::vector<element*> vectorContext = { &elementRoot };              // start: single root node

   for( const xpath_step& xpathstepCurrent : vectorStep )
   {
      std::vector<element*> vectorNextContext;

      for( element* pelementContext : vectorContext )
      {
         if( xpathstepCurrent.m_bIsDescendant )                         // // axis — search entire subtree
         {
            collect_matching_descendants_s( *pelementContext, xpathstepCurrent, vectorNextContext );
         }
         else                                                            // / axis — direct children only
         {
            collect_matching_children_s( *pelementContext, xpathstepCurrent, vectorNextContext );
         }
      }

      vectorContext = std::move( vectorNextContext );
   }

   return vectorContext;
}

std::vector<const element*> xpath::evaluate_all( const element& elementRoot, std::string_view stringExpression )
{
   std::vector<xpath_step> vectorStep = parse_expression_s( stringExpression );
   if( vectorStep.empty() ) { return {}; }

   std::vector<const element*> vectorContext = { &elementRoot };

   for( const xpath_step& xpathstepCurrent : vectorStep )
   {
      std::vector<const element*> vectorNextContext;

      for( const element* pelementContext : vectorContext )
      {
         if( xpathstepCurrent.m_bIsDescendant )
         {
            collect_matching_descendants_s( *pelementContext, xpathstepCurrent, vectorNextContext );
         }
         else
         {
            collect_matching_children_s( *pelementContext, xpathstepCurrent, vectorNextContext );
         }
      }

      vectorContext = std::move( vectorNextContext );
   }

   return vectorContext;
}


// ============================================================================
// xpath  —  parsing
// ============================================================================

/**  -------------------------------------------------------------------------- parse_expression_s
 * @brief  Tokenise an XPath expression into an ordered vector of `xpath_step`
 *
 * Grammar handled (informal):
 *   expression  ::= ( '/' '/'? )? step ( '/' '/'? step )*
 *   step        ::= name_or_wildcard predicate*
 *   predicate   ::= '[' … ']'
 *
 * The `m_bIsDescendant` flag on each step is set when `//` preceded that step,
 * reflecting the descendant-or-self axis.
 *
 * @param  stringExpression  Raw XPath string
 * @return vector<xpath_step>  Parsed steps; empty if the expression is blank
 */
std::vector<xpath_step> xpath::parse_expression_s( std::string_view stringExpression )
{
   std::vector<xpath_step> vectorStep;
   size_t uPosition         = 0;
   bool   bNextIsDescendant = false;

   // ## consume the leading separator(s) — /  or  //
   if( uPosition < stringExpression.size() && stringExpression[uPosition] == '/' )
   {
      ++uPosition;                                                       // consume first '/'
      if( uPosition < stringExpression.size() && stringExpression[uPosition] == '/' )
      {
         bNextIsDescendant = true;
         ++uPosition;                                                    // consume second '/'
      }
   }

   while( uPosition < stringExpression.size() )
   {
      // ## read tag name — stop at '[', '/', or end
      size_t uNameStart = uPosition;
      while( uPosition < stringExpression.size()
             && stringExpression[uPosition] != '['
             && stringExpression[uPosition] != '/' )
      {
         ++uPosition;
      }

      xpath_step xpathstepNew;
      xpathstepNew.m_bIsDescendant = bNextIsDescendant;
      xpathstepNew.m_stringTagName  = std::string( stringExpression.substr( uNameStart, uPosition - uNameStart ) );
      bNextIsDescendant = false;                                         // consumed — reset for next step

      // ## read zero or more predicate clauses  [...]
      while( uPosition < stringExpression.size() && stringExpression[uPosition] == '[' )
      {
         ++uPosition;                                                    // consume '['

         // find matching ']' respecting nested brackets
         size_t uPredicateStart = uPosition;
         int    iDepth          = 1;
         while( uPosition < stringExpression.size() && iDepth > 0 )
         {
            if(      stringExpression[uPosition] == '[' ) { ++iDepth; }
            else if( stringExpression[uPosition] == ']' ) { --iDepth; }
            ++uPosition;
         }
         // uPosition now points one past the closing ']'
         std::string_view stringPredicateContent = stringExpression.substr( uPredicateStart, uPosition - uPredicateStart - 1 );
         xpathstepNew.m_vectorPredicate.push_back( parse_predicate_s( stringPredicateContent ) );
      }

      vectorStep.push_back( std::move( xpathstepNew ) );

      // ## advance past the separator following this step
      if( uPosition < stringExpression.size() && stringExpression[uPosition] == '/' )
      {
         ++uPosition;                                                    // consume '/'
         if( uPosition < stringExpression.size() && stringExpression[uPosition] == '/' )
         {
            bNextIsDescendant = true;
            ++uPosition;                                                 // consume second '/'
         }
      }
   }

   return vectorStep;
}

/**  -------------------------------------------------------------------------- parse_predicate_s
 * @brief  Parse the text content inside `[…]` into an `xpath_predicate`
 *
 * Recognised forms (leading and trailing whitespace is stripped first):
 *   `last()`             → ePredicateTypeLast
 *   `<digits>`           → ePredicateTypePosition,  m_iPosition = parsed value
 *   `@name`              → ePredicateTypeAttribute, existence check only
 *   `@name = "value"`    → ePredicateTypeAttribute, value check (double or single quotes)
 *   `@name = 'value'`    → same
 *
 * @param  stringPredicateContent  Text between the brackets, without the brackets
 * @return xpath_predicate         Populated descriptor
 */
xpath_predicate xpath::parse_predicate_s( std::string_view stringPredicateContent )
{
   xpath_predicate xpathpredicateResult;

   // ## trim surrounding whitespace
   size_t uStart = 0;
   size_t uEnd   = stringPredicateContent.size();
   while( uStart < uEnd && std::isspace( (unsigned char)stringPredicateContent[uStart] ) ) { ++uStart; }
   while( uEnd   > uStart && std::isspace( (unsigned char)stringPredicateContent[uEnd - 1] ) ) { --uEnd; }
   stringPredicateContent = stringPredicateContent.substr( uStart, uEnd - uStart );

   if( stringPredicateContent.empty() ) { return xpathpredicateResult; }

   // ## last()
   if( stringPredicateContent == "last()" )
   {
      xpathpredicateResult.m_eType = xpath_predicate::enumPredicateType::ePredicateTypeLast;
      return xpathpredicateResult;
   }

   // ## pure integer → positional predicate
   bool bIsInteger = !stringPredicateContent.empty();
   for( char iChar : stringPredicateContent ) { if( !std::isdigit( (unsigned char)iChar ) ) { bIsInteger = false; break; } }
   if( bIsInteger )
   {
      xpathpredicateResult.m_eType = xpath_predicate::enumPredicateType::ePredicateTypePosition;
      int iValue = 0;
      for( char iChar : stringPredicateContent ) { iValue = iValue * 10 + ( iChar - '0' ); }
      xpathpredicateResult.m_iPosition = iValue;
      return xpathpredicateResult;
   }

   // ## attribute predicate — must start with '@'
   if( stringPredicateContent[0] != '@' ) { return xpathpredicateResult; } // unrecognised form — return default

   xpathpredicateResult.m_eType = xpath_predicate::enumPredicateType::ePredicateTypeAttribute;

   size_t uEqualsPosition = stringPredicateContent.find( '=' );
   if( uEqualsPosition == std::string_view::npos )
   {
      // ## existence-only: @attrname (no '=')
      std::string_view stringAttrName = stringPredicateContent.substr( 1 );
      // trim trailing whitespace from name
      size_t uNameEnd = stringAttrName.size();
      while( uNameEnd > 0 && std::isspace( (unsigned char)stringAttrName[uNameEnd - 1] ) ) { --uNameEnd; }
      xpathpredicateResult.m_stringAttributeName  = std::string( stringAttrName.substr( 0, uNameEnd ) );
      xpathpredicateResult.m_bAttributeValueCheck = false;
      return xpathpredicateResult;
   }

   // ## value equality: @name = "value" or @name = 'value'
   // extract attribute name (between '@' and '='), trimming inner whitespace
   std::string_view stringRawName = stringPredicateContent.substr( 1, uEqualsPosition - 1 );
   size_t uNameEnd = stringRawName.size();
   while( uNameEnd > 0 && std::isspace( (unsigned char)stringRawName[uNameEnd - 1] ) ) { --uNameEnd; }
   xpathpredicateResult.m_stringAttributeName = std::string( stringRawName.substr( 0, uNameEnd ) );

   // extract value — strip whitespace then strip surrounding quote characters
   std::string_view stringRawValue = stringPredicateContent.substr( uEqualsPosition + 1 );
   size_t uValueStart = 0;
   size_t uValueEnd   = stringRawValue.size();
   while( uValueStart < uValueEnd && std::isspace( (unsigned char)stringRawValue[uValueStart] ) ) { ++uValueStart; }
   while( uValueEnd   > uValueStart && std::isspace( (unsigned char)stringRawValue[uValueEnd - 1] ) ) { --uValueEnd; }

   if( uValueEnd > uValueStart )
   {
      char iQuote = stringRawValue[uValueStart];
      if( ( iQuote == '"' || iQuote == '\'' ) && uValueEnd > uValueStart + 1 && stringRawValue[uValueEnd - 1] == iQuote )
      {
         ++uValueStart;                                                  // strip opening quote
         --uValueEnd;                                                    // strip closing quote
      }
   }

   xpathpredicateResult.m_stringAttributeValue = std::string( stringRawValue.substr( uValueStart, uValueEnd - uValueStart ) );
   xpathpredicateResult.m_bAttributeValueCheck = true;
   return xpathpredicateResult;
}


// ============================================================================
// xpath  —  matching helpers
// ============================================================================

/**  -------------------------------------------------------------------------- match_tag_s
 * @brief  True when `elementCandidate` tag name satisfies the step's node-test
 *
 * An empty tag name or `"*"` is a wildcard and matches any element.
 * All other comparisons are case-insensitive.
 *
 * @param  elementCandidate  Element to test
 * @param  xpathstepStep     Step carrying the node-test
 * @return bool              True if the element passes the tag test
 */
bool xpath::match_tag_s( const element& elementCandidate, const xpath_step& xpathstepStep ) noexcept
{
   const std::string& stringTag = xpathstepStep.m_stringTagName;
   if( stringTag.empty() || stringTag == "*" ) { return true; }        // wildcard
   return element::equal_case_insensitive_s( elementCandidate.name(), stringTag );
}

/**  -------------------------------------------------------------------------- match_non_positional_predicates_s
 * @brief  True when `elementCandidate` satisfies every attribute predicate in the step
 *
 * Positional predicates (`ePredicateTypePosition`, `ePredicateTypeLast`) are
 * intentionally skipped here — they depend on sibling context and are resolved
 * in `collect_matching_children_s` after the candidate list is assembled.
 *
 * @param  elementCandidate  Element to test
 * @param  xpathstepStep     Step whose predicates are evaluated
 * @return bool              True if all non-positional predicates pass
 */
bool xpath::match_non_positional_predicates_s( const element& elementCandidate,
                                               const xpath_step& xpathstepStep ) noexcept
{
   for( const xpath_predicate& xpathpredicateCurrent : xpathstepStep.m_vectorPredicate )
   {
      if( xpathpredicateCurrent.m_eType != xpath_predicate::enumPredicateType::ePredicateTypeAttribute ) { continue; }

      if( !elementCandidate.has_attribute( xpathpredicateCurrent.m_stringAttributeName ) ) { return false; }

      if( xpathpredicateCurrent.m_bAttributeValueCheck )
      {
         std::string_view stringActualValue = elementCandidate.get_attribute( xpathpredicateCurrent.m_stringAttributeName );
         if( stringActualValue != xpathpredicateCurrent.m_stringAttributeValue ) { return false; }
      }
   }
   return true;
}


// ============================================================================
// xpath  —  collection helpers (mutable)
// ============================================================================

/**  -------------------------------------------------------------------------- collect_matching_children_s
 * @brief  Gather direct children of `elementParent` that satisfy `xpathstepStep`
 *
 * **Two-phase approach:**
 * 1. Tag-test + attribute predicates are applied to every child.  All passing
 *    children enter `vectorCandidate` in document order.
 * 2. Positional predicates are applied to `vectorCandidate`:
 *    - `[n]`      → keep only the n-th entry (1-based)
 *    - `[last()]` → keep only the last entry
 *    - none       → keep everything
 *
 * @param  elementParent   Parent node whose children are examined
 * @param  xpathstepStep   Compiled step
 * @param  vectorResult    Accumulator; survivors are appended here
 */
void xpath::collect_matching_children_s( element& elementParent,
                                         const xpath_step& xpathstepStep,
                                         std::vector<element*>& vectorResult )
{
   // ## phase 1 — filter by tag and non-positional predicates
   std::vector<element*> vectorCandidate;
   for( element& elementChild : elementParent )
   {
      if( match_tag_s( elementChild, xpathstepStep ) && match_non_positional_predicates_s( elementChild, xpathstepStep ) )
      {
         vectorCandidate.push_back( &elementChild );
      }
   }

   if( vectorCandidate.empty() ) { return; }

   // ## phase 2 — apply positional predicates
   bool bHasPositionalPredicate = false;
   bool bIsLastPredicate        = false;
   int  iRequiredPosition       = 0;

   for( const xpath_predicate& xpathpredicateCurrent : xpathstepStep.m_vectorPredicate )
   {
      if( xpathpredicateCurrent.m_eType == xpath_predicate::enumPredicateType::ePredicateTypePosition )
      {
         bHasPositionalPredicate = true;
         iRequiredPosition       = xpathpredicateCurrent.m_iPosition;
      }
      else if( xpathpredicateCurrent.m_eType == xpath_predicate::enumPredicateType::ePredicateTypeLast )
      {
         bHasPositionalPredicate = true;
         bIsLastPredicate        = true;
      }
   }

   if( !bHasPositionalPredicate )
   {
      vectorResult.insert( vectorResult.end(), vectorCandidate.begin(), vectorCandidate.end() );
      return;
   }

   if( bIsLastPredicate )
   {
      vectorResult.push_back( vectorCandidate.back() );                 // last() — always valid (size > 0 checked above)
   }
   else if( iRequiredPosition >= 1 && iRequiredPosition <= (int)vectorCandidate.size() )
   {
      vectorResult.push_back( vectorCandidate[(size_t)iRequiredPosition - 1] ); // [n] — 1-based to 0-based
   }
}

/**  -------------------------------------------------------------------------- collect_matching_descendants_s
 * @brief  Walk the subtree of `elementParent` and collect matching elements at every level
 *
 * For each node in the subtree, `collect_matching_children_s` is called so
 * that positional predicates are evaluated relative to each individual parent.
 * This mirrors XPath 1.0 descendant-axis semantics where `//div[2]` means
 * "the second `div` child of any ancestor", not "the second `div` in the document".
 *
 * @param  elementParent   Root of the subtree to traverse
 * @param  xpathstepStep   Step to evaluate at every level
 * @param  vectorResult    Accumulator; survivors are appended here
 */
void xpath::collect_matching_descendants_s( element& elementParent,
                                            const xpath_step& xpathstepStep,
                                            std::vector<element*>& vectorResult )
{
   collect_matching_children_s( elementParent, xpathstepStep, vectorResult );  // check this level's children
   for( element& elementChild : elementParent )
   {
      collect_matching_descendants_s( elementChild, xpathstepStep, vectorResult ); // recurse
   }
}


// ============================================================================
// xpath  —  collection helpers (const)
// ============================================================================

void xpath::collect_matching_children_s( const element& elementParent,
                                         const xpath_step& xpathstepStep,
                                         std::vector<const element*>& vectorResult )
{
   std::vector<const element*> vectorCandidate;
   for( const element& elementChild : elementParent )
   {
      if( match_tag_s( elementChild, xpathstepStep ) && match_non_positional_predicates_s( elementChild, xpathstepStep ) )
      {
         vectorCandidate.push_back( &elementChild );
      }
   }

   if( vectorCandidate.empty() ) { return; }

   bool bHasPositionalPredicate = false;
   bool bIsLastPredicate        = false;
   int  iRequiredPosition       = 0;

   for( const xpath_predicate& xpathpredicateCurrent : xpathstepStep.m_vectorPredicate )
   {
      if( xpathpredicateCurrent.m_eType == xpath_predicate::enumPredicateType::ePredicateTypePosition )
      {
         bHasPositionalPredicate = true;
         iRequiredPosition       = xpathpredicateCurrent.m_iPosition;
      }
      else if( xpathpredicateCurrent.m_eType == xpath_predicate::enumPredicateType::ePredicateTypeLast )
      {
         bHasPositionalPredicate = true;
         bIsLastPredicate        = true;
      }
   }

   if( !bHasPositionalPredicate )
   {
      vectorResult.insert( vectorResult.end(), vectorCandidate.begin(), vectorCandidate.end() );
      return;
   }

   if( bIsLastPredicate )
   {
      vectorResult.push_back( vectorCandidate.back() );
   }
   else if( iRequiredPosition >= 1 && iRequiredPosition <= (int)vectorCandidate.size() )
   {
      vectorResult.push_back( vectorCandidate[(size_t)iRequiredPosition - 1] );
   }
}

void xpath::collect_matching_descendants_s( const element& elementParent,
                                            const xpath_step& xpathstepStep,
                                            std::vector<const element*>& vectorResult )
{
   collect_matching_children_s( elementParent, xpathstepStep, vectorResult );
   for( const element& elementChild : elementParent )
   {
      collect_matching_descendants_s( elementChild, xpathstepStep, vectorResult );
   }
}

_GD_MODULES_HTML_END
