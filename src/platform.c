// MIT License

// Copyright (c) 2022 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#include "platform.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <conio.h>
#else // POSIX
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#endif

#ifndef _WIN32 // POSIX
struct termios old_termios;
bool terminal_is_configured = false;

static void restore_terminal(void) {
    if (terminal_is_configured) {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &old_termios) != EXIT_SUCCESS) {
            fprintf(stderr, "Function \"tcsetattr()\" failed with error: %s\n", strerror(errno));
        }
        terminal_is_configured = false;
    }
}

// This fucntion configures the terminal in the non-canonical mode
// The terminal configuration is automatically restored to the
// default after the program ends its execution (nothing to worry there)
static bool configure_terminal_non_canonical(void) {
    if (!terminal_is_configured) {
        if (!isatty(STDIN_FILENO)) {
            fprintf(stderr, "The standard input must be a terminal!\n");
            return false;
        }
        if (tcgetattr(STDIN_FILENO, &old_termios) != EXIT_SUCCESS) {
            fprintf(stderr, "Function \"tcgetattr()\" failed with error: %s\n", strerror(errno));
            return false;
        }
        struct termios new_terminal = old_termios;
        new_terminal.c_lflag &= (tcflag_t) ~ICANON;  // Disable canonical mode
        new_terminal.c_lflag &= (tcflag_t) ~ECHO;    // Disable echo
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal) != EXIT_SUCCESS) {
            fprintf(stderr, "Function \"tcsetattr()\" failed with error: %s\n", strerror(errno));
            return false;
        }
        // If arrived here, it means that the terminal was successfully configured
        terminal_is_configured = true;
        // Even if the application crashes, the terminal must be restored
        if (atexit(restore_terminal) != EXIT_SUCCESS) {
            restore_terminal();
            fprintf(stderr, "Function \"atexit()\" failed!\n");
            return false;
        }
    }
    return true;
}
#endif

enum Keys read_key_without_echo(char *c) {
#ifndef _WIN32 // POSIX
    // Make sure that the terminal is in the non-canonical mode
    configure_terminal_non_canonical();
#endif
    while (true) { // Keep looping until we get a recognizable key
#ifdef _WIN32
        const int ch = getch();
        if (ch == 224) {
            switch (getch()) {
                case 'H': return KEY_ARROW_UP;
                case 'P': return KEY_ARROW_DOWN;
                case 'M': return KEY_ARROW_RIGHT;
                case 'K': return KEY_ARROW_LEFT;
                case 'O': return KEY_END;
                case 'G': return KEY_HOME;
                case 't': return KEY_CTRL_RIGHT;
                case 's': return KEY_CTRL_LEFT;
                case 'S': return KEY_DEL;
            }
        }
#else // POSIX
        const int ch = getchar();
        if (ch == '\033') { // If is an ANSI escape code
            if (getchar() == '[') {  // Skip the [
                switch (getchar()) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                    case 'F': return KEY_END;
                    case 'H': return KEY_HOME;
                    case '1': // Ctrl + key
                        if ((getchar() == ';') && (getchar() == '5')) {
                            switch (getchar()) {
                            case 'C': return KEY_CTRL_RIGHT;
                            case 'D': return KEY_CTRL_LEFT;
                            }
                        }
                    break;
                }
            }
        }
#endif
        else {
            switch (ch) {
            case '\r':
            case '\n':
                return KEY_ENTER;
            case 127:
            case '\b':
                return KEY_BACKSPACE;
            case '\t': return KEY_TAB;
            case '~':  return KEY_DEL;
            default:
                if (ch >= 32) {
                    *c = (char)ch;
                    return KEY_CHAR;
                }
            }
        }
    }
}

