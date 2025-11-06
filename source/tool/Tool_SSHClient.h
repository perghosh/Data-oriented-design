/*----------------------------------------------------------------------------- SSHClient.h
 * Cross-Platform SSH Client Implementation
 * 
 * Supports both Windows and Linux platforms using libssh2.
 * Provides connection management, authentication, and command execution.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "libssh2/libssh2_setup.h"
#include "libssh2/libssh2.h"


#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define closesocket close
#endif



/*----------------------------------------------------------------------------- CSSHClient
 * @brief SSH client for remote Linux machine connections
 * 
 * Manages SSH connections, authentication, and command execution
 * across Windows and Linux platforms.
 */
class CSSHClient
{
public:
    CSSHClient();
    ~CSSHClient();

    // Connection management
    bool Connect( const std::string& stringHost, 
                  unsigned uPort = 22 );
    bool Authenticate( const std::string& stringUsername, 
                       const std::string& stringPassword );
    bool AuthenticateWithKey( const std::string& stringUsername,
                              const std::string& stringPublicKeyPath,
                              const std::string& stringPrivateKeyPath,
                              const std::string& stringPassphrase = "" );
    void Disconnect();
    bool IsConnected() const;

    // Command execution
    bool ExecuteCommand( const std::string& stringCommand,
                         std::string& stringOutput,
                         int& iExitCode );

    // Error handling
    std::string GetLastError() const;

private:
    // Helper methods
    bool initializeSocket();
    bool initializeSSH();
    void cleanup();
    void setLastError( const std::string& stringError );

    // Member variables
    socket_t m_socket;                                                         // TCP socket
    LIBSSH2_SESSION* m_psessionSSH;                                            // SSH session handle
    std::string m_stringHost;                                                  // Target hostname
    unsigned m_uPort;                                                          // Target port
    bool m_bConnected;                                                         // Connection state
    bool m_bAuthenticated;                                                     // Authentication state
    std::string m_stringLastError;                                             // Last error message

#ifdef _WIN32
    bool m_bWSAInitialized;                                                    // Winsock initialized flag
#endif
};

/*----------------------------------------------------------------------------- SSHClient.cpp
 * Implementation of cross-platform SSH client
 */

// #include "SSHClient.h"
#include <cstring>
#include <sstream>

/*----------------------------------------------------------------------------- CSSHClient::CSSHClient
 * @brief Constructor - initializes SSH client
 */
CSSHClient::CSSHClient()
    : m_socket( INVALID_SOCKET_VALUE )
    , m_psessionSSH( nullptr )
    , m_uPort( 22 )
    , m_bConnected( false )
    , m_bAuthenticated( false )
#ifdef _WIN32
    , m_bWSAInitialized( false )
#endif
{
    // Initialize libssh2
    int iResult = libssh2_init( 0 );
    if( iResult != 0 )
    {
        setLastError( "Failed to initialize libssh2" );
    }

#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if( iResult == 0 )
    {
        m_bWSAInitialized = true;
    }
    else
    {
        setLastError( "Failed to initialize Winsock" );
    }
#endif
}

/*----------------------------------------------------------------------------- CSSHClient::~CSSHClient
 * @brief Destructor - cleans up resources
 */
CSSHClient::~CSSHClient()
{
    cleanup();
    libssh2_exit();

#ifdef _WIN32
    if( m_bWSAInitialized == true )
    {
        WSACleanup();
    }
#endif
}

/*----------------------------------------------------------------------------- CSSHClient::Connect
 * @brief Establishes connection to remote SSH server
 * 
 * Creates TCP socket connection and initializes SSH session.
 * 
 * @param stringHost Hostname or IP address
 * @param uPort SSH port number (default 22)
 * @return bool True if connection succeeded
 */
