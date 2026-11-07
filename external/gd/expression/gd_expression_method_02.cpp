#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
   #ifndef NOMINMAX
   #define NOMINMAX
   #endif
   #ifndef WIN32_LEAN_AND_MEAN
   #define WIN32_LEAN_AND_MEAN
   #endif
   #include <windows.h>
   #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
   #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
   #endif
#else
   #include <sys/ioctl.h>
   #include <termios.h>
   #include <unistd.h>
#endif

#include "gd_expression_method_02.h"

_GD_EXPRESSION_BEGIN

//============================================================================
//=============================================================== console tools
//============================================================================

namespace {

   enum enumConsoleStream { eConsoleStreamInput, eConsoleStreamOutput };

   /// Get value of environment variable, empty string if not found
   std::string console_get_environment_(const char* piName)
   {
#if defined(_WIN32)
      char* piValue = nullptr;
      size_t uLength = 0;
      if( _dupenv_s(&piValue, &uLength, piName) != 0 || piValue == nullptr ) return std::string();
      std::string stringValue(piValue);
      std::free(piValue);
      return stringValue;
#else
      const char* piValue = std::getenv(piName);
      return piValue != nullptr ? std::string(piValue) : std::string();
#endif
   }

   /// Check if standard input or standard output is connected to an interactive terminal
   bool console_is_terminal_(enumConsoleStream eStream)
   {
#if defined(_WIN32)
      HANDLE handleStream = GetStdHandle(eStream == eConsoleStreamInput ? STD_INPUT_HANDLE : STD_OUTPUT_HANDLE);
      DWORD uMode = 0;
      return handleStream != INVALID_HANDLE_VALUE && handleStream != nullptr && GetConsoleMode(handleStream, &uMode) != 0;
#else
      return ::isatty(eStream == eConsoleStreamInput ? STDIN_FILENO : STDOUT_FILENO) != 0;
#endif
   }

   /// Check if ANSI escape sequences can be written to standard output. On windows virtual terminal
   /// processing is enabled the first time this is called. The result is cached.
   bool console_is_ansi_(void)
   {
      static const bool bAnsi = []() -> bool
      {
         if( console_is_terminal_(eConsoleStreamOutput) == false ) return false;
#if defined(_WIN32)
         HANDLE handleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
         DWORD uMode = 0;
         if( GetConsoleMode(handleOutput, &uMode) == 0 ) return false;
         if( ( uMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING ) != 0 ) return true;
         return SetConsoleMode(handleOutput, uMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
         std::string stringTerminal = console_get_environment_("TERM");
         return stringTerminal.empty() == false && stringTerminal != "dumb";
#endif
      }();
      return bAnsi;
   }

   /// Get size of terminal in characters, returns false if size is unknown
   bool console_get_size_(unsigned* puWidth, unsigned* puHeight)
   {
#if defined(_WIN32)
      CONSOLE_SCREEN_BUFFER_INFO infoScreenBuffer;
      HANDLE handleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      if( GetConsoleScreenBufferInfo(handleOutput, &infoScreenBuffer) == 0 ) return false;
      *puWidth  = static_cast<unsigned>( infoScreenBuffer.srWindow.Right - infoScreenBuffer.srWindow.Left + 1 );
      *puHeight = static_cast<unsigned>( infoScreenBuffer.srWindow.Bottom - infoScreenBuffer.srWindow.Top + 1 );
      return true;
#else
      struct winsize winsizeTerminal;
      if( ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsizeTerminal) != 0 ) return false;
      if( winsizeTerminal.ws_col == 0 || winsizeTerminal.ws_row == 0 ) return false;
      *puWidth  = winsizeTerminal.ws_col;
      *puHeight = winsizeTerminal.ws_row;
      return true;
#endif
   }

   /// Read one line from standard input, trailing carriage return is removed. Returns false on end of input
   bool console_read_line_(std::string* pstringLine)
   {
      if( !std::getline(std::cin, *pstringLine) ) return false;
      if( pstringLine->empty() == false && pstringLine->back() == '\r' ) pstringLine->pop_back();
      return true;
   }

