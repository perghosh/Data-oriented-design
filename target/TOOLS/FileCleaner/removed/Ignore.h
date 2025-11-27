/**
* @file Ignore.h
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




/** @CLASS [tag: CIgnore] [summary: Settings for files to ignore  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CIgnore
{
public:
   /// git related stuff
   struct tag_git {};

   // ## construction -------------------------------------------------------------
public:
   CIgnore() {}
   // copy
   CIgnore(const CIgnore& o) { common_construct(o); }
   CIgnore(CIgnore&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CIgnore& operator=(const CIgnore& o) { common_construct(o); return *this; }
   CIgnore& operator=(CIgnore&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CIgnore() {}
private:
   // common copy
   void common_construct(const CIgnore& o) {}
   void common_construct(CIgnore&& o) noexcept {}

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
   std::vector<gd::argument::shared::arguments> m_vectorArguments; ///< vector of arguments

   // ## free functions ------------------------------------------------------------
public:
   /// \brief Read the ignore with git formating into vector of strings.
   static std::pair<bool, std::string> Read_s(const std::string& stringPath, std::vector<std::string>* pvectorPatterns, tag_git);

   static std::string_view Type_s( const std::string_view& stringPattern, tag_git );


};

NAMESPACE_CONFIGURATION_END
