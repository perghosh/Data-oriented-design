#include <cstring>

#ifdef _WIN32
#  include <windows.h>
#endif

#ifndef _WIN32
#  include <sys/ioctl.h>
#  include <unistd.h>
#  include <termios.h>
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

std::pair<bool, std::string> console::move_to(int iX, int iY)
{
   // Validate coordinates
   if (iX < 0 || iY < 0) {
      return { false, "Invalid coordinates: negative values not allowed" };
   }

   // If console dimensions are known, validate bounds
   if (m_iWidth > 0 && m_iHeight > 0) {
      if (iX >= m_iWidth || iY >= m_iHeight) {
         return { false, "Coordinates out of console bounds" };
      }
   }

#ifdef _WIN32
   // Windows implementation
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) 
   {
      return { false, "Failed to get console handle" };
   }

   COORD coord;
   coord.X = static_cast<SHORT>(iX);
   coord.Y = static_cast<SHORT>(iY);

   if( ::SetConsoleCursorPosition(hConsole, coord) ) 
   {
      // Update internal state
      set_xy(iX, iY);
   } 
   else 
   {
      return { false, "Failed to set cursor position" };
   }
#else
   // POSIX implementation (Linux, macOS, etc.)
   // Use ANSI escape sequence to set cursor position
   // Note: ANSI coordinates are 1-based, so we add 1 to our 0-based coordinates
   char piBuffer[32];
   int iLength = snprintf(piBuffer, sizeof(piBuffer), "\033[%d;%dH", iY + 1, iX + 1);

   if(iLength < 0 || iLength >= static_cast<int>(sizeof(piBuffer))) { return { false, "Failed to format escape sequence" }; }

   ssize_t uBytesWritten = write(STDOUT_FILENO, piBuffer, len);
   if(uBytesWritten == iLength) 
   {
      // Update internal state
      set_xy(iX, iY);
      // Flush output to ensure cursor position is immediately updated
      if(fsync(STDOUT_FILENO) == 0 || errno == EINVAL) { return { true, "" }; } 
      else { return { false, "Failed to flush output" }; }
   } 
   else { return { false, "Failed to write cursor position escape sequence" }; }
#endif

   return { true, "" };
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
   if(tcsetattr(STDIN_FILENO, TCSANOW, &termiosNew) != 0) { return { false, "Failed to set terminal attributes" }; }

   // Query cursor position
   char piBuffer[32];
   memset(piBuffer, 0, sizeof(piBuffer));
   unsigned int uX = 0, uY = 0;

   // Send cursor position query
   if(write(STDOUT_FILENO, "\033[6n", 4) == 4) 
   {
      ssize_t uBytesRead = read(STDIN_FILENO, piBuffer, sizeof(piBuffer) - 1); // Read response
      if(uBytesRead > 0) 
      {
         buf[uBytesRead] = '\0';
         if(sscanf(piBuffer, "\033[%u;%uR", &uY, &uX) == 2) { pconsole->set_xy(uX - 1, uY - 1); } // Adjust for 0-based indexing 
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



