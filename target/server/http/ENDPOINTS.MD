## HTTP Command Endpoint Model

Command endpoints are routed only when the request target starts with `!`.

- Example: `/!sys/session/count`
- Static file requests do **not** use `!`.

### Endpoint composition

Endpoints are command chains.  
Atomic operations are the final operation segment in each chain.

- Chain example: `!db/open/select?...`
  - `open` = atomic operation
  - `select` = atomic operation

---

## Atomic Endpoints

## System (`sys`)

### File
- `sys/file/delete`  
  - Query: `path`
  - Deletes a file.
- `sys/file/directory`  
  - Query: `action=get|set`, `type`, `name|value` (for `set`)
  - Gets or sets configured folder paths.
- `sys/file/exists`  
  - Query: `path`
  - Checks whether a file exists.

### Session
- `sys/session/add`  
  - Query: `session` **or** `new`
  - Adds explicit session value or generates a new one.
- `sys/session/count`  
  - Returns active session count.
- `sys/session/exists`  
  - Query: `session`
  - Returns session position/index if found.
- `sys/session/delete`  
  - Query: `session` **or** `index`
  - Deletes a session.
- `sys/session/list`  
  - Returns all sessions.

### Metadata
- `sys/meta/query/add` (alias: `sys/meta/sql/add`)  
  - Query: `name`, `type`, `format`, `query|statement`
  - Adds stored query metadata.
- `sys/meta/query/count` (alias: `sys/meta/sql/count`)  
  - Query: optional `name` or `uuid`
  - Counts query entries.
- `sys/meta/query/delete` (alias: `sys/meta/sql/delete`)  
  - Query: `id`, optional `key`
  - Deletes query metadata.
- `sys/meta/query/exists` (alias: `sys/meta/sql/exists`)  
  - Query: `name`
  - Checks if named query exists.
- `sys/meta/db/fields`  
  - Query: `table`, `field` (single or comma-separated)
  - Returns field metadata.

### Client / Application
- `sys/client/ip`  
  - Returns client IP address.
- `sys/application/quit`  
  - Stops server process.

---

## Database (`db`)

- `db/execute`  
  - XML-driven bulk execution flow.
- `db/create`  
  - Query: `type` (default sqlite), `name`
  - Creates database.
- `db/open`  
  - Query: `type` (default sqlite), `name`, optional `document|doc`
  - Opens database connection for document.
- `db/query`  
  - Query: `query`
  - Executes SQL (non-tabular response path).
- `db/select`  
  - Query: `query` or prepared-template inputs
  - Executes SELECT and returns table result.
- `db/ask`  
  - Query: select-style inputs
  - Returns first row as arguments object.
- `db/insert`  
  - Query: `query` or `values`/`record`/template inputs, optional `mode=single|bulk`
  - Executes insert.
- `db/update`  
  - Query: update inputs via template or `record`
  - Executes update and returns changed row count.
- `db/delete`  
  - Query: delete inputs via template or `record`
  - Executes delete and returns changed row count.

---

## SQL (`sql`)

- `sql/add`  
  - Query: `query`
  - Adds SQL query text to metadata store.

---

## View (`view`)

- `view/ssr`  
  - Server-side page render operation.

---

## Notes

- Commands can be chained in one endpoint path.
- Request body parsing supports XML/JSON based on `Content-Type`.
- Default command response format is XML unless JSON result format is explicitly set by command handling.

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
