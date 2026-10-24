// @FILE [tag: menu, windows, control] [description: Meta data for a complete windows menu, stored as a tree and able to generate HMENU] [type: header] [name: gdwin_control_menu.h]

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif


#include <bitset>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include <windows.h>
#include <windowsx.h>

#include "gd/gd_types.h"
#include "gd/gd_arguments.h"

#include "../gdwin_event.h"

#ifndef _GDWIN_BEGIN
#define _GDWIN_BEGIN namespace gdwin {
#define _GDWIN_END }
#endif

_GDWIN_BEGIN


/** @CLASS [name: menu] [description: Meta data for a complete windows menu stored as a tree, able to generate HMENU from the data ]
 * \brief Holds a menu as a flat vector of items where each item knows its parent (by index).
 *
 * Items are stored in the order they are added and a parent is always stored before its children.
 * Root items have parent `npos`. Index is used (not pointers) to reference items because indexes stay valid
 * when the vector grows. Indexes are only changed by `remove` (items after the removed ones are shifted).
 *
 * Methods that do the same thing for different "things" share name and are selected with tag dispatchers,
 * `add`, `find`, `set` and `create` are the main ones.
 *
 * Each item keeps its information in `gd::argument::arguments`. Known names:
 *
 * | name         | type   | description                                                                |
 * |--------------|--------|----------------------------------------------------------------------------|
 * | name         | string | text for item (UTF-8), use & for mnemonic                                   |
 * | command      | uint32 | command id sent in WM_COMMAND                                               |
 * | tooltip      | string | help text, windows menus have no tooltip so this is for status bar etc      |
 * | shortcut     | string | accelerator text shown to the right ("Ctrl+O"), only text, no real accelerator |
 * | type         | string | "item" (default), "separator" or "submenu"                                  |
 * | checked      | bool   | item is checked                                                             |
 * | disabled     | bool   | item is disabled (grayed)                                                   |
 * | default      | bool   | item is default (bold)                                                      |
 * | radio        | bool   | checked is shown as radio bullet instead of check mark                      |
 * | rightjustify | bool   | right align item (menu bar)                                                 |
 * | break        | bool   | start new column (MFT_MENUBREAK)                                            |
 * | barbreak     | bool   | start new column with separating line (MFT_MENUBARBREAK)                    |
 *
 * An item that has children is always created as a submenu, type "submenu" can be used to create an empty submenu.
 *
 \code
gdwin::menu menuMain;
using M = gdwin::menu;
auto iFile = menuMain.add( M::npos, "&File", M::tag_submenu{} );
auto iOpen = menuMain.add( iFile, "&Open", ID_FILE_OPEN );
menuMain.add( iFile, M::tag_separator{} );
menuMain.add( iFile, "E&xit", ID_FILE_EXIT );
menuMain.set( iOpen, true, M::tag_checked{} );

auto result_ = menuMain.attach( hwndMain );                                   // create menu bar and set to window
if( !result_ ) { DWORD uError = result_.error(); }

uint32_t uCommand = menuMain.popup( hwndMain, x, y );                         // or show as context menu
 \endcode
 *
 * For readable code to build menus see `menu_builder` in gdwin_control_menu_builder.h
 */
class menu
{
// ## typedefs and tag dispatchers -----------------------------------------------
public:
   using tag_name       = gd::types::tag_name;                                 ///< name for item
   using tag_command    = gd::types::tag_command;                              ///< command id for item
   using tag_flag       = gd::types::tag_flag;                                 ///< flag by name (any bool value in arguments)
   using tag_debug      = gd::types::tag_debug;                                ///< debug related logic

   struct tag_separator {};                                                    ///< separator item
   struct tag_submenu {};                                                      ///< submenu item
   struct tag_bar {};                                                          ///< menu bar (top menu in window)
   struct tag_popup {};                                                        ///< popup menu (context menu)
   struct tag_checked {};                                                      ///< checked state
   struct tag_disabled {};                                                     ///< disabled state
   struct tag_default {};                                                      ///< default item (bold)
   struct tag_radio {};                                                        ///< radio check
   struct tag_tooltip {};                                                      ///< tooltip text
   struct tag_shortcut {};                                                     ///< shortcut text

   static constexpr size_t npos = static_cast<size_t>(-1);                     ///< "no item", used as parent for root items

   enum enumType : uint8_t { eTypeItem = 0, eTypeSeparator = 1, eTypeSubmenu = 2 };

   struct item
   {
      item() = default;
      item( size_t iParent, gd::argument::arguments argumentsItem ) : m_iParent( iParent ), m_arguments( std::move( argumentsItem ) ) {}

      std::string name() const { return get_string( "name" ); }
      uint32_t command() const { auto a_ = m_arguments.get_argument( "command" ); return a_.is_null() ? 0u : a_.get_uint(); }
      std::string tooltip() const { return get_string( "tooltip" ); }
      std::string shortcut() const { return get_string( "shortcut" ); }

      enumType type() const;
      bool is_separator() const { return type() == eTypeSeparator; }
      bool is_submenu() const { return type() == eTypeSubmenu; }              ///< only checks type, use menu::is_submenu to also take children into account

      bool flag( std::string_view stringName ) const { auto a_ = m_arguments.get_argument( stringName ); return a_.is_null() == false && a_.get_bool(); }
      bool is_checked() const { return flag( "checked" ); }
      bool is_disabled() const { return flag( "disabled" ); }
      bool is_default() const { return flag( "default" ); }
      bool is_radio() const { return flag( "radio" ); }

      std::string get_string( std::string_view stringName ) const { auto a_ = m_arguments.get_argument( stringName ); return a_.is_null() ? std::string() : a_.get_string(); }

