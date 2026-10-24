#include <sstream>

#include "gdwin_control_menu.h"

_GDWIN_BEGIN

namespace {
   /// UTF-8 to wide string used by windows api
   std::wstring to_wstring_s( const std::string& stringUtf8 )
   {
      if( stringUtf8.empty() ) return std::wstring();
      int iLength = ::MultiByteToWideChar( CP_UTF8, 0, stringUtf8.data(), (int)stringUtf8.size(), nullptr, 0 );
      std::wstring stringWide( iLength, L'\0' );
      ::MultiByteToWideChar( CP_UTF8, 0, stringUtf8.data(), (int)stringUtf8.size(), stringWide.data(), iLength );
      return stringWide;
   }
}

menu::enumType menu::item::type() const
{
   std::string stringType = get_string( "type" );
   if( stringType == "separator" ) return eTypeSeparator;
   if( stringType == "submenu" ) return eTypeSubmenu;
   return eTypeItem;
}

// ## get/set

std::vector<size_t> menu::children( size_t iParent ) const
{
   std::vector<size_t> vectorChild;
   for( size_t i = 0; i < m_vectorItem.size(); i++ )
   {
      if( m_vectorItem[i].m_iParent == iParent ) vectorChild.push_back( i );
   }
   return vectorChild;
}

bool menu::has_children( size_t iParent ) const
{
   for( const auto& itemChild : m_vectorItem ) { if( itemChild.m_iParent == iParent ) return true; }
   return false;
}

bool menu::is_submenu( size_t iItem ) const
{
   if( valid( iItem ) == false ) return false;
   return m_vectorItem[iItem].is_submenu() || has_children( iItem );
}

size_t menu::depth( size_t iItem ) const
{
   size_t uDepth = 0;
   size_t iParent = m_vectorItem.at( iItem ).m_iParent;
   while( iParent != npos ) { uDepth++; iParent = m_vectorItem[iParent].m_iParent; }
   return uDepth;
}

size_t menu::find( uint32_t uCommand, tag_command ) const
{
   for( size_t i = 0; i < m_vectorItem.size(); i++ )
   {
      const auto& itemFind = m_vectorItem[i];
      if( itemFind.type() == eTypeItem && itemFind.command() == uCommand ) return i;
   }
   return npos;
}

size_t menu::find( size_t iParent, std::string_view stringName, tag_name ) const
{
   for( size_t i = 0; i < m_vectorItem.size(); i++ )
   {
      if( m_vectorItem[i].m_iParent == iParent && m_vectorItem[i].name() == stringName ) return i;
   }
   return npos;
}

void menu::set( size_t iItem, std::string_view stringName, bool bValue, tag_flag )
{
   auto& argumentsItem = m_vectorItem.at( iItem ).m_arguments;
   argumentsItem.set( stringName, gd::argument::arguments::eTypeNumberBool, (gd::argument::arguments::const_pointer)&bValue, (unsigned int)sizeof( bool ) );
   sync_( iItem );
}

void menu::text_set_( size_t iItem, std::string_view stringName, std::string_view stringText )
{
   m_vectorItem.at( iItem ).m_arguments.set( stringName, stringText, gd::argument::arguments::tag_string_view{} );
}

// ## operations

size_t menu::add( size_t iParent, gd::argument::arguments argumentsItem )
{
   if( iParent != npos )
   {
      if( valid( iParent ) == false ) return npos;                             // parent must exist (this also guarantees parent is stored before child)
      if( m_vectorItem[iParent].is_separator() ) return npos;                  // separator cant have children
   }
   m_vectorItem.emplace_back( iParent, std::move( argumentsItem ) );
   return m_vectorItem.size() - 1;
}

size_t menu::add( size_t iParent, std::string_view stringName, uint32_t uCommand )
{
   gd::argument::arguments argumentsItem;
   argumentsItem.append( "name", stringName );
   argumentsItem.append( "command", uCommand );
   return add( iParent, std::move( argumentsItem ) );
}

