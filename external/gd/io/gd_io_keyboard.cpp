#include "gd_io_keyboard.h"

_GD_IO_BEGIN

#ifdef _WIN32
#else
/*----------------------------------------------------------------------------- kbhit
* @brief Cross-platform keyboard hit detection for Unix/Linux
* 
* Detects if a key has been pressed without blocking execution
* 
* @return int 1 if key was pressed, 0 otherwise
*/
int kbhit()
{
   struct termios oldt, newt;
   int iCharacter;
   int iOldFlags;

   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   iOldFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
   fcntl(STDIN_FILENO, F_SETFL, iOldFlags | O_NONBLOCK);

   iCharacter = getchar();

   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
   fcntl(STDIN_FILENO, F_SETFL, iOldFlags);

   if( iCharacter != EOF )
   {
      ungetc(iCharacter, stdin);
      return 1;
   }

   return 0;
}

/*----------------------------------------------------------------------------- getch
* @brief Cross-platform character input without echo for Unix/Linux
* 
* Reads a single character from stdin without echoing to console
* 
* @return int character code that was pressed
*/
int getch()
{
   struct termios oldt, newt;
   int iCharacter;
   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   iCharacter = getchar();
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
   return iCharacter;
}
#endif


_GD_IO_END