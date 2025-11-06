
#include <iostream>
#include <memory>
#include <filesystem>
#include <iostream>
#include <cstring>

//#include <boost/asio.hpp>
#include <libssh2/libssh2.h>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    //#pragma comment(lib, "libssh2.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    typedef int socket_t;
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET -1
#endif

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"


//#include "tool/Tool_SSHClient.h"
#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

class SSHConnection {
private:
    socket_t sock;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;

    bool initializeWinsock() {
#ifdef _WIN32
        WSADATA wsadata;
        if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
#endif
        return true;
    }

    void cleanupWinsock() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

public:
    SSHConnection() : sock(INVALID_SOCKET), session(nullptr), channel(nullptr) {
        initializeWinsock();
    }

    ~SSHConnection() {
        disconnect();
        cleanupWinsock();
    }

    bool connect(const std::string& host, int port, 
                 const std::string& username, const std::string& password) {
        
        // Initialize libssh2
        if (libssh2_init(0) != 0) {
            std::cerr << "libssh2 initialization failed" << std::endl;
            return false;
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Resolve hostname
        struct hostent* he = gethostbyname(host.c_str());
        if (he == nullptr) {
            std::cerr << "Failed to resolve hostname: " << host << std::endl;
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        // Connect to SSH server
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        memcpy(&sin.sin_addr, he->h_addr_list[0], he->h_length);
        
        if (::connect(sock, (struct sockaddr*)(&sin), sizeof(sin)) != 0) {
            std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        std::cout << "TCP connection established to " << host << ":" << port << std::endl;

        // Create SSH session
        session = libssh2_session_init();
        if (!session) {
            std::cerr << "Failed to create SSH session" << std::endl;
            return false;
        }

        // Start SSH handshake
        if (libssh2_session_handshake(session, sock) != 0) {
            char *errmsg;
            int errlen;
            libssh2_session_last_error(session, &errmsg, &errlen, 0);
            std::cerr << "SSH handshake failed: " << errmsg << std::endl;
            return false;
        }

        std::cout << "SSH handshake completed" << std::endl;

        // Get authentication methods
        char* userauthlist = libssh2_userauth_list(session, username.c_str(), username.length());
        if (userauthlist) {
            std::cout << "Authentication methods: " << userauthlist << std::endl;
        }

        // Authenticate with password
        if (libssh2_userauth_password(session, username.c_str(), password.c_str()) != 0) {
            char *errmsg;
            int errlen;
            libssh2_session_last_error(session, &errmsg, &errlen, 0);
            std::cerr << "Authentication failed: " << errmsg << std::endl;
            return false;
        }

        std::cout << "Successfully authenticated as " << username << std::endl;
        return true;
    }

    bool executeCommand(const std::string& command, std::string& output) {
        if (!session) {
            std::cerr << "Not connected" << std::endl;
            return false;
        }

        // Open a channel
        channel = libssh2_channel_open_session(session);
        if (!channel) {
            char *errmsg;
            int errlen;
            libssh2_session_last_error(session, &errmsg, &errlen, 0);
            std::cerr << "Failed to open channel: " << errmsg << std::endl;
            return false;
        }

        // Execute command
        if (libssh2_channel_exec(channel, command.c_str()) != 0) {
            char *errmsg;
            int errlen;
            libssh2_session_last_error(session, &errmsg, &errlen, 0);
            std::cerr << "Failed to execute command: " << errmsg << std::endl;
            libssh2_channel_free(channel);
            channel = nullptr;
            return false;
        }

        // Read output (stdout)
        char buffer[1024];
        int nbytes;
        output.clear();

        while ((nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer))) > 0) {
            output.append(buffer, nbytes);
        }

        // Read stderr if there's any
        std::string stderr_output;
        while ((nbytes = libssh2_channel_read_stderr(channel, buffer, sizeof(buffer))) > 0) {
            stderr_output.append(buffer, nbytes);
        }

        if (!stderr_output.empty()) {
            output += "\n[STDERR]:\n" + stderr_output;
        }

        // Get exit status
        int exitcode = libssh2_channel_get_exit_status(channel);
        
        // Close channel
        libssh2_channel_close(channel);
        libssh2_channel_free(channel);
        channel = nullptr;

        if (exitcode != 0) {
            std::cerr << "Command exited with status: " << exitcode << std::endl;
        }

        return true;
    }

    void disconnect() {
        if (channel) {
            libssh2_channel_free(channel);
            channel = nullptr;
        }

        if (session) {
            libssh2_session_disconnect(session, "Normal Shutdown");
            libssh2_session_free(session);
            session = nullptr;
        }

        if (sock != INVALID_SOCKET) {
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
        }

        libssh2_exit();
    }
};


TEST_CASE("[ssh] ssh", "[ssh]")
{
    SSHConnection ssh;

    // Connect to SSH server
    /*
    if(!ssh.connect("192.168.1.100", 22, "username", "password")) 
    {
        return;
    }
    */

    if(!ssh.connect("192.168.1.244", 22, "per", "1309")) 
    {
      std::cerr << "Connection failed!" << std::endl;
      return;
    }

    // Execute a command
    std::string output;
    
    std::cout << "\n=== Executing: ls -la /home ===" << std::endl;
    if (ssh.executeCommand("ls -la /home", output)) {
        std::cout << output << std::endl;
    }

    std::cout << "\n=== Executing: uname -a ===" << std::endl;
    if (ssh.executeCommand("uname -a", output)) {
        std::cout << output << std::endl;
    }

    std::cout << "\n=== Executing: whoami ===" << std::endl;
    if (ssh.executeCommand("whoami", output)) {
        std::cout << output << std::endl;
    }

    // Connection is automatically closed by destructor
    std::cout << "\nDisconnecting..." << std::endl;
}


/*

cd where/you/like/to/install
git clone https://github.com/libssh2/libssh2.git
cd libssh2
mkdir dll <-- directory to install libssh2
cmake -DCRYPTO_BACKEND=WinCNG -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./dll -S . -B build
cmake --build . --target install

*/