/**
* @file RunSettings.h
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
      settings(const std::string& name_, const std::string& description_ ): m_stringName(name_), m_stringDescription(description_) {}
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
      std::string m_stringCommand; ///< Command to execute
      std::vector<gd::argument::arguments> m_vectorArguments; ///< Arguments to pass to the command
   };

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



};




NAMESPACE_CONFIGURATION_END