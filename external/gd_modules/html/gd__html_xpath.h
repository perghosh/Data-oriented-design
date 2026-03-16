// @FILE [tag: html,dom,xpath] [description: Basic XPath 1.0 subset evaluator for the html::element tree] [type: header] [name: gd_tools_html_xpath.h]

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "gd__html_document.h"

_GD_MODULES_HTML_BEGIN

// ============================================================================
// @CLASS [tag: xpath_predicate] [summary: A single predicate clause inside `[…]`]
//
// Three predicate kinds are recognised:
//   ePredicateTypeAttribute  — `[@attr]` (existence) or `[@attr="value"]` (equality)
//   ePredicateTypePosition   — `[n]` selects the n-th match among siblings (1-based)
//   ePredicateTypeLast       — `[last()]` selects the last match among siblings
// ============================================================================

struct xpath_predicate
{
   enum class enumPredicateType : uint8_t
   {
      ePredicateTypeAttribute = 0,  ///< `[@name]` or `[@name="value"]`
      ePredicateTypePosition  = 1,  ///< `[n]` — 1-based sibling index
      ePredicateTypeLast      = 2,  ///< `[last()]`
   };

   enumPredicateType m_eType                = enumPredicateType::ePredicateTypeAttribute; ///< Discriminator
   std::string       m_stringAttributeName;  ///< Attribute name; used when `m_eType == ePredicateTypeAttribute`
   std::string       m_stringAttributeValue; ///< Expected value; used when `m_bAttributeValueCheck` is true
   bool              m_bAttributeValueCheck  = false; ///< True → compare value, false → existence only
   int               m_iPosition             = 0;     ///< 1-based index; used when `m_eType == ePredicateTypePosition`
};


// ============================================================================
// @CLASS [tag: xpath_step] [summary: One axis+node-test unit in a parsed XPath expression]
//
// An XPath expression such as `//table/tr[2]/td[@class="cell"]` is decomposed
// into three `xpath_step` instances:
//   { bIsDescendant:true,  tag:"table" }
//   { bIsDescendant:false, tag:"tr",   predicates:[{position,2}] }
//   { bIsDescendant:false, tag:"td",   predicates:[{attribute,"class","cell"}] }
// ============================================================================

struct xpath_step
{
   bool                         m_bIsDescendant = false; ///< True when `//` precedes this step
   std::string                  m_stringTagName;         ///< Tag to match; empty string or `"*"` = any tag
   std::vector<xpath_predicate> m_vectorPredicate;       ///< Zero or more predicate clauses
};


// ============================================================================
// @CLASS [tag: xpath] [summary: Basic XPath evaluator over the `html::element` tree]
//
// Supported syntax subset:
//   /tag/tag/…         — absolute child-axis steps from the context root
//   //tag              — descendant-or-self axis (finds matches anywhere in subtree)
//   *                  — wildcard: matches any tag name
//   [@attr]            — attribute existence predicate
//   [@attr="value"]    — attribute value equality (also single-quoted values)
//   [n]                — positional predicate (1-based, per-parent)
//   [last()]           — last matching sibling predicate
//
// Multiple predicates on the same step are ANDed together.
// Position predicates (`[n]`, `[last()]`) are applied after all attribute
// predicates so the position counts only the already-filtered candidates.
//
// @NOTE  `evaluate` returns the **first** match in document order.
//        `evaluate_all` returns every match in document order.
// ============================================================================

class xpath
{
public:

   /** -------------------------------------------------------------------------- evaluate
    * @brief  Evaluate an XPath expression and return the first matching element
    * @param  elementRoot        Root of the subtree to search
    * @param  stringExpression   XPath expression string
    * @return element*           First match in document order, or nullptr
    */
   static element*       evaluate( element& elementRoot, std::string_view stringExpression );
   static const element* evaluate( const element& elementRoot, std::string_view stringExpression );

   /** -------------------------------------------------------------------------- evaluate_all
    * @brief  Evaluate an XPath expression and return every matching element
    * @param  elementRoot        Root of the subtree to search
    * @param  stringExpression   XPath expression string
    * @return vector<element*>   All matches in document order; empty if none
    */
   static std::vector<element*>       evaluate_all( element& elementRoot, std::string_view stringExpression );
   static std::vector<const element*> evaluate_all( const element& elementRoot, std::string_view stringExpression );

private:

   /// Parse a complete XPath expression into an ordered list of steps --------- parse_expression_s
   static std::vector<xpath_step> parse_expression_s( std::string_view stringExpression );

   /// Parse the content inside `[…]` into a single predicate descriptor ------- parse_predicate_s
   static xpath_predicate parse_predicate_s( std::string_view stringPredicateContent );

   /// True when `elementCandidate` tag name matches the step's node-test ------- match_tag_s
   static bool match_tag_s( const element& elementCandidate, const xpath_step& xpathstepStep ) noexcept;

   /// True when `elementCandidate` satisfies every **non-positional** predicate  match_non_positional_predicates_s
   static bool match_non_positional_predicates_s( const element& elementCandidate,
                                                  const xpath_step& xpathstepStep ) noexcept;

   /** -------------------------------------------------------------------------- collect_matching_children_s
    * @brief  Find direct children of `elementParent` that satisfy `xpathstepStep`.
    *
    * Non-positional predicates are tested first; if any positional predicate
    * is present it is applied last against the filtered candidate list so that
    * `[2]` selects the second element that already passed all attribute checks.
    *
    * @param  elementParent    Node whose children are examined
    * @param  xpathstepStep    Step containing tag-test and predicates
    * @param  vectorResult     Accumulator — matching pointers are appended
    */
   static void collect_matching_children_s( element& elementParent,
                                            const xpath_step& xpathstepStep,
                                            std::vector<element*>& vectorResult );

   static void collect_matching_children_s( const element& elementParent,
                                            const xpath_step& xpathstepStep,
                                            std::vector<const element*>& vectorResult );

   /** -------------------------------------------------------------------------- collect_matching_descendants_s
    * @brief  Walk the entire subtree of `elementParent` and, for each node,
    *         collect its children that match `xpathstepStep`.
    *
    * This correctly handles position predicates because each parent's children
    * are evaluated independently — `[2]` picks the second match per parent,
    * mirroring XPath 1.0 semantics for the descendant axis.
    *
    * @param  elementParent    Root of the subtree to search
    * @param  xpathstepStep    Step to evaluate at every level
    * @param  vectorResult     Accumulator — matching pointers are appended
    */
   static void collect_matching_descendants_s( element& elementParent,
                                               const xpath_step& xpathstepStep,
                                               std::vector<element*>& vectorResult );

   static void collect_matching_descendants_s( const element& elementParent,
                                               const xpath_step& xpathstepStep,
                                               std::vector<const element*>& vectorResult );
};

_GD_MODULES_HTML_END
