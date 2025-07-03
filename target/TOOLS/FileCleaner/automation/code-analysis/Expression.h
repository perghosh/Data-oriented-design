/**
 * @file Expression.h
 */

#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#ifndef NAMESPACE_AUTOMATION_BEGIN

#  define NAMESPACE_AUTOMATION_BEGIN namespace AUTOMATION {
#  define NAMESPACE_AUTOMATION_END  }

#endif

NAMESPACE_AUTOMATION_BEGIN

extern const gd::expression::method pmethodSelect_g[];
extern const size_t uMethodSelectSize_g;

/**
 * \brief Information about the source used in methods injected into expression engine.
 *
 *
 */
struct expression_source
{
// ## construction ------------------------------------------------------------
   expression_source() {}
   expression_source(std::string&& stringSource, uint64_t uCurrentLine = 0, uint64_t uLineCount = 0)
      : m_stringSource(std::move(stringSource)), m_uCurrentLine(uCurrentLine), m_uLineCount(uLineCount) {}
   // copy
   expression_source(const expression_source& o) { common_construct(o); }
   expression_source(expression_source&& o) noexcept { common_construct(std::move(o)); }
   // assign
   expression_source& operator=(const expression_source& o) { common_construct(o); return *this; }
   expression_source& operator=(expression_source&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~expression_source() {}
   // common copy
   void common_construct(const expression_source& o) {
      m_stringSource = o.m_stringSource; m_stringFile = o.m_stringFile; m_uCurrentLine = o.m_uCurrentLine; m_uLineCount = o.m_uLineCount;
   }
   void common_construct(expression_source&& o) noexcept {
      m_stringSource = std::move(o.m_stringSource); m_stringFile = std::move(o.m_stringFile); m_uCurrentLine = o.m_uCurrentLine; m_uLineCount = o.m_uLineCount;
   }

// ## methods -----------------------------------------------------------------

/** \name DEBUG
*///@{

//@}

// ## attributes -------------------------------------------------------------
   std::string m_stringSource; ///< source code
   std::string m_stringFile;    ///< file name
   uint64_t m_uCurrentLine; ///< current line number
   uint64_t m_uLineCount; ///< total number of lines in source code


// ## free functions ----------------------------------------------------------

};

// std::pair<bool, std::string> ExecuteExpression_g(const std::string_view& stringExpression,  const std::vector<gd::expression::value>& vectorVariable );   



NAMESPACE_AUTOMATION_END
