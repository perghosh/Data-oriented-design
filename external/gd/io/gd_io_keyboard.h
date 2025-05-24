#pragma once

#ifdef _WIN32
#include <conio.h>  // For _kbhit() and _getch() on Windows
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif

#ifndef _GD_IO_BEGIN
#define _GD_IO_BEGIN namespace gd { namespace io { 
#define _GD_IO_END } }
#endif

_GD_IO_BEGIN

#ifdef _WIN32
inline int kbhit() { return _kbhit(); }
inline int getch() { return _getch(); }
#else

/// @brief Cross-platform keyboard hit detection for Unix/Linux
int kbhit();
/// @brief Cross-platform character input without echo for Unix/Linux
int getch();
#endif

_GD_IO_END