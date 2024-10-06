#pragma once

#include <functional>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"

#ifndef APPLICATION_APPLICATION_BASIC_BEGIN
#  define APPLICATION_APPLICATION_BASIC_BEGIN namespace application { namespace basic {
#  define APPLICATION_APPLICATION_BASIC_END } }
#endif

APPLICATION_APPLICATION_BASIC_BEGIN

/**
 * \brief Application root class with basic functionality
 *
 *
 *
 \code
 \endcode
 */
class CApplication
{
   // ## construction -------------------------------------------------------------
public:
   CApplication();
   virtual ~CApplication();

private:
   CApplication( CApplication& o );
   CApplication& operator=( const CApplication& o );

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   std::string GetFolder( const std::string_view& stringFolder ) const { assert( m_argumentsFolder.find(stringFolder) != nullptr );
      return m_argumentsFolder[stringFolder].get_string(); }
   void SetFolder( const std::string_view& stringName, const std::string_view& stringFolder ) { m_argumentsFolder.set( stringName, stringFolder ); }
//@}

/** \name INTERFACE
*///@{
//@}


/** \name OPERATION
*///@{
   /// Method that can be used to harvest main arguments
   virtual std::pair<bool, std::string> Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view& )> process_ );
   /// Initialize application to connect, load needed data and other stuff to make it work
   virtual std::pair<bool, std::string> Initialize();
   /// Use this for clean up
   virtual std::pair<bool, std::string> Exit();


/** \name PROPERTY
*///@{
   // ## Property methods
   void PROPERTY_Add( const std::string_view& stringName, const gd::variant_view& variantviewValue );
   void PROPERTY_Set( const std::string_view& stringName, const gd::variant_view& variantviewValue );
   gd::variant_view PROPERTY_Get( size_t uIndex ) const { return gd::variant_view( m_vectorProperty.at(uIndex).second ); }
   gd::variant_view PROPERTY_Get( const std::string_view& stringName ) const;
   gd::variant_view PROPERTY_Get( const std::string_view& stringName, gd::variant_view variantviewDefault ) const;
   std::string PROPERTY_GetName( size_t uIndex ) const { return m_vectorProperty.at(uIndex).first; }
   bool PROPERTY_Has( const std::string_view& stringName );
   size_t PROPERTY_Size() const noexcept { return m_vectorProperty.size(); }
//@}

   /// ## Application version information

   /// Set version property
   template<typename TYPE>
   void VERSION_Set( const std::string_view& stringName, TYPE value_ ) { m_argumentsVersion.set( stringName, value_ ); }
   /// Get version property
   gd::argument::arguments::argument VERSION_Get( const std::string_view& stringName ) { return m_argumentsVersion.get_argument( stringName ); }

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
   std::vector<std::pair<std::string, gd::variant>> m_vectorProperty;///< global properties for application, name and value
   gd::argument::arguments m_argumentsFolder;            ///< Application folders
   gd::argument::arguments m_argumentsVersion;           ///< application version information

   static unsigned m_uInstanceCount_s; ///< number of running instances

// ## free functions ------------------------------------------------------------
public:



};

/** ---------------------------------------------------------------------------
 * @brief Adds property value to application, properties are just values that works
 *       Like global varables for application.
 * @param stringName property name
 * @param variantviewValue property value
*/
inline void CApplication::PROPERTY_Add( const std::string_view& stringName, const gd::variant_view& variantviewValue ) {
   m_vectorProperty.push_back( std::pair<std::string, gd::variant>{ stringName, variantviewValue.as_variant() });
}

/** ---------------------------------------------------------------------------
 * @brief Set property value
 * @param stringName property name
 * @param variantviewValue value set for property name
*/
inline void CApplication::PROPERTY_Set( const std::string_view& stringName, const gd::variant_view& variantviewValue )
{
   for( auto it = std::begin( m_vectorProperty ), itEnd = std::end( m_vectorProperty ); it != itEnd; it++ ) {
      if( it->first == stringName )      {
         it->second = variantviewValue.as_variant();
         return;
      }
   }
   PROPERTY_Add( stringName,  variantviewValue );
}


/** ---------------------------------------------------------------------------
 * @brief Get property value
 * @param stringName name for property value to get
 * @return value for property if found
*/
inline gd::variant_view CApplication::PROPERTY_Get( const std::string_view& stringName ) const {
   for( auto it = std::begin( m_vectorProperty ), itEnd = std::end( m_vectorProperty ); it != itEnd; it++ ) {
      if( it->first == stringName ) return gd::variant_view( it->second );
   }
   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief Get property value or if not found then return default
 * @param stringName name for property value to get
 * @param variantviewDefault default value if property do not exist
 * @return value for property if found or default if not found
*/
inline gd::variant_view CApplication::PROPERTY_Get( const std::string_view& stringName, gd::variant_view variantviewDefault ) const {
   for( auto it = std::begin( m_vectorProperty ), itEnd = std::end( m_vectorProperty ); it != itEnd; it++ ) {
      if( it->first == stringName ) return gd::variant_view( it->second );
   }
   return variantviewDefault;
}

/** ---------------------------------------------------------------------------
 * @brief Check if property exists
 * @param stringName name for property to check for existance
 * @return bool true if property name is found, false if not
*/
inline bool CApplication::PROPERTY_Has( const std::string_view& stringName )
{
   for( auto it = std::begin( m_vectorProperty ), itEnd = std::end( m_vectorProperty ); it != itEnd; it++ ) {
      if( it->first == stringName ) return true;
   }
   return false;
}


APPLICATION_APPLICATION_BASIC_END