size_t menu::add( size_t iParent, tag_separator )
{
   gd::argument::arguments argumentsItem;
   argumentsItem.append( "type", "separator" );
   return add( iParent, std::move( argumentsItem ) );
}

size_t menu::add( size_t iParent, std::string_view stringName, tag_submenu )
{
   gd::argument::arguments argumentsItem;
   argumentsItem.append( "name", stringName );
   argumentsItem.append( "type", "submenu" );
   return add( iParent, std::move( argumentsItem ) );
}

bool menu::remove( size_t iItem )
{
   if( valid( iItem ) == false ) return false;

   // parents are always stored before children so one pass finds the full subtree
   std::vector<bool> vectorRemove( m_vectorItem.size(), false );
   vectorRemove[iItem] = true;
   for( size_t i = iItem + 1; i < m_vectorItem.size(); i++ )
   {
      size_t iParent = m_vectorItem[i].m_iParent;
      if( iParent != npos && vectorRemove[iParent] ) vectorRemove[i] = true;
   }

   // new index for kept items
   std::vector<size_t> vectorIndex( m_vectorItem.size(), npos );
   std::vector<item> vectorKeep;
   vectorKeep.reserve( m_vectorItem.size() );
   for( size_t i = 0; i < m_vectorItem.size(); i++ )
   {
      if( vectorRemove[i] ) continue;
      vectorIndex[i] = vectorKeep.size();
      vectorKeep.push_back( std::move( m_vectorItem[i] ) );
   }
   for( auto& itemKeep : vectorKeep )
   {
      if( itemKeep.m_iParent != npos ) itemKeep.m_iParent = vectorIndex[itemKeep.m_iParent];
   }
   m_vectorItem = std::move( vectorKeep );
   return true;
}

/// Create the menu for all children of iParent. Submenus are created recursively
std::expected<HMENU, DWORD> menu::build_( size_t iParent, bool bPopup ) const
{
   HMENU hMenu = bPopup ? ::CreatePopupMenu() : ::CreateMenu();
   if( hMenu == nullptr ) return std::unexpected( ::GetLastError() );

   unsigned uPosition = 0;
   for( size_t iItem : children( iParent ) )
   {
      const item& itemBuild = m_vectorItem[iItem];
      HMENU hSubMenu = nullptr;

      std::wstring stringText;                                                 // text needs to live until InsertMenuItem is done
      MENUITEMINFOW mii{};
      mii.cbSize = sizeof( mii );

      if( itemBuild.is_separator() )
      {
         mii.fMask = MIIM_FTYPE | MIIM_DATA;
         mii.fType = MFT_SEPARATOR;
      }
      else
      {
         std::string stringName = itemBuild.name();
         std::string stringShortcut = itemBuild.shortcut();
         if( stringShortcut.empty() == false ) stringName += "\t" + stringShortcut;     // text after tab is right aligned accelerator text
         stringText = to_wstring_s( stringName );

         mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING | MIIM_DATA;
         mii.fType = MFT_STRING;
         if( itemBuild.is_radio() )             mii.fType |= MFT_RADIOCHECK;
         if( itemBuild.flag( "rightjustify" ) ) mii.fType |= MFT_RIGHTJUSTIFY;
         if( itemBuild.flag( "break" ) )        mii.fType |= MFT_MENUBREAK;
         if( itemBuild.flag( "barbreak" ) )     mii.fType |= MFT_MENUBARBREAK;

         mii.fState = MFS_ENABLED;
         if( itemBuild.is_checked() )  mii.fState |= MFS_CHECKED;
         if( itemBuild.is_disabled() ) mii.fState |= MFS_DISABLED;
         if( itemBuild.is_default() )  mii.fState |= MFS_DEFAULT;

         mii.dwTypeData = stringText.data();
         mii.cch = (unsigned)stringText.size();

         if( is_submenu( iItem ) )
         {
            auto result_ = build_( iItem, true );
            if( !result_ ) { ::DestroyMenu( hMenu ); return result_; } // destroys already attached submenus too
            hSubMenu = *result_;
            mii.fMask |= MIIM_SUBMENU;
            mii.hSubMenu = hSubMenu;
         }
         else
         {
            mii.fMask |= MIIM_ID;
            mii.wID = itemBuild.command();
         }
      }

      mii.dwItemData = (ULONG_PTR)iItem;                                       // index back to item, useful in WM_MENUSELECT, owner draw etc

      if( ::InsertMenuItemW( hMenu, uPosition, TRUE, &mii ) == FALSE )
      {
         DWORD uError = ::GetLastError();
         if( hSubMenu != nullptr ) ::DestroyMenu( hSubMenu );                  // not attached to hMenu
         ::DestroyMenu( hMenu );
         return std::unexpected( uError );
      }
      uPosition++;
   }

   return hMenu;
}

