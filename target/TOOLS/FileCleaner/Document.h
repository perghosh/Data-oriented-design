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
   CDocument( const std::string_view& stringName ) { m_arguments.append( "name", stringName ); }
   CDocument( const gd::argument::shared::arguments& arguments_ ): m_arguments( arguments_ ) {}
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
   gd::variant_view Get(const std::string_view& stringName) const { return m_arguments[stringName].as_variant_view(); }
   void Set(const std::string_view& stringName, const gd::variant_view& variantValue) { m_arguments.set(stringName, variantValue); }

   std::string_view GetName() const { return m_arguments["name"].as_string_view(); }
   void SetName(const std::string_view& stringName) { m_arguments.set("name", stringName); }

//@}

/** \name OPERATION
*///@{
   /// Load document from file
   std::pair<bool, std::string> Load(const std::string_view& stringPath);

   /// Save document to file
   std::pair<bool, std::string> Save(const std::string_view& stringPath);

   /// Count characters in file data
   size_t Count( uint8_t uCharacter ) const;
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
   gd::argument::shared::arguments m_arguments;///< document information (members)
   std::vector<uint8_t> m_vectorData;///< document data (file content)

// ## free functions ------------------------------------------------------------
public:



};