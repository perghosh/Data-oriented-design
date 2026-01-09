

## System
System endpoints are mostly used for managing the server's system resources and configuration.
These should be used with caution and not when the server used in public because you can do anything.

For development purposes, you can use these endpoints to test and debug your application.

- `sys/file/delete (path)` - Delete file at specified path
- `sys/file/directory (action, type, name/value)` - Get or set directory paths (action: get/set, type: folder type, name/value: for set)
- `sys/file/exists (path)` - Check if file exists at specified path
- `sys/session/add (session|new)` - Add session by value or generate new session
- `sys/session/count ()` - Count number of active sessions
- `sys/session/exists (session)` - Check if session exists and return its index
- `sys/session/delete (session|index)` - Delete session by uuid or index
- `sys/session/list ()` - List all active sessions

## Database

## Sql
