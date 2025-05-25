#ifdef _WIN32
#  include <windows.h>
#endif

#ifndef _WIN32
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

#include "gd_test.h"

_GD_CONSOLE_BEGIN


std::pair<bool, std::string> console::initialize() 
{
   if( m_iWidth > 0 && m_iHeight > 0 && m_iBufferWidth > 0 && m_iBufferHeight > 0) 
   {
      return {true, "initialized"};
   } 

   return read_console_information_s( this );
}


std::pair<bool, std::string> console::read_console_information_s( console* pconsole )
{

#ifdef _WIN32
   // Windows implementation
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_SCREEN_BUFFER_INFO csbi;

   if( GetConsoleScreenBufferInfo(hConsole, &csbi)) {
      // Console size (visible window)
      int iWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      int iHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
      pconsole->set_size( iWidth, iHeight );

      // Cursor position
      int iCursorX = csbi.dwCursorPosition.X;
      int iCursorY = csbi.dwCursorPosition.Y;
      pconsole->set_xy( iCursorX, iCursorY );

      // Buffer size
      int iBufferWidth = csbi.dwSize.X;
      int iBufferHeight = csbi.dwSize.Y;
      pconsole->set_buffer_size( iBufferWidth, iBufferHeight );
   }
#else
   // POSIX implementation (Linux, macOS, etc.)
   struct winsize winsize_;
   if( ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize_) != -1) 
   {
      pconsole->set_size(winsize_.ws_col, winsize_.ws_row);                    // Console size
      pconsole->set_buffer_size(winsize_.ws_col, winsize_.ws_row);             // Buffer size (same as console size in most POSIX terminals)
   } 
   else { return { false, "Failed to get terminal window size" };  }

   // Cursor position (using ANSI escape codes)
   // Save current terminal settings
   struct termios termiosOld, termiosNew;
   if(tcgetattr(STDIN_FILENO, &termiosOld) != 0) { return { false, "Failed to get terminal attributes" }; }

   // Set terminal to raw mode for reading cursor position
   termiosNew = termiosOld;
   termiosNew.c_lflag &= ~(ICANON | ECHO);
   if(tcsetattr(STDIN_FILENO, TCSANOW, &termiosNew) != 0) {
      return { false, "Failed to set terminal attributes" };
   }

   // Query cursor position
   char buf[32];
   memset(buf, 0, sizeof(buf));
   unsigned int x = 0, y = 0;

   // Send cursor position query
   if(write(STDOUT_FILENO, "\033[6n", 4) == 4) 
   {
      // Read response
      ssize_t bytes_read = read(STDIN_FILENO, buf, sizeof(buf) - 1);
      if(bytes_read > 0) 
      {
         buf[bytes_read] = '\0';
         if(sscanf(buf, "\033[%u;%uR", &y, &x) == 2) { pconsole->set_xy(x - 1, y - 1); } // Adjust for 0-based indexing 
         else 
         {  // Restore terminal settings before returning error
            tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
            return { false, "Failed to parse cursor position response" };
         }
      } 
      else 
      {
         // Restore terminal settings before returning error
         tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
         return { false, "Failed to read cursor position response" };
      }
   } 
   else 
   {
      // Restore terminal settings before returning error
      tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
      return { false, "Failed to send cursor position query" };
   }

   // Restore terminal settings
   if(tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld) != 0) {return { false, "Failed to restore terminal attributes" }; }
#endif


   return { true, "" };
}



_GD_CONSOLE_END



/*

#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Structure to hold console dimensions
struct ConsoleInfo {
    int width;
    int height;
    int cursorX;
    int cursorY;
    int bufferWidth;
    int bufferHeight;
};

ConsoleInfo getConsoleInfo() {
    ConsoleInfo info = {0, 0, 0, 0, 0, 0};

#ifdef _WIN32
    // Windows implementation
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        // Console size (visible window)
        info.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        info.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        // Cursor position
        info.cursorX = csbi.dwCursorPosition.X;
        info.cursorY = csbi.dwCursorPosition.Y;

        // Buffer size
        info.bufferWidth = csbi.dwSize.X;
        info.bufferHeight = csbi.dwSize.Y;
    }
#else
    // POSIX implementation (Linux, macOS, etc.)
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
        // Console size
        info.width = ws.ws_col;
        info.height = ws.ws_row;

        // Buffer size (same as console size in most POSIX terminals)
        info.bufferWidth = ws.ws_col;
        info.bufferHeight = ws.ws_row;
    }

    // Cursor position (using ANSI escape codes)
    char buf[32];
    unsigned int x = 0, y = 0;
    write(STDOUT_FILENO, "\033[6n", 4); // Query cursor position
    if(read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
        if(sscanf(buf, "\033[%u;%uR", &y, &x) == 2) {
            info.cursorX = x - 1; // Adjust for 0-based indexing
            info.cursorY = y - 1;
        }
    }
#endif

    return info;
}

int main() {
    ConsoleInfo info = getConsoleInfo();

    std::cout << "Console Width: " << info.width << std::endl;
    std::cout << "Console Height: " << info.height << std::endl;
    std::cout << "Cursor X: " << info.cursorX << std::endl;
    std::cout << "Cursor Y: " << info.cursorY << std::endl;
    std::cout << "Buffer Width: " << info.bufferWidth << std::endl;
    std::cout << "Buffer Height: " << info.bufferHeight << std::endl;

    return 0;
}



*/

