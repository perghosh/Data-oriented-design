# Overview

## What is this HTTP Server?

This is a **stateful, high-performance web server** designed for modern web applications that require persistent user sessions and efficient data handling. Built with a strong focus on speed and scalability, this server simplifies web development by maintaining state information for each user, eliminating the need to pass complete user data with every request.
Staeful do not mean that the server is not stateless, it can of course be stateless also.

### Key Characteristics

**Performance & Scalability**
- Optimized for high concurrency with a target of handling approximately **100,000 concurrent users** on a single computer with 32GB RAM
- Designed to serve as a high-performance cache layer for other solutions
- Lightweight architecture that maximizes hardware efficiency
- Intelligent caching mechanisms to reduce database load and improve response times
- Fast server-side rendering (SSR) capabilities for dynamic content generation

**Database Integration**
- Compatible with **any SQL database** and SQLite
- Extensive SQL logic support with automatic query generation based on API calls
- Intelligent query optimization and security rule generation
- Built-in metadata management to help developers understand and secure their data operations

**Stateful Architecture**
- Maintains unique session keys for each client, enabling the server to identify users across requests
- Simplifies development by eliminating the need to manage state manually on the client side
- Reduces network overhead by keeping user context on the server

**Atomic Operations**
- Supports atomic operations to ensure data integrity and consistency
- Packs multiple operations into a single request, reducing the number of round-trips between client and server
- Only one entry point for all operations, simplifying the API and improving security and performance
- Easy to learn and use, no need to learn multiple endpoints for different operations.

### Architecture Overview

The server is built around several core components that work together to deliver a seamless experience:

- **Application**: The central entry point and global state manager
- **Server**: Handles HTTP requests using Boost, routes internal APIs (prefixed with `!`)
- **Router**: Directs requests to appropriate handlers based on URL and HTTP method
- **Session**: Manages individual client connections and maintains state

### Extensibility & Customization

**Scripting**
- **Lua** for creating flexible endpoints and custom logic
- **Python** integration for advanced customization and data processing (upcomming)
- **gdscript** gdscript is a highly optimized scripting language for fast execution of expressions and smaller tasks.
- Ability to execute server-side logic to handle edge cases and complex business rules

These scripting capabilities will allow developers to extend the server's functionality without modifying the core codebase, making it adaptable to a wide range of use cases.

### Use Cases

This HTTP server is ideal for:
- **API backends** that require persistent user sessions
- **Caching layers** for existing applications
- **Real-time applications** that benefit from stateful connections
- **Data-intensive applications** with complex SQL requirements
- **High-traffic web services** that need to maximize server resources
- **Weak servers need to handle more users** Servers with limited resources can benefit and manage more users by using this server.




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
- `@OPTIMIZED`: Used to describe code that has been optimized for performance.
- `@DEPRECATED`: Used to describe code that is no longer in use.

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
|   |   APIBase.*
|   |   APIDatabase.*
|   |   APISql.*
|   |   APISystem.*
|   |   APIView.*
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
+-- lua
|   |   LUABindings.*
|   |   LUAObjects.*
|   |   LUAStatePool.*
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

### API Modules

- `APIBase.*`: Contains the base class for all API modules. This is core for handling API and it has the `CAPIContext` where connections for request are found. Also data from client and data returned. Note that there are logic to derive from this doing specific things.
- `APIDatabase.*`: Contains the API module for database operations. Often use statements and scripting to customize the database operations. 
- `APISql.*`: Manages API requests for SQL statements, adding, deleting, or updating statements. Statements are often used in database operations found in `APIDatabase`.
- `APISystem.*`: Handles system-related API requests. The system is the webserver.
- `APIView.*`: Manages API requests related to rendering views like SSR (server side rendering).

### DTO Modules
- `DTOResponse.*`: Contains data transfer logic used for structuring HTTP responses. Internaly data is moved using objects in gd library and these are converted to client ffriendly format using this module. This is the only module that should be used to convert data to client friendly format.

### Lua Scripting
- `LUABindings.*`: Contains bindings for Lua scripting, allowing interaction between Lua scripts and the server.
- `LUAObjects.*`: Manages Lua objects and their lifecycle within the server.
- `LUAStatePool.*`: Handles the pooling of Lua states for efficient script execution. Server is heavily threaded and using Lua scripting in a multi-threaded environment so pooling Lua states is a common solution to manage resources effectively.
