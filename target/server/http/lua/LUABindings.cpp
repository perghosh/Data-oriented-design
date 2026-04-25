
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
      "CreateSql", &Document::CreateSql,
      "GetDatabase", &Document::GetDatabase
   ); 
}

void RegisterDatabase( sol::state& stateLua )
{
	stateLua.new_usertype<Database>(
      "Database", sol::constructors<Database(), Database( void* ), Database( sol::table )>(),
      "Ask", &Database::Ask,
      "IsOpen", &Database::IsOpen,
      "Open", &Database::Open,
		"Execute", &Database::Execute,
      "ExecuteReturn", &Database::ExecuteReturn,
      "Ask", &Database::Ask,
      "Close", &Database::Close
   );
}

void RegisterRequest( sol::state& stateLua )
{
	stateLua.new_usertype<Request>(
      "Request", sol::constructors<Request()>(),                              // constructor
      "CreateSql", &Request::CreateSql,                                       // Create SQL object, will add sql values if request has them
      "GetApplication", &Request::GetApplication,                             // Get the application object associated with this request
      "GetClientValue", &Request::GetClientValue,                             // Get a client value by name, client values are passed from client
      "GetDocument", &Request::GetDocument,                                   // Get the document object associated with this request
      "GetDatabase", &Request::GetDatabase,                                   // Get the database object associated with this request
      "GetIpAddress", &Request::GetIpAddress,                                 // Get the IP address of the client making the request
      "GetScriptValue", &Request::GetScriptValue,                             // Get script value, variables that is not transfered to created SQL object but may be used in this request
      "GetSessionId", &Request::GetSessionId,                                 // Get the session ID associated with this request
      "SetScriptValue", &Request::SetScriptValue                              // Set a script value by name. Script values are added in script and is used there
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

void RegisterSql( sol::state& stateLua )
{
   stateLua.new_usertype<Sql>(
      "Sql", sol::constructors<Sql()>(),
      "AddColumn", &Sql::AddColumn,
      "AddCondition", &Sql::AddCondition,
      "AddValues", &Sql::AddValues,
      "AsInsert", &Sql::AsInsert,
      "AsSelect", &Sql::AsSelect,
      "GetValue", &Sql::GetValue,
      "RemoveColumn", &Sql::RemoveColumn,
      "SetColumn", &Sql::SetColumn
   );
}

void RegisterExpression( sol::state& stateLua )
{
	stateLua.new_usertype<Expression>(
      "Expression", sol::constructors<Expression()>(),
      "Calculate", &Expression::Calculate
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
