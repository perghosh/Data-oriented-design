// @FILE [tag: application, window, dialog] [summary: About dialog] [name: DialogAbout.h] [type: header]

#pragma once

#include "gdwin/gdwin_dialog.h"

/** @CLASS [name: CDialogAbout] [description: Modal about dialog for LogViewer]
 * \brief Shows application information, closed with OK, Enter or Esc.
 *
 \code
 CDialogAbout dialogAbout;
 dialogAbout.do_modal(m_hwnd);
 \endcode
 */
class CDialogAbout : public gdwin::dialog
{
public:
   CDialogAbout() : gdwin::dialog("About LogViewer", 440, 200) {}
   ~CDialogAbout() override = default;

   std::pair<bool, std::string_view> get_window_class() const override { return { false, std::string_view("CDialogAbout") }; }

protected:
   bool on_initialize() override;
};
