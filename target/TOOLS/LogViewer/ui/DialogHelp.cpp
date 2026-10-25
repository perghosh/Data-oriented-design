#include <array>

#include "DialogHelp.h"

bool CDialogHelp::on_initialize()
{
   std::array<std::byte, 256> array_;
   gd::argument::arguments argumentsControl(array_);

   argumentsControl.append({ { "class", "STATIC" }, { "text", "Opening logs\r\n  File -> Open\r\n\nSaving logs\r\n  File -> Save\r\n  File -> Save as" }, { "style", 0 }, { "x", 24 }, { "y", 24 }, { "width", 390 }, { "height", 190 }, { "id", 0 } });
   HWND hwndText = add_control(argumentsControl);
   argumentsControl.clear();
   argumentsControl.append({ { "class", "BUTTON" }, { "text", "OK" }, { "style", int(WS_TABSTOP | BS_DEFPUSHBUTTON) }, { "x", 170 }, { "y", 240 }, { "width", 100 }, { "height", 28 }, { "id", IDOK } });
   HWND hwndOk = add_control(argumentsControl);
   if( hwndText == nullptr || hwndOk == nullptr ) return false;

   ::SetFocus(hwndOk);
   return true;
}
