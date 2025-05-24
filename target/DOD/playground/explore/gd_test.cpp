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

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
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
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
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
    if (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
        if (sscanf(buf, "\033[%u;%uR", &y, &x) == 2) {
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