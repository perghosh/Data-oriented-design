/**
* \file gd_console_console.cpp
*/


#include <cstring>
#include <iostream>

#ifdef _WIN32
#  include <windows.h>
#endif

#ifndef _WIN32
#  include <sys/ioctl.h>
#  include <unistd.h>
#  include <termios.h>
#endif

#include "gd_console_console.h"

_GD_CONSOLE_BEGIN


// Add this method to the gd::console::progress class
void progress::print_to(const std::string& stringLeft, const std::string& stringFill, const std::string& stringPointer, const std::string& stringRight, std::string& stringBar) const 
{
   // Example: [=====>     ]
   //unsigned percent = this->percent(); // assuming percent() returns 0-100
   unsigned uWidth = get_width();
   //unsigned pos = (percent * bar_width) / 100;

   stringBar += stringLeft;
   for( unsigned u = 0; u < uWidth; ++u ) 
   {
      if(u < m_uValue) { stringBar += stringFill; }
      else if(u == m_uValue) { stringBar += stringPointer; }
      else stringBar += " ";
   }
   stringBar += stringRight;
}


std::pair<bool, std::string> console::initialize() 
{
   if( m_iWidth > 0 && m_iHeight > 0 && m_iBufferWidth > 0 && m_iBufferHeight > 0) 
   {
      return {true, "initialized"};
   } 

   return read_console_information_s( this );
}

/// Set text color using ANSI escape codes
void console::set_foreground_color(int iRed, int iGreen, int iBlue)
{
   // ANSI escape code for 24-bit foreground color: \033[38;2;R;G;Bm
   std::cout << "\033[38;2;" << iRed << ";" << iGreen << ";" << iBlue << "m";
}

/// Set background color using ANSI escape codes
void console::set_background_color(int iRed, int iGreen, int iBlue) 
{
   // ANSI escape code for 24-bit background color: \033[48;2;R;G;B m
   std::cout << "\033[48;2;" << iRed << ";" << iGreen << ";" << iBlue << "m";
}


std::pair<bool, std::string> console::move_to(int iRow, int iColumn)
{
   // Validate coordinates
   if(iRow < 0 || iColumn < 0) { return { false, "Invalid coordinates: negative values not allowed" }; }

   // If console dimensions are known, validate bounds
   if(m_iWidth > 0 && m_iHeight > 0) 
   {
      if(iColumn >= m_iWidth || iRow >= m_iHeight) { return { false, "Coordinates out of console bounds" }; }
   }

#ifdef _WIN32
   // Windows implementation
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) 
   {
      return { false, "Failed to get console handle" };
   }

   COORD coord_;
   coord_.X = static_cast<SHORT>(iColumn);
   coord_.Y = static_cast<SHORT>(iRow);

   if( ::SetConsoleCursorPosition(hConsole, coord_) ) 
   {
      set_xy(iColumn, iRow);                                                  // Update internal state
   } 
   else { return { false, "Failed to set cursor position" }; }
#else
   // POSIX implementation (Linux, macOS, etc.)
   // Use ANSI escape sequence to set cursor position
   // Note: ANSI coordinates are 1-based, so we add 1 to our 0-based coordinates
   char piBuffer[32];
   int iLength = snprintf(piBuffer, sizeof(piBuffer), "\033[%d;%dH", iRow + 1, iColumn + 1);

   if(iLength < 0 || iLength >= static_cast<int>(sizeof(piBuffer))) { return { false, "Failed to format escape sequence" }; }

   ssize_t uBytesWritten = write(STDOUT_FILENO, piBuffer, iLength);
   if(uBytesWritten == iLength) 
   {
      // Update internal state
      set_xy(iColumn, iRow);
      // Flush output to ensure cursor position is immediately updated
      if(fsync(STDOUT_FILENO) == 0 || errno == EINVAL) { return { true, "" }; } 
      else { return { false, "Failed to flush output" }; }
   } 
   else { return { false, "Failed to write cursor position escape sequence" }; }
#endif

   return { true, "" };
}

void console::print( const gd::math::algebra::point<unsigned>& point_, std::string_view stringText)
{
   move_to(point_.x(), point_.y());
   std::cout << stringText;
}

void console::print( const std::string& stringText )
{
   std::cout << stringText;
   // Flush output to ensure immediate display
   std::cout.flush();
}



/** ---------------------------------------------------------------------------
 * @brief Reads and updates the console's size, buffer size, and cursor position.
 *
 * This function queries the underlying terminal or console for its current
 * dimensions (width and height), buffer size, and the current cursor position.
 * It updates the provided console object with these values. The implementation
 * is platform-specific:
 * - On Windows, it uses GetConsoleScreenBufferInfo to retrieve the information.
 * - On POSIX systems, it uses ioctl to get the window size and ANSI escape
 *   codes to query the cursor position.
 *
 * @param pconsole Pointer to the console object to update.
 * @return A pair where the first element is true on success, false on failure.
 *         The second element contains an error message if the operation failed.
 */
