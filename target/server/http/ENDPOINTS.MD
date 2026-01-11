

## System
System endpoints are mostly used for managing the server's system resources and configuration.
These should be used with caution and not when the server used in public because you can do anything.

For development purposes, you can use these endpoints to test and debug your application.

### System - File
- `sys/file/delete (path)` - Delete file at specified path
- `sys/file/directory (action, type, name/value)` - Get or set directory paths (action: get/set, type: folder type, name/value: for set)
- `sys/file/exists (path)` - Check if file exists at specified path

### System - Session
- `sys/session/add (session|new)` - Add session by value or generate new session
- `sys/session/count ()` - Count number of active sessions
- `sys/session/exists (session)` - Check if session exists and return its index
- `sys/session/delete (session|index)` - Delete session by uuid or index
- `sys/session/list ()` - List all active sessions

### System - Metadata

- `sys/meta/add (query, id)` - Add session by value or generate new session

## Database

- `db/create (type, name)` - Create a new database with specified type and name, the format of name depends on type of database
- `db/open (type, name)` - Delete database with specified type and name
- `db/list ()` - List all databases

## Sql


## LUA
Endpoints for LUA integration and scripting.

## Python
Endpoints for Python integration and scripting.


```
{
  //"args": [ "--configuration", "C:\\dev\\home\\DOD\\resource\\.http-configuration.xml" ],
  "args": [ "http", "--site", "D:\\temp\\www", "--port", "8001", "--ip", "\"127.0.0.1\"", "--add-session", "00,01,02,03,04,05" ],
  "type": "default",
  "project": "CMakeLists.txt",
  "projectTarget": "http.exe (target\\server\\http\\http.exe)",
  "name": "http.exe (target\\server\\http\\http.exe)",
  "currentDir": "D:\\temp\\www"
}
```