bool CSSHClient::Connect( const std::string& stringHost, unsigned uPort )
{
    if( m_bConnected == true )                                                 // Already connected
    {
        setLastError( "Already connected" );
        return false;
    }

    m_stringHost = stringHost;
    m_uPort = uPort;

    // Create socket
    if( initializeSocket() == false )
    {
        return false;
    }

    // Initialize SSH session
    if( initializeSSH() == false )
    {
        cleanup();
        return false;
    }

    m_bConnected = true;
    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::Authenticate
 * @brief Authenticates using username and password
 * 
 * @param stringUsername SSH username
 * @param stringPassword SSH password
 * @return bool True if authentication succeeded
 */
bool CSSHClient::Authenticate( const std::string& stringUsername, 
                                const std::string& stringPassword )
{
    if( m_bConnected == false )                                                // Not connected
    {
        setLastError( "Not connected to SSH server" );
        return false;
    }

    if( m_psessionSSH == nullptr )                                             // Invalid session
    {
        setLastError( "Invalid SSH session" );
        return false;
    }

    int iResult = libssh2_userauth_password( m_psessionSSH,
                                              stringUsername.c_str(),
                                              stringPassword.c_str() );

    if( iResult != 0 )
    {
        char* pszError = nullptr;
        int iErrorLen = 0;
        libssh2_session_last_error( m_psessionSSH, &pszError, &iErrorLen, 0 );
        std::string stringError = "Authentication failed: ";
        if( pszError != nullptr )
        {
            stringError += std::string( pszError, iErrorLen );
        }
        setLastError( stringError );
        return false;
    }

    m_bAuthenticated = true;
    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::AuthenticateWithKey
 * @brief Authenticates using public/private key pair
 * 
 * @param stringUsername SSH username
 * @param stringPublicKeyPath Path to public key file
 * @param stringPrivateKeyPath Path to private key file
 * @param stringPassphrase Optional passphrase for private key
 * @return bool True if authentication succeeded
 */
bool CSSHClient::AuthenticateWithKey( const std::string& stringUsername,
                                       const std::string& stringPublicKeyPath,
                                       const std::string& stringPrivateKeyPath,
                                       const std::string& stringPassphrase )
{
    if( m_bConnected == false )                                                // Not connected
    {
        setLastError( "Not connected to SSH server" );
        return false;
    }

    if( m_psessionSSH == nullptr )                                             // Invalid session
    {
        setLastError( "Invalid SSH session" );
        return false;
    }

    const char* pszPassphrase = stringPassphrase.empty() ? nullptr 
                                                          : stringPassphrase.c_str();

    int iResult = libssh2_userauth_publickey_fromfile( m_psessionSSH,
                                                        stringUsername.c_str(),
                                                        stringPublicKeyPath.c_str(),
                                                        stringPrivateKeyPath.c_str(),
                                                        pszPassphrase );

    if( iResult != 0 )
    {
        char* pszError = nullptr;
        int iErrorLen = 0;
        libssh2_session_last_error( m_psessionSSH, &pszError, &iErrorLen, 0 );
        std::string stringError = "Key authentication failed: ";
        if( pszError != nullptr )
        {
            stringError += std::string( pszError, iErrorLen );
        }
        setLastError( stringError );
        return false;
    }

    m_bAuthenticated = true;
    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::ExecuteCommand
 * @brief Executes command on remote server
 * 
 * Opens SSH channel, executes command, captures output and exit code.
 * 
 * @param stringCommand Command to execute
 * @param stringOutput Output buffer for command results
 * @param iExitCode Exit code from command
 * @return bool True if execution succeeded
 */
bool CSSHClient::ExecuteCommand( const std::string& stringCommand,
                                  std::string& stringOutput,
                                  int& iExitCode )
{
    if( m_bAuthenticated == false )                                            // Not authenticated
    {
        setLastError( "Not authenticated" );
        return false;
    }

    // Open SSH channel
    LIBSSH2_CHANNEL* pchannelSSH = libssh2_channel_open_session( m_psessionSSH );
    if( pchannelSSH == nullptr )
    {
        setLastError( "Failed to open SSH channel" );
        return false;
    }

    // Execute command
    int iResult = libssh2_channel_exec( pchannelSSH, stringCommand.c_str() );
    if( iResult != 0 )
    {
        libssh2_channel_free( pchannelSSH );
        setLastError( "Failed to execute command" );
        return false;
    }

    // Read output
    std::stringstream ssOutput;
    char acBuffer[4096];
    ssize_t iBytesRead = 0;

    while( true )
    {
        iBytesRead = libssh2_channel_read( pchannelSSH, 
                                            acBuffer, 
                                            sizeof( acBuffer ) );
        
        if( iBytesRead > 0 )                                                   // Data received
        {
            ssOutput.write( acBuffer, iBytesRead );
        }
        else if( iBytesRead == 0 )                                             // EOF reached
        {
            break;
        }
        else                                                                   // Error occurred
        {
            if( iBytesRead != LIBSSH2_ERROR_EAGAIN )
            {
                break;
            }
        }
    }

    stringOutput = ssOutput.str();

    // Get exit status
    iExitCode = libssh2_channel_get_exit_status( pchannelSSH );

    // Close and free channel
    libssh2_channel_close( pchannelSSH );
    libssh2_channel_wait_closed( pchannelSSH );
    libssh2_channel_free( pchannelSSH );

    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::Disconnect
 * @brief Disconnects from SSH server
 */
void CSSHClient::Disconnect()
{
    cleanup();
    m_bConnected = false;
    m_bAuthenticated = false;
}

/*----------------------------------------------------------------------------- CSSHClient::IsConnected
 * @brief Checks if connected to SSH server
 * 
 * @return bool True if connected
 */
bool CSSHClient::IsConnected() const
{
    return m_bConnected;
}

/*----------------------------------------------------------------------------- CSSHClient::GetLastError
 * @brief Retrieves last error message
 * 
 * @return std::string Last error description
 */
std::string CSSHClient::GetLastError() const
{
    return m_stringLastError;
}

/*----------------------------------------------------------------------------- CSSHClient::initializeSocket
 * @brief Creates and connects TCP socket
 * 
 * @return bool True if socket initialization succeeded
 */
bool CSSHClient::initializeSocket()
{
    struct addrinfo hints;
    struct addrinfo* presult = nullptr;
    
    std::memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;                                               // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;                                           // TCP socket
    hints.ai_protocol = IPPROTO_TCP;

    std::string stringPort = std::to_string( m_uPort );
    int iResult = getaddrinfo( m_stringHost.c_str(), 
                               stringPort.c_str(), 
                               &hints, 
                               &presult );

    if( iResult != 0 )
    {
        setLastError( "Failed to resolve hostname" );
        return false;
    }

    // Try each address until successful
    struct addrinfo* pcurrent = nullptr;
    for( pcurrent = presult; pcurrent != nullptr; pcurrent = pcurrent->ai_next )
    {
        m_socket = socket( pcurrent->ai_family, pcurrent->ai_socktype, pcurrent->ai_protocol );

        if( m_socket == INVALID_SOCKET_VALUE ) {continue; }

        iResult = connect( m_socket, pcurrent->ai_addr, static_cast<int>( pcurrent->ai_addrlen ) );

        if( iResult == 0 )                                                     // Connection successful
        {
            break;
        }

        closesocket( m_socket );
        m_socket = INVALID_SOCKET_VALUE;
    }

    freeaddrinfo( presult );

    if( m_socket == INVALID_SOCKET_VALUE )
    {
        setLastError( "Failed to connect to host" );
        return false;
    }

    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::initializeSSH
 * @brief Initializes SSH session on connected socket
 * 
 * @return bool True if SSH initialization succeeded
 */
bool CSSHClient::initializeSSH()
{
    m_psessionSSH = libssh2_session_init();
    if( m_psessionSSH == nullptr )
    {
        setLastError( "Failed to create SSH session" );
        return false;
    }

    // Set blocking mode
    libssh2_session_set_blocking( m_psessionSSH, 1 );

    // Perform SSH handshake
    int iResult = libssh2_session_handshake( m_psessionSSH, m_socket );
    if( iResult != 0 )
    {
        char* pszError = nullptr;
        int iErrorLen = 0;
        libssh2_session_last_error( m_psessionSSH, &pszError, &iErrorLen, 0 );
        std::string stringError = "SSH handshake failed: ";
        if( pszError != nullptr )
        {
            stringError += std::string( pszError, iErrorLen );
        }
        setLastError( stringError );
        return false;
    }

    return true;
}

/*----------------------------------------------------------------------------- CSSHClient::cleanup
 * @brief Cleans up SSH session and socket resources
 */
void CSSHClient::cleanup()
{
    if( m_psessionSSH != nullptr )
    {
        libssh2_session_disconnect( m_psessionSSH, "Normal shutdown" );
        libssh2_session_free( m_psessionSSH );
        m_psessionSSH = nullptr;
    }

    if( m_socket != INVALID_SOCKET_VALUE )
    {
        closesocket( m_socket );
        m_socket = INVALID_SOCKET_VALUE;
    }
}

/*----------------------------------------------------------------------------- CSSHClient::setLastError
 * @brief Sets last error message
 * 
 * @param stringError Error message to store
 */
void CSSHClient::setLastError( const std::string& stringError )
{
    m_stringLastError = stringError;
}

/*----------------------------------------------------------------------------- main.cpp
 * Example usage of CSSHClient
 */

/*
// #include "SSHClient.h"
#include <iostream>

int main()
{
    CSSHClient sshclientRemote;

    // Connect to remote server
    std::cout << "Connecting to SSH server..." << std::endl;
    if( sshclientRemote.Connect( "192.168.1.100", 22 ) == false )
    {
        std::cerr << "Connection failed: " 
                  << sshclientRemote.GetLastError() 
                  << std::endl;
        return 1;
    }

    std::cout << "Connected successfully!" << std::endl;

    // Authenticate with password
    if( sshclientRemote.Authenticate( "username", "password" ) == false )
    {
        std::cerr << "Authentication failed: " 
                  << sshclientRemote.GetLastError() 
                  << std::endl;
        sshclientRemote.Disconnect();
        return 1;
    }

    std::cout << "Authenticated successfully!" << std::endl;

    // Execute command
    std::string stringOutput;
    int iExitCode = 0;
    
    if( sshclientRemote.ExecuteCommand( "ls -la /home", 
                                         stringOutput, 
                                         iExitCode ) == true )
    {
        std::cout << "Command output:" << std::endl;
        std::cout << stringOutput << std::endl;
        std::cout << "Exit code: " << iExitCode << std::endl;
    }
    else
    {
        std::cerr << "Command execution failed: " 
                  << sshclientRemote.GetLastError() 
                  << std::endl;
    }

    // Disconnect
    sshclientRemote.Disconnect();
    std::cout << "Disconnected" << std::endl;

    return 0;
}
*/