std::pair<bool, std::string> console::read_console_information_s( console* pconsole )
{                                                                                                  assert( pconsole != nullptr );
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
         piBuffer[uBytesRead] = '\0';
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

/// Query actual console foreground color (from terminal/console)
std::pair<bool, std::tuple<int, int, int>> console::query_foreground_color_s()
{
#ifdef _WIN32
   // Windows implementation
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) { return { false, {0, 0, 0} }; }

   CONSOLE_SCREEN_BUFFER_INFOEX csbiex_ = {};
   csbiex_.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
   
   if(GetConsoleScreenBufferInfoEx(hConsole, &csbiex_))
   {
      // Extract foreground color index from attributes
      WORD foregroundIndex = csbiex_.wAttributes & 0x0F;
      
      // Get RGB values from color table
      COLORREF uColor = csbiex_.ColorTable[foregroundIndex];
      int iRed = GetRValue(uColor);
      int iGreen = GetGValue(uColor);
      int iBlue = GetBValue(uColor);
      
      return { true, {iRed, iGreen, iBlue} };
   }
#else
   // POSIX implementation using OSC escape sequences
   // Save current terminal settings
   struct termios termiosOld, termiosNew;
   if(tcgetattr(STDIN_FILENO, &termiosOld) != 0) { return { false, {0, 0, 0} }; }

   // Set terminal to raw mode for reading response
   termiosNew = termiosOld;
   termiosNew.c_lflag &= ~(ICANON | ECHO);
   termiosNew.c_cc[VMIN] = 0;   // Non-blocking read
   termiosNew.c_cc[VTIME] = 10;  // 1 second timeout
   
   if(tcsetattr(STDIN_FILENO, TCSANOW, &termiosNew) != 0) { return { false, {0, 0, 0} }; }

   // Query foreground color using OSC 10
   const char* piszQuery = "\033]10;?\007";
   if(write(STDOUT_FILENO, piszQuery, strlen(piszQuery)) != static_cast<ssize_t>(strlen(piszQuery)))
   {
      tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
      return { false, {0, 0, 0} };
   }

   // Read response
   char piBuffer[256];
   memset(piBuffer, 0, sizeof(piBuffer));
   ssize_t uBytesRead = read(STDIN_FILENO, piBuffer, sizeof(piBuffer) - 1);
   
   // Restore terminal settings
   tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
   
   if(uBytesRead > 0)
   {
      piBuffer[uBytesRead] = '\0';
      
      // Parse response format: \033]10;rgb:rrrr/gggg/bbbb\007
      // or \033]10;#rrggbb\007
      char* piColorStart = strstr(piBuffer, "rgb:");
      if(piColorStart != nullptr)
      {
         unsigned int r16, g16, b16;
         if(sscanf(piColorStart, "rgb:%x/%x/%x", &r16, &g16, &b16) == 3)
         {
            // Convert from 16-bit to 8-bit values
            int iRed = (r16 >> 8) & 0xFF;
            int iGreen = (g16 >> 8) & 0xFF;  
            int iBlue = (b16 >> 8) & 0xFF;
            return { true, {iRed, iGreen, iBlue} };

         }
      }
      else
      {
         // Try hex format
         piColorStart = strchr(piBuffer, '#');
         if(piColorStart)
         {
            unsigned int uRGB;
            if(sscanf(piColorStart, "#%x", &uRGB) == 1)
            {
               int iRed = (uRGB >> 16) & 0xFF;
               int iGreen = (uRGB >> 8) & 0xFF;
               int iBlue = uRGB & 0xFF;
               return { true, {iRed, iGreen, iBlue} };
            }
         }
      }
   }
#endif
   return { false, {0, 0, 0} };
}

/// Query actual console background color (from terminal/console)
std::pair<bool, std::tuple<int, int, int>> console::query_background_color_s()
{
#ifdef _WIN32
   // Windows implementation
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) { return { false, {0, 0, 0} }; }

   CONSOLE_SCREEN_BUFFER_INFOEX csbiex_ = {};
   csbiex_.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
   
   if(GetConsoleScreenBufferInfoEx(hConsole, &csbiex_))
   {
      // Extract background color index from attributes
      WORD backgroundIndex = (csbiex_.wAttributes & 0xF0) >> 4;
      
      // Get RGB values from color table
      COLORREF uColor = csbiex_.ColorTable[backgroundIndex];
      int iRed = GetRValue(uColor);
      int iGreen = GetGValue(uColor);
      int iBlue = GetBValue(uColor);

      return { true, {iRed, iGreen, iBlue} };
   }
