// @FILE [tag: application, window, dialog] [summary: Base class for dialog windows, adds modal loop and OK/Cancel handling] [name: Dialog.h] [type: header]

#pragma once

#include <string>

#include <windows.h>
#include <windowsx.h>

#include "gdwin_window.h"

#ifndef _GDWIN_BEGIN
#define _GDWIN_BEGIN namespace gdwin {
#define _GDWIN_END }
#endif

_GDWIN_BEGIN


class dialog : public gdwin::window<>
{
   // @API [tag: construction]
public:
   dialog(std::string_view stringTitle, int iClientWidth, int iClientHeight);
   dialog(const dialog &) = delete;
   dialog& operator=(const dialog&) = delete;
   ~dialog() override = default;

// @API [tag: window]
public:
   gdwin::lresult on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) override;

// @API [tag: operation]
public:
   /// Create the dialog window as owned by hwndParent, controls are created in OnInitDialog
   bool create(HWND hwndParent);
   /// Show the dialog modal, returns when the dialog is ended. Returns the value sent to EndDialog or -1 if creation failed
   intptr_t do_modal(HWND hwndParent);

   /// End the dialog, for a modal dialog this makes DoModal return iResult
   void end_dialog(intptr_t iResult);
   /// Center the dialog over its owner or over the work area if there is no owner
   void center();

   bool IsModal() const { return m_bModal; }

protected:
   // @API [tag: internal]
      /// Called when window is created, create controls here. Return false to cancel creation
   virtual bool on_init() { return true; }
   /// Called for WM_COMMAND, return true if handled. Not called for IDOK and IDCANCEL, see on_ok and on_cancel
   //virtual bool on_command(uint32_t uId, uint32_t uNotifyCode) { return false; }
   virtual void on_ok() { end_dialog(IDOK); }
   virtual void on_cancel() { end_dialog(IDCANCEL); }



   /// Create a child control and give it the standard gui font. uId is the command id sent in WM_COMMAND
   HWND add_control(gd::argument::arguments& argumentsControl );
   HWND add_control_(const char* pbszClass, const char* pbszText, uint32_t uStyle, int iX, int iY, int iWidth, int iHeight, uint32_t uId = 0);


// ## attributes ----------------------------------------------------------------
protected:
   gd::argument::arguments m_argumentsSettings; ///< arguments used to create the window
   std::string m_stringTitle;                   ///< dialog caption
   int m_iClientWidth = 0;                      ///< wanted width of client area
   int m_iClientHeight = 0;                     ///< wanted height of client area
   HWND m_hwndOwner = nullptr;                  ///< owner window, disabled while modal
   bool m_bOwnerDisabled = false;               ///< true if this dialog has disabled the owner
   bool m_bModal = false;                       ///< true while the modal loop runs
   intptr_t m_iResult = IDCANCEL;               ///< value returned from DoModal
};

_GDWIN_END