   /// Read one line from standard input without echo if input is a terminal
   bool console_read_secret_(std::string* pstringSecret)
   {
#if defined(_WIN32)
      HANDLE handleInput = GetStdHandle(STD_INPUT_HANDLE);
      DWORD uMode = 0;
      bool bTerminal = GetConsoleMode(handleInput, &uMode) != 0;
      if( bTerminal ) SetConsoleMode(handleInput, uMode & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
      bool bRead = console_read_line_(pstringSecret);
      if( bTerminal ) { SetConsoleMode(handleInput, uMode); std::cout << '\n'; }
      return bRead;
#else
      struct termios termiosOriginal;
      bool bTerminal = ::isatty(STDIN_FILENO) != 0 && ::tcgetattr(STDIN_FILENO, &termiosOriginal) == 0;
      if( bTerminal )
      {
         struct termios termiosSilent = termiosOriginal;
         termiosSilent.c_lflag &= ~static_cast<tcflag_t>(ECHO);
         ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &termiosSilent);
      }
      bool bRead = console_read_line_(pstringSecret);
      if( bTerminal ) { ::tcsetattr(STDIN_FILENO, TCSANOW, &termiosOriginal); std::cout << '\n'; }
      return bRead;
#endif
   }

   /// Write prompt text to standard output and flush so it is visible before waiting for input
   void console_write_prompt_(const std::string& stringPrompt)
   {
      std::cout << stringPrompt << std::flush;
   }

   /// Join all values as text separated by a space
   std::string console_join_(const std::vector<value>& vectorArgument)
   {
      std::string stringResult;
      for( const auto& valueArgument : vectorArgument )
      {
         if( stringResult.empty() == false ) stringResult += ' ';
         stringResult += valueArgument.as_string();
      }
      return stringResult;
   }

   /// Trim whitespace from both ends
   std::string console_trim_(const std::string& stringText)
   {
      size_t uBegin = 0;
      size_t uEnd = stringText.length();
      while( uBegin < uEnd && std::isspace(static_cast<unsigned char>(stringText[uBegin])) ) ++uBegin;
      while( uEnd > uBegin && std::isspace(static_cast<unsigned char>(stringText[uEnd - 1])) ) --uEnd;
      return stringText.substr(uBegin, uEnd - uBegin);
   }

   /// Convert text to lowercase
   std::string console_lower_(std::string stringText)
   {
      for( auto& cCharacter : stringText ) cCharacter = static_cast<char>( std::tolower(static_cast<unsigned char>(cCharacter)) );
      return stringText;
   }

   /// Parse text as integer or floating point number, returns false if text is not a number
   bool console_parse_number_(const std::string& stringText, value* pvalueNumber)
   {
      if( stringText.empty() ) return false;

      const char* piBegin = stringText.c_str();
      char* piEnd = nullptr;

      errno = 0;
      long long iNumber = std::strtoll(piBegin, &piEnd, 10);
      if( errno == 0 && piEnd != piBegin && *piEnd == '\0' )
      {
         *pvalueNumber = static_cast<int64_t>(iNumber);
         return true;
      }

      errno = 0;
      double dNumber = std::strtod(piBegin, &piEnd);
      if( errno != 0 || piEnd == piBegin || *piEnd != '\0' || std::isfinite(dNumber) == false ) return false;
      *pvalueNumber = value(dNumber);
      return true;
   }

   /// Get ANSI SGR code for style name, returns -1 if name is unknown
   int console_get_style_code_(const std::string& stringName)
   {
      struct style { const char* m_piName; int m_iCode; };
      static const style pstyleName[] = {
         { "bold", 1 }, { "dim", 2 }, { "italic", 3 }, { "underline", 4 },
         { "black", 30 }, { "red", 31 }, { "green", 32 }, { "yellow", 33 },
         { "blue", 34 }, { "magenta", 35 }, { "cyan", 36 }, { "white", 37 },
         { "gray", 90 }, { "bright_red", 91 }, { "bright_green", 92 }, { "bright_yellow", 93 },
         { "bright_blue", 94 }, { "bright_magenta", 95 }, { "bright_cyan", 96 }, { "bright_white", 97 },
         { "bg_black", 40 }, { "bg_red", 41 }, { "bg_green", 42 }, { "bg_yellow", 43 },
         { "bg_blue", 44 }, { "bg_magenta", 45 }, { "bg_cyan", 46 }, { "bg_white", 47 },
      };

      for( const auto& styleName : pstyleName )
      {
         if( stringName == styleName.m_piName ) return styleName.m_iCode;
      }
      return -1;
   }

} // anonymous namespace


//============================================================================
//============================================================= console methods
//============================================================================

