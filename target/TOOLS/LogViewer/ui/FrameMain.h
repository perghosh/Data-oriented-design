#pragma once


//#include "../window.h"

#include "gdwin/gdwin_window.h"

/** @CLASS [name: CFrameMain] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CFrameMain : public gdwin::window<>
{
   // @API [tag: construction]
public:
   CFrameMain();
   // copy
   CFrameMain(const CFrameMain& o) { common_construct(o); }
   CFrameMain(CFrameMain&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFrameMain& operator=(const CFrameMain& o) { common_construct(o); return *this; }
   CFrameMain& operator=(CFrameMain&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFrameMain() override = default;
private:
   // common copy
   void common_construct(const CFrameMain& o) {}
   void common_construct(CFrameMain&& o) noexcept {}

   // @API [tag: operator]
public:


   // ## methods ------------------------------------------------------------------
public:
   // @API [tag: get, set]

   // @API [tag: operation]


protected:
   // @API [tag: internal]
   void OnMouseDown(UINT, POINT point);
   void OnMouseMove(UINT, POINT point);

public:
   // @API [tag: debug]

   // ## attributes ----------------------------------------------------------------
public:


   // @API [tag: free-functions]
public:



};



