#include <string>

#include "DialogAbout.h"
#include "DialogHelp.h"

#include "FrameMain.h"
#include <commctrl.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


enum enumMenu {
   /* file */
   eOpen = 1000,
   eSave,
   eSaveAs,
   eExit,

   /* view */
   eRefresh = 2000,
   eClear,
   eShowLineNumbers,

   /* search */
   eFind = 3000,

   /* tools */
   eFilter = 4000,

   /* help */
   eAbout = 5000,
   eHelp
};

/// Child control ids, needed to find controls and to recognize them in notifications
enum enumControl {
   eControlRebar = 100,
   eControlStatusBar,
};

/// Height of window in pixels, 0 if there is no window
static int GetWindowHeight_s(HWND hwnd)
{
   RECT rectWindow;
   if(hwnd == nullptr || ::GetWindowRect(hwnd, &rectWindow) == FALSE) return 0;
   return rectWindow.bottom - rectWindow.top;
}


CFrameMain::CFrameMain()
{
   bind(gdwin::eWindowEvent::eCommand);
   bind(gdwin::eWindowEvent::eNotification);                                   // rebar sends RBN_HEIGHTCHANGE through WM_NOTIFY
   bind(gdwin::eWindowEvent::eResize);
   bind(gdwin::eWindowEvent::eKeyDown);                                        // bind the key down event to this window (WM_KEYDOWN)
   bind(gdwin::eWindowEvent::eMouseDown);                                      // bind the mouse down event to this window (WM_LBUTTONDOWN)
   //bind(gdwin::eWindowEvent::eMouseMove);                                      // bind the mouse move event to this window (WM_MOUSEMOVE)
   bind(gdwin::eWindowEvent::eClose);                                          // bind the close event to this window (WM_CLOSE)
   bind(gdwin::eWindowEvent::eDestroy);                                        // bind the destroy event to this window (WM_DESTROY)
}

CFrameMain::~CFrameMain()
{
   if(m_hMenu != nullptr)
   {
      if(m_hwnd != nullptr && ::GetMenu(m_hwnd) == m_hMenu) { ::SetMenu(m_hwnd, nullptr); }
      ::DestroyMenu(m_hMenu);
      m_hMenu = nullptr;
   }
}