bool foreground_color(FILE *file, enum Foreground_Color color) {
#ifdef _WIN32
    const int file_fd = _fileno(file);
    if (file_fd == -1) {
        fprintf(stderr, "Failed to get file descriptor for the provided file.\n");
        return false;
    }
    const HANDLE hFile = (HANDLE)_get_osfhandle(file_fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to convert file descriptor to a handle.\n");
        return false;
    }
    static const WORD wAttributes[] = {
        [DEFAULT_FOREGROUND] = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        [BLACK_FOREGROUND] = 0,
        [RED_FOREGROUND] = FOREGROUND_INTENSITY | FOREGROUND_RED,
        [GREEN_FOREGROUND] = FOREGROUND_GREEN,
        [YELLOW_FOREGROUND] = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
        [BLUE_FOREGROUND] = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
        [MAGENTA_FOREGROUND] = FOREGROUND_RED | FOREGROUND_BLUE,
        [CYAN_FOREGROUND] = FOREGROUND_GREEN | FOREGROUND_BLUE ,
        [WHITE_FOREGROUND] = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    };
    if (!SetConsoleTextAttribute(hFile, wAttributes[color])) {
        fprintf(stderr, "Failed to set the text color.\n");
        return false;
    }
#else // POSIX
    static const int values[] = {
        [DEFAULT_FOREGROUND] = 0,
        [BLACK_FOREGROUND] = 30,
        [RED_FOREGROUND] = 31,
        [GREEN_FOREGROUND] = 32,
        [YELLOW_FOREGROUND] = 33,
        [BLUE_FOREGROUND] = 34,
        [MAGENTA_FOREGROUND] = 35,
        [CYAN_FOREGROUND] = 36,
        [WHITE_FOREGROUND] = 37,
    };
    fprintf(file, "\033[%dm", values[color]);
#endif
    return true;
}

bool move_cursor_right(FILE *file, const int n) {
#ifdef _WIN32
    const int file_fd = _fileno(file);
    if (file_fd == -1) {
        fprintf(stderr, "Failed to get file descriptor for the provided file.\n");
        return false;
    }
    const HANDLE hFile = (HANDLE)_get_osfhandle(file_fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to convert file descriptor to a handle.\n");
        return false;
    }
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD currentPos;
    // Get the current cursor position
    if (!GetConsoleScreenBufferInfo(hFile, &csbi)) {
        fprintf(stderr, "Failed to get console screen buffer info.\n");
        return false;
    }
    currentPos = csbi.dwCursorPosition;
    // Move the cursor right
    currentPos.X += (SHORT)n;
    // Set the new cursor position
    if (!SetConsoleCursorPosition(hFile, currentPos)) {
        fprintf(stderr, "Failed to set the cursor position.\n");
        return false;
    }
#else // POSIX
    fprintf(file, "\033[%dC", n);
#endif
    return true;
}

bool move_cursor_left(FILE *file, const int n) {
#ifdef _WIN32
    const int file_fd = _fileno(file);
    if (file_fd == -1) {
        fprintf(stderr, "Failed to get file descriptor for the provided file.\n");
        return false;
    }
    const HANDLE hFile = (HANDLE)_get_osfhandle(file_fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to convert file descriptor to a handle.\n");
        return false;
    }
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD currentPos;
    // Get the current cursor position
    if (!GetConsoleScreenBufferInfo(hFile, &csbi)) {
        fprintf(stderr, "Failed to get console screen buffer info.\n");
        return false;
    }
    currentPos = csbi.dwCursorPosition;
    // Move the cursor left
    currentPos.X -= (SHORT)n;
    // Set the new cursor position
    if (!SetConsoleCursorPosition(hFile, currentPos)) {
        fprintf(stderr, "Failed to set the cursor position.\n");
        return false;
    }
#else // POSIX
    fprintf(file, "\033[%dD", n);
#endif
    return true;
}

bool move_cursor_to_column(FILE *file, const int n) {
#ifdef _WIN32
    const int file_fd = _fileno(file);
    if (file_fd == -1) {
        fprintf(stderr, "Failed to get file descriptor for the provided file.\n");
        return false;
    }
    const HANDLE hFile = (HANDLE)_get_osfhandle(file_fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to convert file descriptor to a handle.\n");
        return false;
    }
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD currentPos;
    // Get the current cursor position
    if (!GetConsoleScreenBufferInfo(hFile, &csbi)) {
        fprintf(stderr, "Failed to get console screen buffer info.\n");
        return false;
    }
    currentPos = csbi.dwCursorPosition;
    // Define the new X position
    currentPos.X = (SHORT)n;
    // Set the new cursor position
    if (!SetConsoleCursorPosition(hFile, currentPos)) {
        fprintf(stderr, "Failed to set the cursor position.\n");
        return false;
    }
#else // POSIX
    fprintf(file, "\033[%dG", n);
#endif
    return true;
}

//------------------------------------------------------------------------------
// END
//------------------------------------------------------------------------------

// MIT License

// Copyright (c) 2022 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