      size_t m_iParent = npos;                                                 ///< index to parent item, npos for root items
      gd::argument::arguments m_arguments;
   };

// @API [tag: construction]
public:
   menu() {}
   // copy (copies the item data, not any windows handle)
   menu(const menu& o) { common_construct(o); }
   menu(menu&& o) noexcept { common_construct(std::move(o)); }
   // assign
   menu& operator=(const menu& o) { if( this != &o ) { destroy(); common_construct(o); } return *this; }
   menu& operator=(menu&& o) noexcept { if( this != &o ) { destroy(); common_construct(std::move(o)); } return *this; }

   ~menu() { destroy(); }
private:
   // common copy
   void common_construct(const menu& o) { m_vectorItem = o.m_vectorItem; }
   void common_construct(menu&& o) noexcept {
      m_vectorItem = std::move(o.m_vectorItem);
      m_hwnd = o.m_hwnd; m_hmenu = o.m_hmenu; m_bOwner = o.m_bOwner;
      o.m_hwnd = nullptr; o.m_hmenu = nullptr; o.m_bOwner = false;
   }

// @API [tag: operator]
public:
   item& operator[](size_t iItem) { return m_vectorItem[iItem]; }
   const item& operator[](size_t iItem) const { return m_vectorItem[iItem]; }

// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]
   size_t size() const { return m_vectorItem.size(); }
   bool empty() const { return m_vectorItem.empty(); }
   bool valid( size_t iItem ) const { return iItem < m_vectorItem.size(); }
   item& at( size_t iItem ) { return m_vectorItem.at( iItem ); }
   const item& at( size_t iItem ) const { return m_vectorItem.at( iItem ); }

   size_t parent( size_t iItem ) const { return m_vectorItem.at( iItem ).m_iParent; }
   std::vector<size_t> children( size_t iParent ) const;                       ///< direct children (index) in order
   bool has_children( size_t iParent ) const;
   bool is_submenu( size_t iItem ) const;                                      ///< type is submenu or item has children
   size_t depth( size_t iItem ) const;                                         ///< 0 for root items

   /// find item, returns npos if not found
   size_t find( uint32_t uCommand, tag_command ) const;                        ///< first item with command
   size_t find( size_t iParent, std::string_view stringName, tag_name ) const; ///< direct child with name

   /// change item, if menu is created the live menu is updated for checked and disabled
   void set( size_t iItem, bool bValue, tag_checked ) { set( iItem, "checked", bValue, tag_flag{} ); }
   void set( size_t iItem, bool bValue, tag_disabled ) { set( iItem, "disabled", bValue, tag_flag{} ); }
   void set( size_t iItem, bool bValue, tag_default ) { set( iItem, "default", bValue, tag_flag{} ); }
   void set( size_t iItem, bool bValue, tag_radio ) { set( iItem, "radio", bValue, tag_flag{} ); }
   void set( size_t iItem, std::string_view stringName, bool bValue, tag_flag );///< any bool value
   void set( size_t iItem, std::string_view stringText, tag_name ) { text_set_( iItem, "name", stringText ); }
   void set( size_t iItem, std::string_view stringText, tag_tooltip ) { text_set_( iItem, "tooltip", stringText ); }
   void set( size_t iItem, std::string_view stringText, tag_shortcut ) { text_set_( iItem, "shortcut", stringText ); }

// @API [tag: operation]
   /// add item, returns index to new item or npos if parent is invalid
   size_t add( size_t iParent, gd::argument::arguments argumentsItem );        ///< add item described by arguments
   size_t add( size_t iParent, std::string_view stringName, uint32_t uCommand );///< add normal item
   size_t add( size_t iParent, tag_separator );                                ///< add separator
   size_t add( size_t iParent, std::string_view stringName, tag_submenu );     ///< add submenu

   bool remove( size_t iItem );                                                ///< remove item and all its children, indexes after removed items are changed
   void clear() { destroy(); m_vectorItem.clear(); }                           ///< remove all items

   /// create windows menu from data, menu is owned by this object until it is attached to a window
   std::expected<HMENU, DWORD> create( tag_bar ) { return create_( false ); }  ///< create menu bar
   std::expected<HMENU, DWORD> create( tag_popup ) { return create_( true ); } ///< create popup menu
   /// create menu bar and set it to window, window owns menu after this
   std::expected<HMENU, DWORD> attach( HWND hwnd );
   /// show as context menu, returns selected command or 0 if canceled
   uint32_t popup( HWND hwnd, int iX, int iY, UINT uFlags = TPM_RIGHTBUTTON );
   /// destroy created menu (if owned by this object)
   void destroy();

   HMENU hmenu() const { return m_hmenu; }

protected:
// @API [tag: internal]
   void text_set_( size_t iItem, std::string_view stringName, std::string_view stringText );
   std::expected<HMENU, DWORD> create_( bool bPopup );
   std::expected<HMENU, DWORD> build_( size_t iParent, bool bPopup ) const;    ///< create popup or menu bar for children of iParent
   void sync_( size_t iItem ) const;                                           ///< update check/enable state in live menu

public:
// @API [tag: debug]
   std::string to_string( tag_debug ) const;                                   ///< item tree as text

// ## attributes ----------------------------------------------------------------
public:
   HWND m_hwnd = nullptr;                                                      ///< window menu is attached to
   HMENU m_hmenu = nullptr;                                                    ///< created menu (root)
   bool m_bOwner = false;                                                      ///< true if this object is responsible to destroy m_hmenu
   std::vector<item> m_vectorItem;

// @API [tag: free-functions]
public:



};



_GDWIN_END