/// Ring the terminal bell, nothing is written if output is not a terminal.
std::pair<bool, std::string> beep_g(const std::vector<value>& vectorArgument)
{
   if( console_is_terminal_(eConsoleStreamOutput) == true ) { std::cout << '\a' << std::flush; }
   return { true, "" };
}

/// Ask user to select one of the options. Options are listed as a numbered menu and the
/// selected number is returned (1 for first option). Asks again until a valid number is entered.
std::pair<bool, std::string> choose_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.size() < 2 ) return { false, "choose requires a prompt and at least 1 option" };

   const size_t uOptionCount = vectorArgument.size() - 1;

   std::cout << vectorArgument[0].as_string() << '\n';
   for( size_t u = 1; u < vectorArgument.size(); ++u )
   {
      std::cout << "  " << u << ") " << vectorArgument[u].as_string() << '\n';
   }

   while( true )
   {
      console_write_prompt_("> ");
      std::string stringAnswer;
      if( console_read_line_(&stringAnswer) == false ) return { false, "choose - end of input stream" };

      value valueNumber;
      if( console_parse_number_(console_trim_(stringAnswer), &valueNumber) == true && valueNumber.is_integer() == true )
      {
         int64_t iSelected = valueNumber.as_integer();
         if( iSelected >= 1 && iSelected <= static_cast<int64_t>(uOptionCount) )
         {
            *pvalueResult = iSelected;
            return { true, "" };
         }
      }

      std::cout << "Enter a number between 1 and " << uOptionCount << '\n';
   }
}

/// Clear the screen and move cursor to top left, nothing is done if output is not a terminal.
std::pair<bool, std::string> clear_g(const std::vector<value>& vectorArgument)
{
   if( console_is_ansi_() == true ) { std::cout << "\x1b[2J\x1b[H" << std::flush; }
   return { true, "" };
}

/// Return text wrapped in ANSI style codes. Style is one or more names separated by space, comma or plus,
/// for example "red", "bold green" or "white,bg_blue". If output is not a terminal, or environment
/// variable NO_COLOR is set, the text is returned unchanged.
std::pair<bool, std::string> color_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.size() < 2 ) return { false, "color requires 2 arguments" };

   std::string stringText = vectorArgument[0].as_string();
   std::string stringStyle = vectorArgument[1].as_string();

   // ## Split style into names and find the code for each name
   std::string stringCodes;
   std::string stringName;
   stringStyle += ' ';                                                        // add terminator so last name is handled in the loop
   for( char cCharacter : stringStyle )
   {
      if( cCharacter != ' ' && cCharacter != ',' && cCharacter != '+' ) { stringName += static_cast<char>( std::tolower(static_cast<unsigned char>(cCharacter)) ); continue; }
      if( stringName.empty() ) continue;

      int iCode = console_get_style_code_(stringName);
      if( iCode < 0 ) return { false, "color - unknown style: " + stringName };
      if( stringCodes.empty() == false ) stringCodes += ';';
      stringCodes += std::to_string(iCode);
      stringName.clear();
   }

   if( stringCodes.empty() ) return { false, "color - style is empty" };

   bool bPlain = console_is_ansi_() == false || console_get_environment_("NO_COLOR").empty() == false;
   if( bPlain == true ) { *pvalueResult = value(stringText); }
   else { *pvalueResult = value("\x1b[" + stringCodes + "m" + stringText + "\x1b[0m"); }

   return { true, "" };
}

/// Ask a yes/no question and return answer as boolean. Empty answer returns the default value
/// (second argument). Accepts y, yes, true, 1 and n, no, false, 0. Asks again for any other answer.
std::pair<bool, std::string> confirm_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.size() < 2 ) return { false, "confirm requires 2 arguments" };

   const bool bDefault = vectorArgument[1].as_bool();
   const std::string stringPrompt = vectorArgument[0].as_string() + ( bDefault ? " [Y/n] " : " [y/N] " );

   while( true )
   {
      console_write_prompt_(stringPrompt);
      std::string stringAnswer;
      if( console_read_line_(&stringAnswer) == false ) return { false, "confirm - end of input stream" };

      stringAnswer = console_lower_( console_trim_(stringAnswer) );
      if( stringAnswer.empty() ) { *pvalueResult = value(bDefault); return { true, "" }; }
      if( stringAnswer == "y" || stringAnswer == "yes" || stringAnswer == "true" || stringAnswer == "1" )  { *pvalueResult = value(true); return { true, "" }; }
      if( stringAnswer == "n" || stringAnswer == "no" || stringAnswer == "false" || stringAnswer == "0" ) { *pvalueResult = value(false); return { true, "" }; }

      std::cout << "Please answer y or n\n";
   }
}

