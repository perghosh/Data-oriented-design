#pragma once

#include <utility>
#include <iostream>

#include "gd/gd_arguments.h"


constexpr std::string_view ROOT_MARKER = "__root";

/// create applicaiton object
//extern std::unique_ptr<CApplication> APPLICATION_Create();

/// Get root folder for application, walk upp in folder hierarchy and finds to specified file
extern std::string FOLDER_GetRoot_g( const std::string_view& stringSubfolder );

/// Get root folder using hard coded root file
inline std::string FOLDER_GetRoot_g() { return FOLDER_GetRoot_g( "" ); }


/**
* \brief data transfer object to access application arguments
*/
struct MainArguments
{
   // ## construction -------------------------------------------------------------

   MainArguments() {}
   MainArguments( int iArgumentCount, char** ppbszArgumentValue ): m_iArgumentCount{iArgumentCount}, m_ppbszArgumentValue{ppbszArgumentValue} {}
   ~MainArguments() {}

   operator gd::argument::arguments&() { return m_argumentsOptions; }
   operator const gd::argument::arguments&() const { return m_argumentsOptions; }
   MainArguments& operator()( const std::string_view& n_, const gd::variant_view& v_ ) { m_argumentsOptions.set( n_, v_ ); return *this; }
   const char* operator[]( int iIndex ) const { return m_ppbszArgumentValue[iIndex]; }
   const gd::variant_view operator[]( const std::string_view& stringName ) const { return m_argumentsOptions[stringName].as_variant_view(); }

   // ## attributes
   /// options used in current test scenario, like global values
   gd::argument::arguments m_argumentsOptions;
   int m_iArgumentCount = 0;              ///< Number of arguments
   char** m_ppbszArgumentValue = nullptr; ///< pointer to argument values
};

extern MainArguments mainarguments_g;

