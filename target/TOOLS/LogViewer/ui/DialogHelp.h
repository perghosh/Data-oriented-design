// @FILE [tag: application, window, dialog] [summary: Help dialog] [name: DialogHelp.h] [type: header]

#pragma once

#include "gdwin/gdwin_dialog.h"

/** @CLASS [name: DialogHelp] [description: Modal help dialog for LogViewer]
 * \brief Shows application information, closed with OK, Enter or Esc.
 *
 \code
 CDialogHelp dialogHelp;
 dialogHelp.do_modal(m_hwnd);
 \endcode
 */
class CDialogHelp: public gdwin::dialog
{
public:
   CDialogHelp(): gdwin::dialog("Help LogViewer", 440, 300) {}
   ~CDialogHelp() override = default;

   std::pair<bool, std::string_view> get_window_class() const override { return { false, std::string_view("CDialogHelp") }; }

protected:
   bool on_initialize() override;
};