std::expected<HMENU, DWORD> menu::create_( bool bPopup )
{
   destroy();
   auto result_ = build_( npos, bPopup );
   if( result_ ) { m_hmenu = *result_; m_bOwner = true; }
   return result_;
}

std::expected<HMENU, DWORD> menu::attach( HWND hwnd )
{
   auto result_ = create( tag_bar{} );
   if( !result_ ) return result_;

   if( ::SetMenu( hwnd, m_hmenu ) == FALSE )
   {
      DWORD uError = ::GetLastError();
      destroy();
      return std::unexpected( uError );
   }
   m_hwnd = hwnd;
   m_bOwner = false;                                                           // window destroys menu when window is destroyed
   ::DrawMenuBar( hwnd );
   return m_hmenu;
}

uint32_t menu::popup( HWND hwnd, int iX, int iY, unsigned uFlags )
{
   auto result_ = build_( npos, true );
   if( !result_ ) return 0;

   ::SetForegroundWindow( hwnd );                                              // needed for popup to close when clicking outside
   unsigned uCommand = (unsigned)::TrackPopupMenuEx( *result_, uFlags | TPM_RETURNCMD, iX, iY, hwnd, nullptr );
   ::PostMessage( hwnd, WM_NULL, 0, 0 );
   ::DestroyMenu( *result_ );
   return uCommand;
}

void menu::destroy()
{
   if( m_hmenu != nullptr && m_bOwner ) ::DestroyMenu( m_hmenu );
   m_hmenu = nullptr;
   m_bOwner = false;
}

/// Update check and enable state for item in live menu
void menu::sync_( size_t iItem ) const
{
   if( m_hmenu == nullptr || valid( iItem ) == false ) return;
   const item& itemSync = m_vectorItem[iItem];
   if( itemSync.type() != eTypeItem || is_submenu( iItem ) ) return;           // only command items can be found by command

   unsigned uCommand = itemSync.command();
   ::CheckMenuItem( m_hmenu, uCommand, MF_BYCOMMAND | ( itemSync.is_checked() ? MF_CHECKED : MF_UNCHECKED ) );
   ::EnableMenuItem( m_hmenu, uCommand, MF_BYCOMMAND | ( itemSync.is_disabled() ? MF_GRAYED : MF_ENABLED ) );
   if( m_hwnd != nullptr ) ::DrawMenuBar( m_hwnd );
}

// ## debug

std::string menu::to_string( tag_debug ) const
{
   std::ostringstream stream_;
   for( size_t i = 0; i < m_vectorItem.size(); i++ )
   {
      const item& itemDump = m_vectorItem[i];
      stream_ << std::string( depth( i ) * 3, ' ' );
      if( itemDump.is_separator() ) { stream_ << "----\n"; continue; }
      stream_ << ( is_submenu( i ) ? "[+] " : "    " ) << itemDump.name();
      if( is_submenu( i ) == false ) stream_ << " (" << itemDump.command() << ")";
      if( itemDump.is_checked() ) stream_ << " [checked]";
      if( itemDump.is_disabled() ) stream_ << " [disabled]";
      stream_ << "\n";
   }
   return stream_.str();
}

_GDWIN_END