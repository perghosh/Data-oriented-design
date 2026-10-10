// @FILE [tag: 
#pragma once


#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>



/** @CLASS [name: CApplication] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CApplication
{
   // @API [tag: construction]
public:
   CApplication() {}
   // copy
   CApplication(const CApplication& o) { common_construct(o); }
   CApplication(CApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CApplication& operator=(const CApplication& o) { common_construct(o); return *this; }
   CApplication& operator=(CApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CApplication() {}
private:
   // common copy
   void common_construct(const CApplication& o) {}
   void common_construct(CApplication&& o) noexcept {}

   // @API [tag: operator]
public:


   // ## methods ------------------------------------------------------------------
public:
   // @API [tag: get, set]

   // @API [tag: operation]


protected:
   // @API [tag: internal]

public:
   // @API [tag: debug]

   // ## attributes ----------------------------------------------------------------
public:


   // @API [tag: free-functions]
public:



};