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

#include "input_stream.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "print_errors.h"

static struct termios old_terminal = {0};

static void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
}

static inline void configure_terminal(void) {
    // If the flag ICANON is set, the terminal was already configured
    if ((old_terminal.c_lflag & ICANON) == 0) {
        if (tcgetattr(STDIN_FILENO, &old_terminal) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal for the \"Input_Stream\", the function \"tcgetattr()\" failed with error: %s\n", strerror(errno));
        }
        struct termios new_terminal = old_terminal;
        new_terminal.c_lflag &= ~ICANON;  // Disable canonical mode
        new_terminal.c_lflag &= ~ECHO;    // Disable echo
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal for the \"Input_Stream\", the function \"tcsetattr()\" failed with error: %s\n", strerror(errno));
        }
        // Even if the application crashes, the terminal must be restored
        if (atexit(restore_terminal) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal for the \"Input_Stream\", the function \"atexit()\" failed!\n");
        }
    }
}

struct Input_Stream create_input_stream(void) {
    configure_terminal();
    return (struct Input_Stream){0};
}

bool is_empty(const char *const string) {
    const size_t len = strlen(string);
    if (len == 0) {
        return true;
    }
    for (size_t i = 0; i < len; i++) {
        if (strchr(" \t\n\r", string[i]) == NULL) {
            return false;
        }
    }
    return true;
}

// Find previous non-empty line
static inline size_t previous_line(const struct Input_Stream *const input_stream, const size_t line_index) {
    size_t previous_index = line_index;
    do {
        previous_index--;
        if (previous_index >= NUMBER_OF_LINES) {
            previous_index = NUMBER_OF_LINES - 1;
        }
        if (previous_index == input_stream->line_index) {
            // There is no previous empty line
            return line_index;
        }
    } while (is_empty(input_stream->lines[previous_index].str));
    return previous_index;
}

// Find next non-empty line
static inline size_t next_line(const struct Input_Stream *const input_stream, const size_t line_index) {
    size_t next_index = line_index;
    do {
        if (next_index == input_stream->line_index) {
            // It reached the current line
            return input_stream->line_index;
        }
        next_index++;
        if (next_index >= NUMBER_OF_LINES) {
            next_index = 0;
        }
    } while (is_empty(input_stream->lines[next_index].str));
    return next_index;
}

char *get_line_from_input(struct Input_Stream *input_stream) {
    const char *const command = "> ";
    const int command_length = strlen(command);
    size_t line_index = input_stream->line_index;
    input_stream->lines[input_stream->line_index].size = 0;
    size_t line_position = 0;
    printf(command);
    for (;;) {
        char c = getchar();
        // If is an ANSI escape code
        if (c == '\033') {
            getchar();  // Skip the [
            switch (getchar()) {
                case 'A':  // Arrow up
                {
                    const size_t previous_index = previous_line(input_stream, line_index);
                    // If found a previous non-empty line
                    if (line_index != previous_index) {
                        line_position = input_stream->lines[previous_index].size;
                        const int spaces = (input_stream->lines[line_index].size - line_position);
                        line_index = previous_index;
                        // Print the line found
                        printf("\033[%dG%s", (command_length + 1), input_stream->lines[previous_index].str);
                        // Completes the line with spaces
                        printf("%*s", (spaces > 0 ? spaces : 0), "");
                        // Restores cursor position
                        printf("\033[%dG", (int)(line_position + command_length + 1));
                    }
                } break;
                case 'B':  // Arrow down
                {
                    const size_t next_index = next_line(input_stream, line_index);
                    // If found a next non-empty line
                    if (line_index != next_index) {
                        line_position = input_stream->lines[next_index].size;
                        const int spaces = (input_stream->lines[line_index].size - line_position);
                        line_index = next_index;
                        // Print the line found
                        printf("\033[%dG%s", (command_length + 1), input_stream->lines[next_index].str);
                        // Completes the line with spaces
                        printf("%*s", (spaces > 0 ? spaces : 0), "");
                        // Restores cursor position
                        printf("\033[%dG", (int)(line_position + command_length + 1));
                    }
                } break;
                case 'C':  // Arrow right
                    if (line_position < input_stream->lines[line_index].size) {
                        printf("\033[C");
                        line_position++;
                    }
                    break;
                case 'D':  // Arrow left
                    if (line_position > 0) {
                        printf("\033[D");
                        line_position--;
                    }
                    break;
                case 'F':  // End key
                    printf("\033[%dG", (int)(input_stream->lines[line_index].size + command_length + 1));
                    line_position = input_stream->lines[line_index].size;
                    break;
                case 'H':  // Home key
                    printf("\033[%dG", (command_length + 1));
                    line_position = 0;
                    break;
            }
        } else {
            struct Line *const line = &input_stream->lines[input_stream->line_index];
            if (line_index != input_stream->line_index) {
                // The user changed the line been displayed using the arrows
                // In this case, we must copy the line shown to the current line buffer
                // This way we don't lose the command history
                memcpy(line->str, input_stream->lines[line_index].str, input_stream->lines[line_index].size);
                line->size = input_stream->lines[line_index].size;
                line_index = input_stream->line_index;
            }
            if (line->size >= LINE_SIZE) {
                print_crash_and_exit("\nYou have typed more characters than the line limit!\n");
            }
            switch (c) {
                case '\n':
                case '\r':
                    putchar('\n');
                    line->str[line->size] = '\0';
                    if (!is_empty(line->str)) {
                        input_stream->line_index++;
                        if (input_stream->line_index >= NUMBER_OF_LINES) {
                            input_stream->line_index = 0;
                        }
                    }
                    return line->str;
                    break;
                case 127:  // Backspace
                case '\b':
                    if (line_position > 0) {
                        // Update line to the output
                        printf("\033[D%.*s ", (int)(line->size - line_position), &line->str[line_position]);
                        // Move characters left
                        for (size_t i = (line_position - 1); i < line->size; i++) {
                            line->str[i] = line->str[i + 1];
                        }
                        // Restores cursor position
                        printf("\033[%dG", (int)(line_position + command_length));
                        line_position--;
                        line->size--;
                    }
                    break;
                case '~':  // Delete
                    if (line_position < line->size) {
                        // Update line to the output
                        printf("%.*s ", (int)(line->size - line_position - 1), &line->str[line_position + 1]);
                        // Move characters left
                        for (size_t i = line_position; i < line->size; i++) {
                            line->str[i] = line->str[i + 1];
                        }
                        // Restores cursor position
                        printf("\033[%dG", (int)(line_position + command_length + 1));
                        line->size--;
                    }
                    break;
                default:
                    putchar(c);  // Echo character
                    if (line_position != line->size) {
                        // Update line to the output
                        printf("%.*s", (int)(line->size - line_position), &line->str[line_position]);
                        // Moves characters to make space for the new one
                        for (size_t i = line->size; i > line_position; i--) {
                            line->str[i] = line->str[i - 1];
                        }
                        // Restores cursor position
                        printf("\033[%dG", (int)(line_position + command_length + 2));
                    }
                    line->str[line_position] = c;
                    line_position++;
                    line->size++;
            }
        }
    }
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