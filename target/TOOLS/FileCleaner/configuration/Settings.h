/**
* @file Settings.h
*  
*/

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <filesystem>
#include <iostream>

#include "gd/gd_arguments_shared.h"

#include "../Document.h"

#ifndef NAMESPACE_CONFIGURATION_BEGIN

#  define NAMESPACE_CONFIGURATION_BEGIN namespace CONFIGURATION {
#  define NAMESPACE_CONFIGURATION_END  }

#endif

NAMESPACE_CONFIGURATION_BEGIN  

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CSettings
{
public:
   struct settings
   {
      // ## construction ------------------------------------------------------------
      settings() = default;
      settings( const std::string& name_, const std::string& description_ ): m_stringName(name_), m_stringDescription(description_) {}
      settings(const std::string& name_, const std::string& description_, const std::string& command_ ): m_stringName(name_), m_stringDescription(description_), m_stringCommand(command_) {}
      settings(const std::string& name_, const std::string& description_, const std::string& command_, const std::vector<gd::argument::arguments>& v_) :
         m_stringName(name_), m_stringDescription(description_), m_stringCommand(command_), m_vectorArguments(v_) {
      }
      settings(const settings& o) { common_construct(o); }
      settings(settings&& o) noexcept { common_construct(std::move(o)); }
      settings& operator=(const settings& o) { common_construct(o); return *this; }
      settings& operator=(settings&& o) noexcept { common_construct(std::move(o)); return *this; }
      ~settings() = default;

      void common_construct(const settings& o) {
         m_stringName = o.m_stringName; m_stringDescription = o.m_stringDescription; m_stringCommand = o.m_stringCommand;
         m_vectorArguments = o.m_vectorArguments;
      }
      void common_construct(settings&& o) noexcept {
         m_stringName = std::move(o.m_stringName); m_stringDescription = std::move(o.m_stringDescription); m_stringCommand = std::move(o.m_stringCommand);
         m_vectorArguments = std::move(o.m_vectorArguments);
      }

      // ## getters and setters -----------------------------------------------------
      // m_stringName
      const std::string& get_name() const { return m_stringName; }
      const std::string& name() const { return m_stringName; }
      void set_name(const std::string& name_) { m_stringName = name_; }

      // m_stringDescription
      const std::string& get_description() const { return m_stringDescription; }
      const std::string& description() const { return m_stringDescription; }
      void set_description(const std::string& description_) { m_stringDescription = description_; }

      // m_stringCommand
      const std::string& get_command() const { return m_stringCommand; }
      const std::string& command() const { return m_stringCommand; }
      void set_command(const std::string& command_) { m_stringCommand = command_; }

      // m_vectorArguments
      const std::vector<gd::argument::arguments>& get_arguments() const { return m_vectorArguments; }
      const std::vector<gd::argument::arguments>& arguments() const { return m_vectorArguments; }
      void set_arguments(const std::vector<gd::argument::arguments>& v_) { m_vectorArguments = v_; }

      // ## attributes --------------------------------------------------------------
      std::string m_stringName; ///< Name of the settings
      std::string m_stringDescription; ///< Description of the settings
      std::string m_stringCommand; ///< Command to execute, raw string in same format as in the command line
      std::vector<gd::argument::arguments> m_vectorArguments; ///< Arguments to pass to the command
   };

public:
   // ## types -------------------------------------------------------------------
   using value_type = settings; ///< Type of the value stored in the settings
   using const_value_type = const settings; ///< Type of the value stored in the settings (const version)
   using reference = settings&; ///< Reference type for non-const access
   using const_reference = const settings&; ///< Reference type for const access

   using iterator = std::vector<settings>::iterator; ///< Iterator type for non-const access
   using const_iterator = std::vector<settings>::const_iterator; ///< Iterator type for const access
   // ## constants ---------------------------------------------------------------
   static constexpr const char* CLASS_NAME = "CSettings"; ///< Class name for debugging purposes
   
// ## construction -------------------------------------------------------------
public:
   CSettings() {}
   // copy
   CSettings(const CSettings& o) { common_construct(o); }
   CSettings(CSettings&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CSettings& operator=(const CSettings& o) { common_construct(o); return *this; }
   CSettings& operator=(CSettings&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CSettings() {}
private:
   // common copy
   void common_construct(const CSettings& o) {}
   void common_construct(CSettings&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   /// Access settings by index
   reference operator[](size_t uIndex) { return m_vectorSettings[uIndex]; }
   const_reference operator[](size_t uIndex) const { return m_vectorSettings[uIndex]; }

   /// Access settings by name (throws if not found)
   reference operator[](const std::string& name_);
   const_reference operator[](const std::string& name_) const;

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   /// Get settings at specified index with bounds checking
   reference At(size_t uIndex) { return m_vectorSettings.at(uIndex); }
   const_reference At(size_t uIndex) const { return m_vectorSettings.at(uIndex); }

   /// Get first/last settings
   reference Front() { return m_vectorSettings.front(); }
   const_reference Front() const { return m_vectorSettings.front(); }
   reference Back() { return m_vectorSettings.back(); }
   const_reference Back() const { return m_vectorSettings.back(); }

   /// Get underlying vector (for advanced operations)
   std::vector<settings>& GetSettings() { return m_vectorSettings; }
   const std::vector<settings>& GetSettings() const { return m_vectorSettings; }
//@}

/** \name OPERATION
*///@{
   settings* Add( const settings& settings_ ) { m_vectorSettings.push_back( settings_ ); return &m_vectorSettings.back(); }
   settings* Add( settings&& settings_ ) { m_vectorSettings.push_back(std::move(settings_)); return &m_vectorSettings.back(); }
   settings* Add( const std::string& name_, const std::string& description_ ) { m_vectorSettings.emplace_back(name_, description_); return &m_vectorSettings.back(); }
   settings* Add( const std::string& name_, const std::string& description_, const std::string& command_ ) { m_vectorSettings.emplace_back(name_, description_, command_); return &m_vectorSettings.back(); }

   settings* AddCommand( const std::string& name_, const std::string& description_, const std::string& command_ );

   /// Insert settings at specific position
   iterator Insert(const_iterator it, const settings& settings_) { return m_vectorSettings.insert(it, settings_); }
   iterator Insert(const_iterator it, settings&& settings_) { return m_vectorSettings.insert(it, std::move(settings_)); }


   /// Remove settings by name. Returns true if a setting was removed.
   bool Remove(const std::string& name_);
   /// Remove settings at the specified iterator position.
   iterator Remove(const_iterator it) { return m_vectorSettings.erase(it); }
   /// Remove settings in the specified iterator range [first_, last_).
   iterator Remove(const_iterator itFirst, const_iterator itLast) { return m_vectorSettings.erase(itFirst, itLast); }

   // ## Container operations

   void Clear() { m_vectorSettings.clear(); }
   void Reserve(size_t capacity_) { m_vectorSettings.reserve(capacity_); }
   void ShrinkToFit() { m_vectorSettings.shrink_to_fit(); }

   settings* Find( const std::string& name_ );
   const settings* Find( const std::string& name_ ) const;
   bool Exists( const std::string& name_ ) const { return Find(name_) != nullptr;  }
   size_t Count() const { return m_vectorSettings.size(); }
   size_t Size() const { return m_vectorSettings.size(); }

   // ## Iterators
   
   // ### begin/end for non-const

   iterator begin() { return m_vectorSettings.begin(); }
   iterator end() { return m_vectorSettings.end(); }

   // ### begin/end for const

   const_iterator begin() const { return m_vectorSettings.begin(); }
   const_iterator end() const { return m_vectorSettings.end(); }
   const_iterator cbegin() const { return m_vectorSettings.cbegin(); }
   const_iterator cend() const { return m_vectorSettings.cend(); }
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
   std::vector<settings> m_vectorSettings; ///< Vector of settings


// ## free functions ------------------------------------------------------------
public:

};

/// @brief Find settings by name. Returns nullptr if not found.
inline CSettings::reference CSettings::operator[](const std::string& name_) {
   auto* p_ = Find(name_);
   if(p_ == nullptr) { assert(false); throw std::out_of_range("Settings with name '" + name_ + "' not found"); }
   return *p_;
}

/// @brief Access settings by name (const version). Throws if not found.
inline CSettings::const_reference CSettings::operator[](const std::string& name_) const {
   const auto* p_ = Find(name_);
   if(p_ == nullptr) { assert(false); throw std::out_of_range("Settings with name '" + name_ + "' not found"); }
   return *p_;
}

/// @brief Remove settings by name. Returns true if a setting was removed.
inline bool CSettings::Remove(const std::string& name_) {
   auto it = std::find_if(m_vectorSettings.begin(), m_vectorSettings.end(), [&](const settings& s) { return s.get_name() == name_; });
   if(it != m_vectorSettings.end()) { m_vectorSettings.erase(it); return true; }
   return false;
}




NAMESPACE_CONFIGURATION_END