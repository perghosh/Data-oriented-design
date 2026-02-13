## Search tags

- `@CRITICAL`: Indicates critical sections of code that require immediate attention.
- `@FILE`: Describes the file (always placed at the top).
- `@PROJECT`: Used for project management. Searching for a project name lists all its tasks.
- `@TASK`: Describes a specific task or feature within the project.
- `@API`: Used to describe methods and groups of methods. A way to organize and document code.
- `@TODO`: Used to describe tasks that need to be completed. Short reminder
- `@DEBUG`: Used to describe code used for debugging purposes.
- `@CLASS`: classes and structs
- `@DEPRICATED`: Used to describe code that is no longer in use.

## Folder Structure
- **api**: Contains modules that handle specific API endpoints for database, SQL, and system operations.
- **cli**: Contains modules for command-line interface HTTP handling.
- **convert**: Contains modules for data conversion and transformation.
- **dto**: Contains data transfer objects used for structuring HTTP responses.
- **meta**: Contains metadata-related modules, data used in webserver.
- **playground**: A space for experimental code and testing new features.
- **render**: Contains modules for rendering information, for example render SQL queries and results.

```
|   Application.*
|   Document.*
|   HttpServer.*
|   main.cpp
|   Router.*
|   Server.*
|   Session.*
|
+-- api
|   |   APIDatabase.*
|   |   APISql.*
|   |   APISystem.*
|
+-- cli
|   |   CLIHttp.*
|
+-- convert
|   |   CONVERTCore.*
|
+-- dto
|   |   DTOResponse.*
|
+-- meta
|   |   METADatabase.*
|   |   METAQueries.*
|
+-- render
|   |   RENDERSql.*

+-- playground
|
+-- state
|   |   Session.cpp
|   |   Session.h

```


### Description of objects and files
- `Application.cpp` / `Application.h`: Defines the main application class that initializes and runs the HTTP server.
- `Document.cpp` / `Document.h`: Manages the creation and handling data used in HTTP server.
- `Router.cpp` / `Router.h`: Implements routing logic to direct incoming HTTP requests to the appropriate handlers.
- `Server.cpp` / `Server.h`: Contains the core server functionality, including request handling and response generation.
- `Session.cpp` / `Session.h`: Manages individual client sessions, handling request parsing and response sending.

### Application
Application acts as the hallway to enter the server based on code. There are only one application object and using
this object you can access almost everything in the server. Compare application with the hallway in a big 
building, you enter the building using the hallway and then you can go to different rooms (modules) in the building (server).
Application have some helper functions that are mostly free fucntions or like globals for logic that are dependent on the global state of the server.

### Server
**Server** is the layer above boost to handle HTTP requests. It listens for incoming connections, processes requests, and sends back responses.
Server is also used to check the url and initialize the correct handlers. Internal api are identified by the prefix `!` and then routed using `Router`
to the right handler.

### Router
**Router** is responsible for routing incoming HTTP requests to the appropriate handlers based on the URL and HTTP method. By reading
the url Router can identify which module should handle the request.

### Session
**Session** represents a single client connection to the server. It manages the lifecycle of conected clients. Http server is a **stateful** server, the session
holds unique keys for each client so the server can identify clients between requests.
