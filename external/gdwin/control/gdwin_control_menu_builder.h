// @FILE [tag: menu, builder, windows] [description: Fluent builder that wraps gdwin::menu to build menu trees] [type: header] [name: gdwin_control_menu_builder.h]

/**
 * \file gdwin_control_menu_builder.h
 *
 * \brief Fluent builder for gdwin::menu
 *
 * The builder wraps a menu object (it does not own it) and keeps track of the current parent and last added item.
 * `sub` starts a submenu and everything added after is placed in that submenu until `end` is called.
 * Modifiers (`checked`, `disabled`, `tip` ...) are applied to the last added item.
 *
 \code
gdwin::menu menuMain;
gdwin::menu_g( menuMain )
   .sub( "&File" )
      .item( "&Open", ID_FILE_OPEN ).key( "Ctrl+O" ).tip( "Open a file" )
      .item( "&Save", ID_FILE_SAVE ).key( "Ctrl+S" ).disabled()
      .sep()
      .sub( "&Recent" )
         .item( "first.txt", ID_RECENT_1 )
      .end()
      .sep()
      .item( "E&xit", ID_FILE_EXIT )
   .end()
   .sub( "&View" )
      .item( "&Status bar", ID_VIEW_STATUS ).checked()
      .item( "&Compact", ID_VIEW_COMPACT ).radio().checked()
   .end();

menuMain.attach( hwndMain );
 \endcode
 *
| Area               | menu_builder Methods (Examples)                              | Description                                                   |
|--------------------|--------------------------------------------------------------|---------------------------------------------------------------|
| Construction       | `menu_g( menu_ )`                                            | Wrap menu object.                                             |
| Add                | `item("name", cmd)`, `sub("name")`, `sep()`, `add(arguments)` | Add normal item, submenu (becomes current parent), separator or any item described by arguments. |
| Navigate           | `end()`, `root()`                                            | Leave current submenu, or go back to root level.              |
| Modifiers          | `checked()`, `disabled()`, `radio()`, `default_()`           | State for last added item (pass false to clear).              |
| | `right()`, `brk()`, `flag("name", true)`                     | Right justify, new column, or any bool value by name.         |
| | `tip("text")`, `key("Ctrl+O")`, `name("text")`               | Tooltip, shortcut text and item name.                         |
 */

#pragma once

#include <cassert>
#include <string_view>
#include <utility>

#include "gdwin_control_menu.h"

_GDWIN_BEGIN

/** ==========================================================================
 * @brief Fluent builder for menu trees
 *
 * Methods are ref-qualified (`&` and `&&`) so the builder works both as a named object and as a temporary
 */
struct menu_builder
{
   using tag_name = menu::tag_name;
   using tag_flag = menu::tag_flag;

   explicit menu_builder( menu& menu_ ) : m_pmenu( &menu_ ) {}

   // @API [tag: operator] [summary: simplify use of wrapped menu]
   operator menu& () { return *m_pmenu; }
   operator const menu& () const { return *m_pmenu; }

   // @API [tag: getter] [summary: get wrapped menu and state]
   [[nodiscard]] menu& get() noexcept { return *m_pmenu; }
   [[nodiscard]] size_t get_parent() const noexcept { return m_iParent; }       ///< current parent, npos for root
   [[nodiscard]] size_t get_last() const noexcept { return m_iLast; }           ///< last added item, npos if nothing added

   // @API [tag: add] [summary: add items]
   menu_builder& item( std::string_view stringName, uint32_t uCommand )& { item_( stringName, uCommand ); return *this; }
   menu_builder&& item( std::string_view stringName, uint32_t uCommand )&& { item_( stringName, uCommand ); return std::move( *this ); }

   menu_builder& sub( std::string_view stringName )& { sub_( stringName ); return *this; }
   menu_builder&& sub( std::string_view stringName )&& { sub_( stringName ); return std::move( *this ); }

   menu_builder& sep()& { sep_(); return *this; }
   menu_builder&& sep()&& { sep_(); return std::move( *this ); }

   menu_builder& add( gd::argument::arguments argumentsItem )& { add_( std::move( argumentsItem ) ); return *this; }
   menu_builder&& add( gd::argument::arguments argumentsItem )&& { add_( std::move( argumentsItem ) ); return std::move( *this ); }

   // @API [tag: navigate] [summary: move between levels]
   /// Leave current submenu, last item is the submenu so modifiers like `disabled()` after `end()` are applied to it
   menu_builder& end()& { end_(); return *this; }
   menu_builder&& end()&& { end_(); return std::move( *this ); }

   /// Go back to root level
   menu_builder& root()& { m_iParent = menu::npos; return *this; }
   menu_builder&& root()&& { m_iParent = menu::npos; return std::move( *this ); }