gdwin::lresult CFrameMain::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch(eEvent)
   {
   case gdwin::eWindowEvent::eMouseMove: { OnMouseMove(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eMouseDown: { OnMouseDown(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eKeyDown: { OnKeyDown(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eCommand:
      if(uNativeMessage == WM_COMMAND) { OnCommand(*pargumentsParam); }       // eCommand also maps WM_SYSCOMMAND, leave that to default processing
      break;
   case gdwin::eWindowEvent::eNotification: { OnNotify(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eResize:
      if(uNativeMessage == WM_SIZE) { Layout(); }                              // eResize also maps WM_SIZING where lparam is a RECT pointer, not width and height
      break;
   case gdwin::eWindowEvent::eClose:
      ::DestroyWindow(m_hwnd);
      return gdwin::lresult{ gdwin::lresult::enumState::eHandled, 0 };         // window is gone, do not let default processing close it again
   case gdwin::eWindowEvent::eDestroy:
      if(uNativeMessage == WM_DESTROY)                                         // eDestroy also maps WM_NCDESTROY, only quit once
      {
         m_hMenu = nullptr;                                                    // menu is destroyed together with the window
         m_hwndRebar = nullptr;
         m_hwndStatusBar = nullptr;
         ::PostQuitMessage(0);
      }
      break;
   }

   return gdwin::lresult();
}

/** ---------------------------------------------------------------------------
 * @brief Creates menu, main window, rebar and status bar
 * @return true if everything was created, false if something failed. A window that was created is destroyed again
 */
bool CFrameMain::Create()
{
   if(InitializeControls() == false) return false;
   if(CreateMenuBar() == false) return false;
   if(CreateMainWindow() == false) return false;

   if(CreateRebar() == false || CreateStatusBar() == false)
   {
      ::DestroyWindow(m_hwnd);                                                 // do not leave a half built window behind
      return false;
   }

   SetStatusText("Ready");
   Layout();
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Positions rebar and status bar and calculates the area that is left for the main view
 * @note Rebar docks to the top (CCS_TOP) and status bar to the bottom (CCS_BOTTOM), both place themselves when they get WM_SIZE
 */
void CFrameMain::Layout()
{
   if(m_hwnd == nullptr) return;

   RECT rectClient;
   ::GetClientRect(m_hwnd, &rectClient);

   if(m_hwndRebar != nullptr) { ::SendMessageW(m_hwndRebar, WM_SIZE, 0, 0); }
   if(m_hwndStatusBar != nullptr) { ::SendMessageW(m_hwndStatusBar, WM_SIZE, 0, 0); }

   int iTop = rectClient.top + GetWindowHeight_s(m_hwndRebar);
   int iBottom = rectClient.bottom - GetWindowHeight_s(m_hwndStatusBar);
   if(iBottom < iTop) { iBottom = iTop; }                                      // window is smaller than the bars, keep rectangle valid

   m_rectClient = { rectClient.left, iTop, rectClient.right, iBottom };
}

/** ---------------------------------------------------------------------------
 * @brief Initializes the common control classes used by the frame
 */
bool CFrameMain::InitializeControls()
{
   INITCOMMONCONTROLSEX initCommonControls_{};
   initCommonControls_.dwSize = sizeof(initCommonControls_);
   initCommonControls_.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;             // status bar and rebar
   if(::InitCommonControlsEx(&initCommonControls_) == FALSE)
   {
      LogError("Could not initialize common controls.");
      return false;
   }
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Creates the menu, it is owned by this object until the window takes it over
 */
bool CFrameMain::CreateMenuBar()
{
   HMENU hMenu = ::CreateMenu();
   HMENU hSubMenu = nullptr;
   if(hMenu == nullptr) { LogError("Could not create the menu."); return false; }

   /* Create file menu */
   hSubMenu = ::CreateMenu();
   ::AppendMenuW(hSubMenu, MF_STRING, eOpen, L"Open");
   ::AppendMenuW(hSubMenu, MF_STRING, eSave, L"Save");
   ::AppendMenuW(hSubMenu, MF_STRING, eSaveAs, L"Save as");
   ::AppendMenuW(hSubMenu, MF_SEPARATOR, 0, nullptr);
   ::AppendMenuW(hSubMenu, MF_STRING, eExit, L"Exit");
   ::AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"File");

   /* Create view menu */
   hSubMenu = ::CreateMenu();
   ::AppendMenuW(hSubMenu, MF_STRING, eRefresh, L"Refresh");
   ::AppendMenuW(hSubMenu, MF_STRING, eClear, L"Clear");
   ::AppendMenuW(hSubMenu, MF_STRING | MF_CHECKED, eShowLineNumbers, L"Show line numbers");
   ::AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"View");

   /* Create search menu */
   hSubMenu = ::CreateMenu();
   ::AppendMenuW(hSubMenu, MF_STRING, eFind, L"Find");
   ::AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Search");

   /* Create tools menu */
   hSubMenu = ::CreateMenu();
   ::AppendMenuW(hSubMenu, MF_STRING, eFilter, L"Filter");
   ::AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Tools");

   /* Create help menu */
   hSubMenu = ::CreateMenu();
   ::AppendMenuW(hSubMenu, MF_STRING, eAbout, L"About");
   ::AppendMenuW(hSubMenu, MF_STRING, eHelp, L"Help");
   ::AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Help");

   m_hMenu = hMenu;
   return true;
}

bool CFrameMain::CreateMainWindow()
{
   bool bCreated = gdwin::window<>::create("Blank Window", { { "style", "overlappedwindow" },{ "width", 800 },{ "height", 600 },{ "callback", gdwin::window<>::get_callback_s() },{ "menu", m_hMenu } });
   if(bCreated == false)
   {
      LogError("Could not create the main window.");
      return false;
   }
   return true;
}

bool CFrameMain::CreateRebar()
{
   m_hwndRebar = CreateChild(REBARCLASSNAMEW, L"", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE, eControlRebar);
   if(m_hwndRebar == nullptr) return false;

   // placeholder content for the band, replace with a toolbar (TOOLBARCLASSNAMEW) when there are commands to show
   HWND hwndRebarBandContent = CreateChild(L"STATIC", L"  Main", SS_CENTERIMAGE, 0, m_hwndRebar);
   if(hwndRebarBandContent == nullptr) return false;
   ::SendMessageW(hwndRebarBandContent, WM_SETFONT, reinterpret_cast<WPARAM>(::GetStockObject(DEFAULT_GUI_FONT)), TRUE);

   return InsertRebarBand(hwndRebarBandContent, L"Tools", 100, 28, 160);
}

bool CFrameMain::CreateStatusBar()
{
   m_hwndStatusBar = CreateChild(STATUSCLASSNAMEW, L"", SBARS_SIZEGRIP | CCS_BOTTOM, eControlStatusBar);
   return m_hwndStatusBar != nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Creates a child control, size and position are set later by Layout or by the parent control (rebar)
 * @param pwszClass Window class name
 * @param pwszText Initial text
 * @param uStyle Style flags, WS_CHILD and WS_VISIBLE are added
 * @param iId Control id, 0 if the control has no id
 * @param hwndParent Parent window, the frame window if null
 * @return Handle to the control or nullptr if it failed
 */
HWND CFrameMain::CreateChild(const wchar_t* pwszClass, const wchar_t* pwszText, DWORD uStyle, int iId, HWND hwndParent)
{
   if(hwndParent == nullptr) { hwndParent = m_hwnd; }

   HWND hwnd = ::CreateWindowExW(0, pwszClass, pwszText, WS_CHILD | WS_VISIBLE | uStyle, 0, 0, 0, 0, hwndParent, (HMENU)(UINT_PTR)iId, ::GetModuleHandleW(nullptr), nullptr);
   if(hwnd == nullptr) { LogError("Could not create child control."); }
   return hwnd;
}

bool CFrameMain::InsertRebarBand(HWND hwndChild, const wchar_t* pwszTitle, int iMinWidth, int iMinHeight, int iWidth)
{
   REBARBANDINFOW rebarBandInfo{};
   rebarBandInfo.cbSize = sizeof(REBARBANDINFOW);
   rebarBandInfo.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_TEXT;   // no RBBIM_COLORS, let visual styles draw the band
   rebarBandInfo.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;
   rebarBandInfo.lpText = const_cast<LPWSTR>(pwszTitle);
   rebarBandInfo.hwndChild = hwndChild;
   rebarBandInfo.cxMinChild = iMinWidth;
   rebarBandInfo.cyMinChild = iMinHeight;
   rebarBandInfo.cx = iWidth;
   if(::SendMessageW(m_hwndRebar, RB_INSERTBANDW, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(&rebarBandInfo)) == FALSE)
   {
      // RB_INSERTBANDW does not set last-error; the usual causes are cbSize/comctl32 version mismatch or a bad child window
      std::string stringError = "Could not add the rebar band. cbSize=" + std::to_string(rebarBandInfo.cbSize) +
         ", rebar valid=" + std::to_string(::IsWindow(m_hwndRebar)) + ", child valid=" + std::to_string(::IsWindow(hwndChild)) + "\n";
      ::OutputDebugStringA(stringError.c_str());
      return false;
   }
   return true;
}

void CFrameMain::SetStatusText(const char* pszText)
{
   if(m_hwndStatusBar != nullptr) { :: SendMessage(m_hwndStatusBar, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(pszText)); }
}

void CFrameMain::ToggleMenuCheck(uint32_t uCommandId)
{
   if(m_hMenu == nullptr) return;

   UINT uState = ::GetMenuState(m_hMenu, uCommandId, MF_BYCOMMAND);
   if(uState == (UINT)-1) return;                                              // no such item
   ::CheckMenuItem(m_hMenu, uCommandId, MF_BYCOMMAND | ((uState & MF_CHECKED) ? MF_UNCHECKED : MF_CHECKED));
}

void CFrameMain::LogError(const char* pszMessage) const
{
   DWORD uError = ::GetLastError();                                            // read first, the calls below may change it
   std::string stringMessage = std::string(pszMessage) + " Error: " + std::to_string(uError) + "\n";
   ::OutputDebugStringA(stringMessage.c_str());
}

void CFrameMain::OnMouseMove(gd::argument::arguments& arguments)
{
   ::OutputDebugStringA(("Left button move at: (" + arguments["x"].as_string() + ", " + arguments["y"].as_string() + ")\n").c_str());
}


void CFrameMain::OnMouseDown(gd::argument::arguments& arguments)
{
   ::OutputDebugStringA(("Left button down at: (" + arguments["x"].as_string() + ", " + arguments["y"].as_string() + ")\n").c_str());
}

void CFrameMain::OnKeyDown(gd::argument::arguments& arguments)
{
   ::OutputDebugStringA(("Key down: " + arguments["key"].as_string() + "\n").c_str());
}


void CFrameMain::OnCommand(gd::argument::arguments& arguments)
{
   uint32_t uCommandId = arguments["id"].as_uint();
   char pszMenuText[128] = {};
   if(m_hMenu != nullptr && ::GetMenuStringA(m_hMenu, uCommandId, pszMenuText, 128, MF_BYCOMMAND) > 0)
   {
      SetStatusText(pszMenuText);                                            // text of the menu item matching the command id
   }

   if(uCommandId == eAbout)
   {
      CDialogAbout dialogAbout;
      if(dialogAbout.do_modal(m_hwnd) == -1)
      {
         ::OutputDebugStringA("About dialog failed to create.\n");
      }
   }
   else if(uCommandId == eHelp)
   {
      CDialogHelp dialogHelp;
      if(dialogHelp.do_modal(m_hwnd) == -1)
      {
         ::OutputDebugStringA("Help dialog failed to create.\n");
      }
   }
   else if(uCommandId == eShowLineNumbers) { ToggleMenuCheck(uCommandId); }
   else if(uCommandId == eExit) { ::PostMessageW(m_hwnd, WM_CLOSE, 0, 0); }

   ::OutputDebugStringA(("Command: " + arguments["id"].as_string() + "\n").c_str());
}

/** ---------------------------------------------------------------------------
 * @brief Handles WM_NOTIFY, rebar height changes when bands wrap or are resized and then the content area has to move
 */
void CFrameMain::OnNotify(gd::argument::arguments& arguments)
{
   uint32_t uCode = arguments["code"].as_uint();
   if(uCode == static_cast<uint32_t>(RBN_HEIGHTCHANGE)) { Layout(); }
}
