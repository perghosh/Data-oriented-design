
// @FILE [tag: lua, bindings] [summary: Lua bindings for web server] [description: Exposes C++ functions and classes to Lua scripts executed in the web server context. This includes utilities for handling HTTP requests, responses, and application-specific logic. The bindings are implemented using the sol2 library to create seamless interoperability between C++ and Lua. The file defines functions that can be called from Lua scripts to interact with the web server's functionality, such as sending responses, accessing request data, and managing sessions.]

#include "LUAObjects.h"
#include "LUABindings.h"

LUA_BEGIN

void RegisterApplication( sol::state& stateLua )
{
	stateLua.new_usertype<Application>(
      "Application", sol::constructors<Application()>(),
      "GetDatabase", &Application::GetDatabase,
      "GetDocument", &Application::GetDocument,
		"GetProperty", &Application::GetProperty,
      "GetPropertyCount", &Application::GetPropertyCount,
      "GetPropertyName", &Application::GetPropertyName,
      "Message", &Application::Message,
      "Initialize", &Application::Initialize,
      "SetLogLevel", &Application::SetLogLevel,
		"SetProperty", &Application::SetProperty
   ); 
}

void RegisterDocument( sol::state& stateLua )
{
	stateLua.new_usertype<Document>(
      "Document", sol::constructors<Document()>(),
      "GetDatabase", &Document::GetDatabase
   ); 
}

void RegisterDatabase( sol::state& stateLua )
{
	stateLua.new_usertype<Database>(
      "Database", sol::constructors<Database(), Database( void* ), Database( sol::table )>(),
      "IsOpen", &Database::IsOpen,
      "Open", &Database::Open,
		"Execute", &Database::Execute,
      "ExecuteReturn", &Database::ExecuteReturn,
      "Ask", &Database::Ask,
      "Close", &Database::Close
   );
}

void RegisterCursor( sol::state& stateLua )
{
	stateLua.new_usertype<Cursor>(
      "Cursor", sol::constructors<Cursor( Database* )>(),
      "IsOpen", &Cursor::IsOpen,
		"Open", &Cursor::Open,
      "Next", &Cursor::Next,
      "IsValidRow", &Cursor::IsValidRow,
      "Close", &Cursor::Close,
      "GetValue", &Cursor::GetValue,
      "GetTable", &Cursor::GetTable
   );
}

void RegisterTable( sol::state& stateLua )
{
	stateLua.new_usertype<Table>(
      "Table", sol::constructors<Table(), Table( const std::string_view& ), Table( uint64_t, const std::string_view& )>(),
      "__len", &Table::GetRowCount,
      "AddRow", &Table::AddRow,
      "GetColumnCount", &Table::GetColumnCount,
      "GetColumns", &Table::GetColumns,
      "GetRowCount", &Table::GetRowCount,
      "Fill", &Table::Fill,
      "Find", &Table::Find,
      "GetCellValue", &Table::GetCellValue,
      "SetCellValue", &Table::SetCellValue,
      "SetCellValues", &Table::SetCellValues,
      "SetColumnAttribute", &Table::SetColumnAttribute,
      "Read", &Table::Read,
      "Write", &Table::Write
      );
}

LUA_END