   // @API [tag: attribute] [summary: modify last added item]
   menu_builder& checked( bool bValue = true )& { m_pmenu->set( last_(), bValue, menu::tag_checked{} ); return *this; }
   menu_builder&& checked( bool bValue = true )&& { m_pmenu->set( last_(), bValue, menu::tag_checked{} ); return std::move( *this ); }

   menu_builder& disabled( bool bValue = true )& { m_pmenu->set( last_(), bValue, menu::tag_disabled{} ); return *this; }
   menu_builder&& disabled( bool bValue = true )&& { m_pmenu->set( last_(), bValue, menu::tag_disabled{} ); return std::move( *this ); }

   menu_builder& default_( bool bValue = true )& { m_pmenu->set( last_(), bValue, menu::tag_default{} ); return *this; }
   menu_builder&& default_( bool bValue = true )&& { m_pmenu->set( last_(), bValue, menu::tag_default{} ); return std::move( *this ); }

   menu_builder& radio( bool bValue = true )& { m_pmenu->set( last_(), bValue, menu::tag_radio{} ); return *this; }
   menu_builder&& radio( bool bValue = true )&& { m_pmenu->set( last_(), bValue, menu::tag_radio{} ); return std::move( *this ); }

   menu_builder& right( bool bValue = true )& { m_pmenu->set( last_(), "rightjustify", bValue, tag_flag{} ); return *this; }
   menu_builder&& right( bool bValue = true )&& { m_pmenu->set( last_(), "rightjustify", bValue, tag_flag{} ); return std::move( *this ); }

   /// New column for this item, `bLine` adds separating line between columns
   menu_builder& brk( bool bLine = false )& { m_pmenu->set( last_(), bLine ? "barbreak" : "break", true, tag_flag{} ); return *this; }
   menu_builder&& brk( bool bLine = false )&& { m_pmenu->set( last_(), bLine ? "barbreak" : "break", true, tag_flag{} ); return std::move( *this ); }

   menu_builder& flag( std::string_view stringName, bool bValue = true )& { m_pmenu->set( last_(), stringName, bValue, tag_flag{} ); return *this; }
   menu_builder&& flag( std::string_view stringName, bool bValue = true )&& { m_pmenu->set( last_(), stringName, bValue, tag_flag{} ); return std::move( *this ); }

   menu_builder& name( std::string_view stringText )& { m_pmenu->set( last_(), stringText, menu::tag_name{} ); return *this; }
   menu_builder&& name( std::string_view stringText )&& { m_pmenu->set( last_(), stringText, menu::tag_name{} ); return std::move( *this ); }

   menu_builder& tip( std::string_view stringText )& { m_pmenu->set( last_(), stringText, menu::tag_tooltip{} ); return *this; }
   menu_builder&& tip( std::string_view stringText )&& { m_pmenu->set( last_(), stringText, menu::tag_tooltip{} ); return std::move( *this ); }

   menu_builder& key( std::string_view stringText )& { m_pmenu->set( last_(), stringText, menu::tag_shortcut{} ); return *this; }
   menu_builder&& key( std::string_view stringText )&& { m_pmenu->set( last_(), stringText, menu::tag_shortcut{} ); return std::move( *this ); }

// @API [tag: internal]
private:
   void item_( std::string_view stringName, uint32_t uCommand ) { m_iLast = m_pmenu->add( m_iParent, stringName, uCommand ); assert( m_iLast != menu::npos ); }
   void sub_( std::string_view stringName ) { m_iLast = m_pmenu->add( m_iParent, stringName, menu::tag_submenu{} ); assert( m_iLast != menu::npos ); m_iParent = m_iLast; }
   void sep_() { m_iLast = m_pmenu->add( m_iParent, menu::tag_separator{} ); assert( m_iLast != menu::npos ); }
   void add_( gd::argument::arguments argumentsItem ) { m_iLast = m_pmenu->add( m_iParent, std::move( argumentsItem ) ); assert( m_iLast != menu::npos ); }
   void end_() { assert( m_iParent != menu::npos && "end() without matching sub()" ); if( m_iParent == menu::npos ) return; m_iLast = m_iParent; m_iParent = m_pmenu->parent( m_iParent ); }
   size_t last_() const { assert( m_iLast != menu::npos && "no item to modify" ); return m_iLast; }

public:
   menu* m_pmenu;                                                              ///< wrapped menu, not owned
   size_t m_iParent = menu::npos;                                              ///< current parent where new items are added
   size_t m_iLast = menu::npos;                                                ///< last added item, modifiers are applied to this
};

/// global method to create menu builder: menu_g( menuMain ).sub( "&File" ).item( "&Open", ID_OPEN ).end();
inline menu_builder menu_g( menu& menu_ ) { return menu_builder{ menu_ }; }

_GDWIN_END