#else
   // POSIX implementation using OSC escape sequences
   // Save current terminal settings
   struct termios termiosOld, termiosNew;
   if(tcgetattr(STDIN_FILENO, &termiosOld) != 0) { return { false, {0, 0, 0} }; }

   // Set terminal to raw mode for reading response
   termiosNew = termiosOld;
   termiosNew.c_lflag &= ~(ICANON | ECHO);
   termiosNew.c_cc[VMIN] = 0;   // Non-blocking read
   termiosNew.c_cc[VTIME] = 10;  // 1 second timeout
   
   if(tcsetattr(STDIN_FILENO, TCSANOW, &termiosNew) != 0) {return { false, {0, 0, 0} };}

   // Query background color using OSC 11
   const char* query = "\033]11;?\007";
   if(write(STDOUT_FILENO, query, strlen(query)) != static_cast<ssize_t>(strlen(query)))
   {
      tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
      return { false, {0, 0, 0} };
   }

   // Read response
   char piBuffer[256];
   memset(piBuffer, 0, sizeof(piBuffer));
   ssize_t uBytesRead = read(STDIN_FILENO, piBuffer, sizeof(piBuffer) - 1);

   // Restore terminal settings
   tcsetattr(STDIN_FILENO, TCSANOW, &termiosOld);
   
   if(uBytesRead > 0)
   {
      piBuffer[uBytesRead] = '\0';
      
      // Parse response format: \033]11;rgb:rrrr/gggg/bbbb\007
      // or \033]11;#rrggbb\007
      char* piColorStart = strstr(piBuffer, "rgb:");
      if(piColorStart)
      {
         unsigned int r16, g16, b16;
         if(sscanf(piColorStart, "rgb:%x/%x/%x", &r16, &g16, &b16) == 3)
         {
            // Convert from 16-bit to 8-bit values
            int iRed = (r16 >> 8) & 0xFF;
            int iGreen = (g16 >> 8) & 0xFF;  
            int iBlue = (b16 >> 8) & 0xFF;
            return { true, {iRed, iGreen, iBlue} };
         }
      }
      else
      {
         // Try hex format
         piColorStart = strchr(piBuffer, '#');
         if(piColorStart != nullptr)
         {
            unsigned int uRGB;
            if(sscanf(piColorStart, "#%x", &uRGB) == 1)
            {
               int iRed = (uRGB >> 16) & 0xFF;
               int iGreen = (uRGB >> 8) & 0xFF;
               int iBlue = uRGB & 0xFF;
               return { true, {iRed, iGreen, iBlue} };
            }
         }
      }
   }
#endif
   return { false, {0, 0, 0} };
}


/// Clear entire current line
std::pair<bool, std::string> console::clear_line_s()
{
#ifdef _WIN32
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) { return { false, "Failed to get console handle" }; }

   CONSOLE_SCREEN_BUFFER_INFO csbi_;
   if(!GetConsoleScreenBufferInfo(hConsole, &csbi_)) { return { false, "Failed to get console screen buffer info" }; }

   COORD coords_ = { 0, csbi_.dwCursorPosition.Y };
   DWORD uCharsToWrite = csbi_.dwSize.X;
   DWORD uCharsWritten;

   if(!FillConsoleOutputCharacter(hConsole, ' ', uCharsToWrite, coords_, &uCharsWritten)) { return { false, "Failed to clear line" }; }
   
   // Move cursor to beginning of line
   if(!SetConsoleCursorPosition(hConsole, coords_)) { return { false, "Failed to move cursor to beginning of line" }; }
#else
   // POSIX implementation
   const char* piszClearSequence = "\033[2K\r";  // Clear line and return to beginning
   ssize_t uBytesWritten = write(STDOUT_FILENO, piszClearSequence, strlen(piszClearSequence));
   
   if(uBytesWritten != static_cast<ssize_t>(strlen(piszClearSequence))) { return { false, "Failed to write clear sequence" }; }
#endif

   return { true, "" };
}


/// Read characters from console buffer at specified position
std::pair<bool, std::string> console::read_text_s(int iStartX, int iStartY, int iLength)
{
   // Validate parameters
   if(iStartX < 0 || iStartY < 0 || iLength <= 0) { return { false, "" }; }

#ifdef _WIN32
   // Windows implementation using ReadConsoleOutputCharacter
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if(hConsole == INVALID_HANDLE_VALUE) { return { false, "" }; }

   // Allocate buffer for characters
   std::vector<TCHAR> vectorBuffer(iLength + 1);
   COORD readCoord = { static_cast<SHORT>(iStartX), static_cast<SHORT>(iStartY) };
   DWORD uCharsRead = 0;

   if(ReadConsoleOutputCharacter(hConsole, vectorBuffer.data(), iLength, readCoord, &uCharsRead))
   {
      vectorBuffer[uCharsRead] = '\0';
      std::string stringResult(vectorBuffer.begin(), vectorBuffer.begin() + uCharsRead);
      
      // Remove trailing spaces (common in console buffers)
      while (!stringResult.empty() && stringResult.back() == ' ') {
         stringResult.pop_back();
      }
      
      return { true, stringResult };
   }

#else
   assert(false && "POSIX console buffer reading not implemented yet");
#endif
   return { false, "" };
}


_GD_CONSOLE_END



