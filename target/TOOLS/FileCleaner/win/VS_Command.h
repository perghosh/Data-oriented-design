/*
 * 
 */

#ifdef _WIN32


#pragma once

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


// ## free functions ------------------------------------------------------------
public:
   static std::pair<bool, std::string> Print_s( const std::string_view& stringText, tag_vs_output );


};


} // namespace VC

#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif


#endif // _WIN32



