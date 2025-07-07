/**
 * @file Expression.h
 */

#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "gd/expression/gd_expression_parse_state.h"
#include "gd/parse/gd_parse_window_line.h"


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
struct ExpressionSource
{
// ## construction ------------------------------------------------------------
   ExpressionSource(): m_uCurrentLine{}, m_uLineCount{} {}
   ExpressionSource(std::string&& stringSource, uint64_t uCurrentLine = 0, uint64_t uLineCount = 0)
      : m_stringSource(std::move(stringSource)), m_uCurrentLine(uCurrentLine), m_uLineCount(uLineCount) {}
   // copy
   ExpressionSource(const ExpressionSource& o) { common_construct(o); }
   ExpressionSource(ExpressionSource&& o) noexcept { common_construct(std::move(o)); }
   // assign
   ExpressionSource& operator=(const ExpressionSource& o) { common_construct(o); return *this; }
   ExpressionSource& operator=(ExpressionSource&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~ExpressionSource() { CloseFile(); }
   // common copy
   void common_construct(const ExpressionSource& o) {
      m_stringSource = o.m_stringSource; m_stringFile = o.m_stringFile; m_uCurrentLine = o.m_uCurrentLine; m_uLineCount = o.m_uLineCount;
   }
   void common_construct(ExpressionSource&& o) noexcept {
      m_stringSource = std::move(o.m_stringSource); m_stringFile = std::move(o.m_stringFile); m_uCurrentLine = o.m_uCurrentLine; m_uLineCount = o.m_uLineCount;
   }

// ## methods -----------------------------------------------------------------
   const std::string& source() const { return m_stringSource; } ///< get source code
   void set_source(const std::string_view& stringSource) { m_stringSource = std::string(stringSource); } ///< set source code
   void set_source(std::string&& stringSource) { m_stringSource = std::move(stringSource); } ///< set source code
   const std::string& file() const { return m_stringFile; } ///< get file name
   void set_file(const std::string_view& stringFile) { m_stringFile = std::string(stringFile); } ///< set file name
   uint64_t goto_line() const { return m_uGotoLine; } ///< get goto line number
   void set_goto_line(uint64_t uGotoLine) { m_uGotoLine = uGotoLine; } ///< set goto line number
   void set_current_line(uint64_t uCurrentLine) { m_uCurrentLine = uCurrentLine; } ///< set current line number
   uint64_t current_line() const { return m_uCurrentLine; } ///< get current line number
   uint64_t line_count() const { return m_uLineCount; } ///< get total number of lines in source code
   void set_line_count(uint64_t uLineCount) { m_uLineCount = uLineCount; } ///< set total number of lines in source code

   std::pair<bool, std::string> GotoLine(uint64_t uLine); ///< go to line number, this will set current line number to the specified line number and return true if successful, false otherwise
   std::pair<bool, std::string> GotoLine(); ///< go to internal goto line number.

   std::pair<bool, std::string> OpenFile(); ///< open file for reading, this will read the source code from file if needed
   void CloseFile(); ///< close file if it is open, this will free the resources used for reading source code from file
   void Seek(uint64_t uOffset = 0u) { assert( m_ifstream.is_open() == true ); m_ifstream.seekg(uOffset, std::ios::beg); } ///< seek to offset in file, this will set the current position in file to the specified offset

   std::string GetGotoLineText() const;

   void AddResult(const std::string& stringResult) { m_vectorResult.push_back(stringResult); } ///< add result to vector of results, this will be used to store lines read from source code

   void Reset() { m_uCurrentLine = 0; m_uLineCount = 0; m_stringSource.clear(); m_line.reset(); } ///< reset positons in source code, this will reset current line number and total number of lines in source code


/** \name DEBUG
*///@{

//@}

// ## attributes -------------------------------------------------------------
   std::string m_stringSource; ///< source code
   std::string m_stringFile;    ///< file name
   uint64_t m_uGotoLine = 0; ///< line number to go to
   uint64_t m_uCurrentLine; ///< current line number
   uint64_t m_uLineCount; ///< total number of lines in source code

   // ## for reading source code from file                                     @TAG #expression #source #state
   std::ifstream m_ifstream; ///< input file stream, used to read source code from file if needed
   gd::parse::window::line m_line; ///< line window, used to read lines from source code
   gd::expression::parse::state m_state; ///< state of the parser, used to read lines from source code

   std::vector<std::string> m_vectorResult; ///< vector of lines, used to store lines read from source code



// ## free functions ----------------------------------------------------------

};

inline std::pair<bool, std::string> ExpressionSource::GotoLine(uint64_t uLine) {
   m_uGotoLine = uLine; // set goto line number
   return GotoLine(); // go to line number
}

// std::pair<bool, std::string> ExecuteExpression_g(const std::string_view& stringExpression,  const std::vector<gd::expression::value>& vectorVariable );   



NAMESPACE_AUTOMATION_END