/// Print values separated by a space and a newline to standard error.
std::pair<bool, std::string> error_g(const std::vector<value>& vectorArgument)
{
   std::cout << std::flush;                                                   // keep order between standard output and standard error
   std::cerr << console_join_(vectorArgument) << std::endl;
   return { true, "" };
}

/// Show prompt and read one line as string.
std::pair<bool, std::string> input_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "input requires 1 argument" };

   console_write_prompt_(vectorArgument[0].as_string());
   std::string stringInput;
   if( console_read_line_(&stringInput) == false ) return { false, "input - end of input stream" };

   *pvalueResult = value(stringInput);
   return { true, "" };
}

/// Show prompt and read a number. Returns integer if input is a whole number, otherwise double.
/// If input is empty or not a number, the default (second argument) is returned.
std::pair<bool, std::string> input_number_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.size() < 2 ) return { false, "input_number requires 2 arguments" };

   console_write_prompt_(vectorArgument[0].as_string());
   std::string stringInput;
   if( console_read_line_(&stringInput) == false ) return { false, "input_number - end of input stream" };

   value valueNumber;
   if( console_parse_number_(console_trim_(stringInput), &valueNumber) == true ) { *pvalueResult = valueNumber; }
   else { *pvalueResult = vectorArgument[1]; }

   return { true, "" };
}

/// Return true if standard output is an interactive terminal.
std::pair<bool, std::string> is_tty_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   *pvalueResult = value( console_is_terminal_(eConsoleStreamOutput) );
   return { true, "" };
}

/// Show prompt and read one line without echo, used for passwords.
std::pair<bool, std::string> password_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "password requires 1 argument" };

   console_write_prompt_(vectorArgument[0].as_string());
   std::string stringSecret;
   if( console_read_secret_(&stringSecret) == false ) return { false, "password - end of input stream" };

   *pvalueResult = value(stringSecret);
   return { true, "" };
}

/// Wait for enter. Optional argument is the text to show, default is "Press enter to continue...".
/// End of input stream is not an error, it just continues.
std::pair<bool, std::string> pause_g(const std::vector<value>& vectorArgument)
{
   std::string stringPrompt = vectorArgument.empty() ? std::string("Press enter to continue...") : console_join_(vectorArgument);
   console_write_prompt_(stringPrompt);
   std::string stringIgnore;
   console_read_line_(&stringIgnore);
   return { true, "" };
}

/// Print values separated by a space, without newline.
std::pair<bool, std::string> print_g(const std::vector<value>& vectorArgument)
{
   std::cout << console_join_(vectorArgument) << std::flush;
   return { true, "" };
}

/// Print values separated by a space, followed by a newline.
std::pair<bool, std::string> println_g(const std::vector<value>& vectorArgument)
{
   std::cout << console_join_(vectorArgument) << '\n' << std::flush;
   return { true, "" };
}

/// Return number of rows in terminal, 24 if unknown.
std::pair<bool, std::string> term_height_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   unsigned uWidth = 80, uHeight = 24;
   console_get_size_(&uWidth, &uHeight);
   *pvalueResult = static_cast<int64_t>(uHeight);
   return { true, "" };
}

/// Return number of columns in terminal, 80 if unknown.
std::pair<bool, std::string> term_width_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   unsigned uWidth = 80, uHeight = 24;
   console_get_size_(&uWidth, &uHeight);
   *pvalueResult = static_cast<int64_t>(uWidth);
   return { true, "" };
}

/// Set title of the terminal window, nothing is done if output is not a terminal.
std::pair<bool, std::string> title_g(const std::vector<value>& vectorArgument)
{
   if( vectorArgument.empty() ) return { false, "title requires 1 argument" };
   if( console_is_terminal_(eConsoleStreamOutput) == false ) return { true, "" };

   std::string stringTitle = vectorArgument[0].as_string();
#if defined(_WIN32)
   SetConsoleTitleA(stringTitle.c_str());
#else
   std::cout << "\x1b]0;" << stringTitle << '\a' << std::flush;
#endif
   return { true, "" };
}

_GD_EXPRESSION_END