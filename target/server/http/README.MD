

```
|   Application.cpp
|   Application.h
|   CMakeLists.txt
|   Document.cpp
|   Document.h
|   HttpServer.cpp
|   HttpServer.h
|   main.cpp
|   Router.cpp
|   Router.h
|   Server.cpp
|   Server.h
|
+-- api
|
+-- playground
|
+-- state
    |   Session.cpp
    |    Session.h

```



## Description of objects and files
- `Application.cpp` / `Application.h`: Defines the main application class that initializes and runs the HTTP server.
- `Document.cpp` / `Document.h`: Manages the creation and handling data used in HTTP server.
- `Router.cpp` / `Router.h`: Implements routing logic to direct incoming HTTP requests to the appropriate handlers.
- `Server.cpp` / `Server.h`: Contains the core server functionality, including request handling and response generation.