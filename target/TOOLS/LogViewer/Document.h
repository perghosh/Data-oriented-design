// @FILE [tag: document, data] [description: Manage data in LogViewer, acts as a type of facade for data in application] [type: header] [name: Document.h]

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

#include "Application.h"


/** @CLASS [name: CDocument] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CDocument
{
   // @API [tag: construction]
public:
   CDocument() {}
   // copy
   CDocument(const CDocument& o) { common_construct(o); }
   CDocument(CDocument&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CDocument& operator=(const CDocument& o) { common_construct(o); return *this; }
   CDocument& operator=(CDocument&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CDocument() {}
private:
   // common copy
   void common_construct(const CDocument& o) {}
   void common_construct(CDocument&& o) noexcept {}

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
   CApplication* m_pApplication = nullptr;


   // @API [tag: free-functions]
public:



};