#include <array>

#include "DialogAbout.h"

bool CDialogAbout::on_init()
{
   std::array<std::byte, 256> array_;
   gd::argument::arguments argumentsControl(array_);

   argumentsControl.append({{ "class", "STATIC" }, { "text", "LogViewer\r\nVersion 1.0\r\nBuilt with gd + gdwin" }, { "style", 0 }, { "x", 24 }, { "y", 24 }, { "width", 390 }, { "height", 90 }, { "id", 0 }});
   HWND hwndText = add_control(argumentsControl);
   argumentsControl.clear();
   argumentsControl.append({ { "class", "BUTTON" },{ "text", "OK" },{ "style", int(WS_TABSTOP | BS_DEFPUSHBUTTON) },{ "x", 170 },{ "y", 140 },{ "width", 100 },{ "height", 28 },{ "id", IDOK } });
   HWND hwndOk = add_control(argumentsControl);
   if(hwndText == nullptr || hwndOk == nullptr) return false;

   ::SetFocus(hwndOk);
   return true;
}
