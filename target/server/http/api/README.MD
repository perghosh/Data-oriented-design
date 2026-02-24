# API folder

## CAPIDatabase
Simple logic for handling CAPIDatabase requests. This may be used to work with from terminals.
You do not need to have a user and are able to work with databases directly.

### Endpoints
- `db/create` — Create a database.
- `db/open` — Open/connect to a database.
- `db/query` — Execute a SQL query.
- `db/select` — Execute a select operation.
- `db/ask` — Execute an ask/select-style operation.
- `db/insert` — Insert data.
- `db/update` — Update data.
- `db/delete` — Delete data.
- `db/drop` — Drop database objects (reserved).

## CAPISystem
System related APIs. This are things like work with internal data inside webserver. Or logick that are OS specific, can be cache, session, file operations and similar.

### Endpoints
- `sys/file/delete` — Delete a file.
- `sys/file/directory` — Get or set directory paths.
- `sys/file/exists` — Check if a file exists.
- `sys/meta/query/add` — Add query.
- `sys/meta/query/delete` — Delete a query.
- `sys/meta/query/exists` — Check if a query exists.
- `sys/meta/db/fields` — Return metadata for database fields.
- `sys/session/add` — Add a session.
- `sys/session/count` — Count active sessions.
- `sys/session/exists` — Check if a session exists.
- `sys/session/delete` — Delete a session.
- `sys/session/list` — List active sessions.
