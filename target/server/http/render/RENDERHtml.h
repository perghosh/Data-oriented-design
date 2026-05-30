// @PAGE 

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class CDocument;
class CAPIContext;


/** @CLASS [name: CRENDERHtml] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CRENDERHtml
{
   // @API [tag: construction]
public:
   CRENDERHtml() {}
   explicit CRENDERHtml(std::string_view stringPath) : m_stringPath(stringPath) {}
   CRENDERHtml(std::string_view stringPath, std::string_view stringPage) : m_stringPath(stringPath), m_stringPage(stringPage) {}
   CRENDERHtml(const CAPIContext* papicontext) : m_papicontext(papicontext) {}
   CRENDERHtml(const CAPIContext* papicontext, std::string_view stringPath) : m_papicontext(papicontext), m_stringPath(stringPath) {}
   CRENDERHtml(const CAPIContext* papicontext, std::string_view stringPath, std::string_view stringPage) : m_papicontext(papicontext), m_stringPath(stringPath), m_stringPage(stringPage) {}
    
   // copy
   CRENDERHtml(const CRENDERHtml& o) { common_construct(o); }
   CRENDERHtml(CRENDERHtml&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CRENDERHtml& operator=(const CRENDERHtml& o) { common_construct(o); return *this; }
   CRENDERHtml& operator=(CRENDERHtml&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CRENDERHtml() {}
private:
   // common copy
   void common_construct(const CRENDERHtml& o) {}
   void common_construct(CRENDERHtml&& o) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   std::pair<bool, std::string> Render(std::string& stringRendered);

   std::pair<bool, std::string> Run(std::string_view stringType, std::string_view stringCode, std::string* pstringPage);


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   const CAPIContext* m_papicontext = nullptr; ///< pointer to api context, used to get arguments for query, access internal data in web server
   std::string m_stringPath;
   std::string m_stringPage;


// @API [tag: free-functions]
public:



};
