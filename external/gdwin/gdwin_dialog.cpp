// @FILE [tag: dialog] [summary: Implementation of the base dialog class] [type: source] [name: gdwin_dialog.cpp]

#include <array>

#include "gdwin_dialog.h"

_GDWIN_BEGIN

dialog::dialog(std::string_view stringTitle, int iClientWidth, int iClientHeight)
   : m_stringTitle(stringTitle), m_iClientWidth(iClientWidth), m_iClientHeight(iClientHeight)
{
   bind(gdwin::eWindowEvent::eCommand);
   bind(gdwin::eWindowEvent::eClose);
   bind(gdwin::eWindowEvent::eDestroy);                                        // WM_DESTROY and WM_NCDESTROY
}


/** --------------------------------------------------------------------------- create
 * @brief Creates a dialog window for the given parent window handle.
 * @param hwndParent The handle to the parent window.
 * @return A boolean indicating whether the dialog was successfully created.
 */
bool dialog::create(HWND hwndParent)
{                                                                                                  assert( hwndParent != nullptr );
   if(m_hwnd != nullptr) return true;

   m_hwndOwner = hwndParent;

   const uint32_t uStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU;
   const uint32_t uStyleEx = WS_EX_DLGMODALFRAME;

   RECT rect = { 0, 0, m_iClientWidth, m_iClientHeight };
   ::AdjustWindowRectEx(&rect, uStyle, FALSE, uStyleEx);                       // client size to outer window size

   std::array<std::byte, 256> arrayBuffer;
   gd::argument::arguments argumentsWindow{ arrayBuffer, { { "style", uStyle },{ "styleex", uStyleEx },{ "width", (int)(rect.right - rect.left) },{ "height", (int)(rect.bottom - rect.top) },{ "parent", (void*)hwndParent } } };
   bool bCreated = gdwin::window<>::create(m_stringTitle, argumentsWindow);
   if(bCreated == false) return false;

   if(on_initialize() == false)
   {
      ::DestroyWindow(m_hwnd);
      return false;
   }

   center();                                                                  // center dialog in window
   return true;
}

/** --------------------------------------------------------------------------- on_event
 * @brief Handles dialog window events by processing command, close, and destroy messages, and returns whether the event was handled.
 * @param eEvent The high-level window event type to process.
 * @param pargumentsParam A pointer to the event argument collection containing message-specific values such as command ID and notification code.
 * @param uNativeMessage The underlying native Windows message identifier associated with the event.
 * @return A gdwin::lresult indicating whether the event was handled and the associated result value
 */
gdwin::lresult dialog::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch(eEvent)
   {
   case gdwin::eWindowEvent::eCommand:
   {
      if(uNativeMessage != WM_COMMAND) break;                                  // eCommand also maps WM_SYSCOMMAND, leave that to default processing

      uint32_t uId = (*pargumentsParam)["id"].as_uint();
      uint32_t uNotifyCode = (*pargumentsParam)["notify_code"].as_uint();
      if(uId == IDOK) { on_ok(); return gdwin::lresult{ gdwin::lresult::enumState::eHandled, 0 }; }
      if(uId == IDCANCEL) { on_cancel(); return gdwin::lresult{ gdwin::lresult::enumState::eHandled, 0 }; }
   }
   break;

   case gdwin::eWindowEvent::eClose:
      on_cancel();                                                              // do not let default processing destroy the window, owner needs to be enabled first
      return gdwin::lresult{ gdwin::lresult::enumState::eHandled, 0 };

   case gdwin::eWindowEvent::eDestroy:
   {
      if(m_bOwnerDisabled == true)                                             // destroyed without end_dialog, give owner back
      {
         m_bOwnerDisabled = false;
         ::EnableWindow(m_hwndOwner, TRUE);
      }

      if(uNativeMessage == WM_NCDESTROY)                                       // last message, let default processing run and then forget the handle
      {
         call_default(WM_NCDESTROY, 0, 0);
         ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0);
         m_hwnd = nullptr;
         m_bModal = false;
         return gdwin::lresult{ gdwin::lresult::enumState::eHandled, 0 };
      }
   }
   break;
   }

   return gdwin::lresult();                                                    // not handled, default processing
}

