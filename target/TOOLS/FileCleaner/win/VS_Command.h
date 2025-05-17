/*
 * 
 */

#ifdef _WIN32


#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>


#if defined( __clang__ )
#elif defined( __GNUC__ )
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning(disable : 4278)
#endif


#include <windows.h>
#include <string>
#include <iostream>
#include <comdef.h>
#include <atlbase.h>

#include "gd/gd_table_column-buffer.h"


#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids
#import "libid:1A31287A-4D7D-413e-8E32-3B374931BD89" version("8.0") lcid("0") raw_interfaces_only named_guids


namespace VS
{

// tag dispatcher for visual studio output
struct tag_vs_output {};

 /**
  * \brief
  *
  *
  *
  \code
  \endcode
  */
class CVisualStudio
{
// ## construction -------------------------------------------------------------
public:
   CVisualStudio() {}
   CVisualStudio( CComPtr<EnvDTE::_DTE> pDTE ) { m_pDTE = pDTE; }
   // copy
   CVisualStudio(const CVisualStudio& o) { common_construct(o); }
   CVisualStudio(CVisualStudio&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CVisualStudio& operator=(const CVisualStudio& o) { common_construct(o); return *this; }
   CVisualStudio& operator=(CVisualStudio&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CVisualStudio() {}
private:
   // common copy
   void common_construct(const CVisualStudio& o) {}
   void common_construct(CVisualStudio&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   CComPtr<EnvDTE::_DTE> GetDTE() const { return m_pDTE; }
//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> Connect();
   std::pair<bool, std::string> Print( const std::string_view& stringText, tag_vs_output );
   std::pair<bool, std::string> Open(const std::vector<std::string>& vectorFile);


   std::pair<bool, std::string> ExecuteExpression( const std::string_view& stringExpression );

   void AddTable(const gd::table::dto::table* ptable) { m_vectorTable.push_back(ptable); }
   const gd::table::dto::table* GetTable() const { return m_vectorTable[0]; }

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
   CComPtr<EnvDTE::_DTE> m_pDTE;
   std::vector<const gd::table::dto::table*> m_vectorTable;


// ## free functions ------------------------------------------------------------
public:
   static std::pair<bool, std::string> Print_s( const std::string_view& stringText, tag_vs_output );

};

/// \brief Print to Visual Studio output window
inline std::pair<bool, std::string> CVisualStudio::Print_s( const std::string_view& stringText, tag_vs_output )
{
   CVisualStudio VS_;
   auto result = VS_.Connect();
   if( !result.first ) return result;
   return VS_.Print(stringText, tag_vs_output{});
}


} // namespace VC

#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif


#endif // _WIN32



