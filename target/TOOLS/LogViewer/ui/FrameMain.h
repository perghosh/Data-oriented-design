#pragma once


//#include "../window.h"

#include "gdwin/control/gdwin_control_menu.h"

#include "gdwin/gdwin_window.h"

/** @CLASS [name: CFrameMain] [description:  ]
 * \brief
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

   ~CFrameMain() override;
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
   gdwin::lresult on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) override;
   std::pair<bool, std::string_view> get_window_class() const override { return { false, std::string_view("CFrameMain")}; }

   void Create();


protected:
// @API [tag: internal]
   void OnMouseMove(gd::argument::arguments& arguments);
   void OnMouseDown(gd::argument::arguments& arguments);
   void OnKeyDown(gd::argument::arguments& arguments);
   void OnCommand(gd::argument::arguments& arguments);

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   HMENU m_hMenu = nullptr;


// @API [tag: free-functions]
public:



};



