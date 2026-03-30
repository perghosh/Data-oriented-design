# Overview

## What is this HTTP Server?

This is a **stateful, high-performance web server** designed for modern web applications that require persistent user sessions and efficient data handling. Built with a strong focus on speed and scalability, this server simplifies web development by maintaining state information for each user, eliminating the need to pass complete user data with every request.

### Key Characteristics

**Performance & Scalability**
- Optimized for high concurrency with a target of handling approximately **50,000 concurrent users** on a single computer with 64GB RAM
- Designed to serve as a high-performance cache layer for other solutions
- Lightweight architecture that maximizes hardware efficiency

**Database Integration**
- Compatible with **any SQL database** and SQLite
- Extensive SQL logic support with automatic query generation based on API calls
- Intelligent query optimization and security rule generation
- Built-in metadata management to help developers understand and secure their data operations

**Stateful Architecture**
- Maintains unique session keys for each client, enabling the server to identify users across requests
- Simplifies development by eliminating the need to manage state manually on the client side
- Reduces network overhead by keeping user context on the server

### Architecture Overview

The server is built around several core components that work together to deliver a seamless experience:

- **Application**: The central entry point and global state manager
- **Server**: Handles HTTP requests using Boost, routes internal APIs (prefixed with `!`)
- **Router**: Directs requests to appropriate handlers based on URL and HTTP method
- **Session**: Manages individual client connections and maintains state

### Extensibility & Customization

**Upcoming Features**
- **Lua scripting** for creating flexible endpoints and custom logic
- **Python scripting** integration for advanced customization
- Ability to execute server-side logic to handle edge cases and complex business rules

These scripting capabilities will allow developers to extend the server's functionality without modifying the core codebase, making it adaptable to a wide range of use cases.

### Use Cases

This HTTP server is ideal for:
- **API backends** that require persistent user sessions
- **Caching layers** for existing applications
- **Real-time applications** that benefit from stateful connections
- **Data-intensive applications** with complex SQL requirements
- **High-traffic web services** that need to maximize server resources




## Search tags

- `@CRITICAL`: Indicates critical sections of code that require immediate attention.
- `@NOTE` : Something that is important to note, may effect other parts, important to understand context etc
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
