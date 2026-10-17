// 

#pragma once



/** @CLASS [name: menu] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class menu
{
   // @API [tag: construction]
public:
   menu() {}
   // copy
   menu(const menu& o) { common_construct(o); }
   menu(menu&& o) noexcept { common_construct(std::move(o)); }
   // assign
   menu& operator=(const menu& o) { common_construct(o); return *this; }
   menu& operator=(menu&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~menu() {}
private:
   // common copy
   void common_construct(const menu& o) {}
   void common_construct(menu&& o) noexcept {}

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