intptr_t dialog::do_modal(HWND hwndParent)
{
   if(create(hwndParent) == false) return -1;

   m_iResult = IDCANCEL;
   m_bModal = true;
   if(hwndParent != nullptr && ::IsWindow(hwndParent) != FALSE && ::IsWindowEnabled(hwndParent) != FALSE)
   {
      ::EnableWindow(hwndParent, FALSE);
      m_bOwnerDisabled = true;
   }

   show(SW_SHOW);

   MSG msg;
   while(m_bModal == true)
   {
      BOOL bResult = ::GetMessageW(&msg, nullptr, 0, 0);
      if(bResult == 0)                                                         // WM_QUIT, pass it on to the outer loop and leave
      {
         ::PostQuitMessage((int)msg.wParam);
         break;
      }
      if(bResult == -1) break;

      if(m_hwnd == nullptr || ::IsDialogMessageW(m_hwnd, &msg) == FALSE)
      {
         ::TranslateMessage(&msg);
         ::DispatchMessageW(&msg);
      }
   }

   m_bModal = false;
   if(m_hwnd != nullptr) { ::DestroyWindow(m_hwnd); }                          // leaving because of WM_QUIT or error, window is still alive
   return m_iResult;
}

void dialog::end_dialog(intptr_t iResult)
{
   m_iResult = iResult;
   m_bModal = false;

   if(m_bOwnerDisabled == true)                                                // enable owner before destroy so activation goes back to it
   {
      m_bOwnerDisabled = false;
      ::EnableWindow(m_hwndOwner, TRUE);
   }

   if(m_hwnd != nullptr) { ::DestroyWindow(m_hwnd); }
   ::PostMessageW(nullptr, WM_NULL, 0, 0);                                     // wake up the modal loop if it is waiting in GetMessage
}

void dialog::center()
{
   if(m_hwnd == nullptr) return;

   RECT rectDialog;
   ::GetWindowRect(m_hwnd, &rectDialog);

   RECT rectArea;
   if(m_hwndOwner != nullptr && ::IsWindow(m_hwndOwner) != FALSE) { ::GetWindowRect(m_hwndOwner, &rectArea); }
   else { ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &rectArea, 0); }

   int iX = rectArea.left + ((rectArea.right - rectArea.left) - (rectDialog.right - rectDialog.left)) / 2;
   int iY = rectArea.top + ((rectArea.bottom - rectArea.top) - (rectDialog.bottom - rectDialog.top)) / 2;
   ::SetWindowPos(m_hwnd, nullptr, iX, iY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

HWND dialog::add_control(gd::argument::arguments& argumentsControl)
{
   std::string stringClass = argumentsControl["class"].as_string();
   std::string stringText = argumentsControl["text"].as_string();
   uint32_t uStyle = argumentsControl["style"].as_uint();
   int iX = argumentsControl["x"].as_int();
   int iY = argumentsControl["y"].as_int();
   int iWidth = argumentsControl["width"].as_int();
   int iHeight = argumentsControl["height"].as_int();
   uint32_t uId = argumentsControl["id"].as_uint();

   return add_control_(stringClass.c_str(), stringText.c_str(), uStyle, iX, iY, iWidth, iHeight, uId);
}

HWND dialog::add_control_(const char* pbszClass, const char* pbszText, uint32_t uStyle, int iX, int iY, int iWidth, int iHeight, uint32_t uId)
{
   auto hmodule_ = ::GetModuleHandleA(nullptr);
   HWND hwnd = ::CreateWindowExA(0, pbszClass, pbszText, WS_CHILD | WS_VISIBLE | uStyle, iX, iY, iWidth, iHeight, m_hwnd, (HMENU)(UINT_PTR)uId, hmodule_, nullptr);
   if(hwnd != nullptr) { ::SendMessageW(hwnd, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), TRUE); }
   return hwnd;
}

_GDWIN_END
