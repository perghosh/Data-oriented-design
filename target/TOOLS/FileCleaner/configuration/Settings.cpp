/**
 * @file RunSettings.cpp
 */

#include "Settings.h"


NAMESPACE_CONFIGURATION_BEGIN


CSettings::settings* CSettings::AddCommand( const std::string& name_, const std::string& description_, const std::string& command_ ) 
{
   settings settings_( name_, description_, command_ );
   m_vectorSettings.emplace_back(settings_);
   return &m_vectorSettings.back();
}


CSettings::settings* CSettings::Find( const std::string& name_ )
{
   for ( auto& setting : m_vectorSettings )
   {
      if ( setting.name() == name_ ) return &setting;
   }
   return nullptr;
}

const CSettings::settings* CSettings::Find( const std::string& name_ ) const
{
   for ( const auto& setting : m_vectorSettings )
   {
      if ( setting.name() == name_ ) return &setting;
   }
   return nullptr;
}



NAMESPACE_CONFIGURATION_END