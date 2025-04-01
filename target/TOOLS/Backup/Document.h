/**
 * \file Document.h
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.Document` - Represents a single argument in `arguments`.
 */

#pragma once

// ## STL

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ## GD

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#include "command/Commands.h"





/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CDocument
{
// ## construction -------------------------------------------------------------
public: // 0TAG0construct.Document
   CDocument() {}
   CDocument(const std::string_view& stringName) : m_stringName(stringName) {}
   CDocument(CCommands&& commands) { m_vectorCommand.emplace_back(std::move(commands)); }
// copy
   CDocument(const CDocument& o) { common_construct(o); }
   CDocument(CDocument&& o) noexcept { common_construct(std::move(o)); }
// assign
   CDocument& operator=(const CDocument& o) { common_construct(o); return *this; }
   CDocument& operator=(CDocument&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CDocument() {}
private:
// common copy
   void common_construct(const CDocument& o);
   void common_construct(CDocument&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   std::string_view GetName() const { return m_stringName; }

//@}

/** \name OPERATION
*///@{
//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::string m_stringName; ///< Name of the document
   std::vector<CCommands> m_vectorCommand; ///< List of commands for this document

// ## free functions ------------------------------------------------------------
public:



};