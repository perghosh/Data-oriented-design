#include <iostream>
#include <filesystem>


#include "gd/gd_file.h"
#include "gd/gd_log_logger_define.h"

#include "ApplicationBasic.h"


APPLICATION_APPLICATION_BASIC_BEGIN

//CApplication applicationApp_g;
unsigned CApplication::m_uInstanceCount_s = 0;

// root marker file is used to find root file that is relative to other important files

CApplication::CApplication()
{
   CApplication::m_uInstanceCount_s++;
}


CApplication::~CApplication() 
{ 
}

// ## Private copy and assignment to avoid copy application object
CApplication::CApplication( CApplication& o ) {}
CApplication& CApplication::operator=( const CApplication& o ) { return *this; }


/** ---------------------------------------------------------------------------
 * @brief Harvest arguments sent to main method, this should be overridden
 * @param iArgumentCount number of arguments
 * @param ppbszArgument pointer list to argument values
 * @param unused callback not used here
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> CApplication::Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view&)> )
{
   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Initialize application instance, this should be overridden
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> CApplication::Initialize()
{
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Exit application instance, this should be overridden
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> CApplication::Exit()
{
   return { true, "" };
}

gd::argument::arguments CApplication::PROPERTY_Get( const std::initializer_list<std::string_view>& listName, gd::types::tag_argument ) const
{
   gd::argument::arguments arguments_;
   for( const auto& it : m_vectorProperty )
   {
      // ## Compare with it.first for all values in listName
      for( const auto& name_ : listName )
      {
         if( it.first == name_ ) 
         {
            arguments_.push_back( it.first, it.second );
            break;
         }
      }
   }

   return arguments_;
}


APPLICATION_APPLICATION_BASIC_END
