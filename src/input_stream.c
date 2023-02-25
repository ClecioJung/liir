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

#include "data-structures/sized_string.h"
#include "data-structures/string_buffer.h"
#include "printing.h"

struct Input_Stream {
    struct termios old_termios;
    bool terminal_is_configured;
    struct String_Buffer lines;
};

static struct Input_Stream input_stream = {0};

static void restore_terminal(void) {
    if (input_stream.terminal_is_configured) {
        tcsetattr(STDIN_FILENO, TCSANOW, &input_stream.old_termios);
    }
}

static void configure_terminal(void) {
    if (!input_stream.terminal_is_configured) {
        if (!isatty(STDIN_FILENO)) {
            print_crash_and_exit("The standard input must be a terminal!\n");
        }
        if (tcgetattr(STDIN_FILENO, &input_stream.old_termios) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal, the function \"tcgetattr()\" failed with error: %s\n", strerror(errno));
        }
        input_stream.terminal_is_configured = true;
        struct termios new_terminal = input_stream.old_termios;
        new_terminal.c_lflag &= (tcflag_t) ~ICANON;  // Disable canonical mode
        new_terminal.c_lflag &= (tcflag_t) ~ECHO;    // Disable echo
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal, the function \"tcsetattr()\" failed with error: %s\n", strerror(errno));
        }
        // Even if the application crashes, the terminal must be restored
        if (atexit(restore_terminal) != EXIT_SUCCESS) {
            print_crash_and_exit("When configuring the terminal, the function \"atexit()\" failed!\n");
        }
    }
}

void initialize_input_stream(void) {
    configure_terminal();
    input_stream.lines = create_string_buffer();
}

static inline String_Length jump_words_right(const String_Node_Index line_index, const String_Length position) {
    const struct String_Node *const string = get_node_from_index(&input_stream.lines, line_index);
    const String_Length length = string->length;
    if (position == length) {
        return length;
    }
    String_Length i = position + 1;
    for (; i < length; i++) {
        if (isspace(string->data[position])) {
            if (!isspace(string->data[i])) {
                break;
            }
        } else if (!isalnum(string->data[i])) {
            break;
        }
    }
    return i;
}

static inline String_Length jump_words_left(const String_Node_Index line_index, const String_Length position) {
    if (position == 0) {
        return 0;
    }
    const struct String_Node *const string = get_node_from_index(&input_stream.lines, line_index);
    String_Length i = position - 1;
    for (; i > 0; i--) {
        if (isspace(string->data[position])) {
            if (!isspace(string->data[i])) {
                break;
            }
        } else if (!isalnum(string->data[i])) {
            break;
        }
    }
    return i;
}

struct String get_line_from_input(void) {
    const struct String command = create_string("> ");
    print_string(command);
    // Local variable used to store the current position of the cursor
    String_Length position = 0;
    // Variables used to iterate over the linked list of strings in the buffer
    // Since the linked list is of the XOR type, we need to know the direction of the iteration,
    // and also an extra index
    bool previous_direction = true;
    String_Node_Index auxiliar_index = INVALID_STRING_INDEX;
    String_Node_Index line_index = input_stream.lines.current_index;
    for (;;) {
        const char c = (char)getchar();
        // If is an ANSI escape code
        if (c == '\033') {
            if (getchar() == '[') {  // Skip the [
                const String_Length length = get_node_from_index(&input_stream.lines, line_index)->length;
                switch (getchar()) {
                case 'A':  // Arrow up
                {
                    String_Node_Index previous_index;
                    if (previous_direction) {
                        previous_index = get_previous_node(&input_stream.lines, line_index, auxiliar_index);
                    } else {
                        previous_index = auxiliar_index;
                        previous_direction = true;
                    }
                    // If there is a previous non-empty line
                    if (previous_index >= 0) {
                        auxiliar_index = line_index;
                        line_index = previous_index;
                        position = get_node_from_index(&input_stream.lines, line_index)->length;
                        const int spaces = length - position;
                        // Print the line found
                        printf("\033[%dG", (command.length + 1));
                        print_string_at_index(&input_stream.lines, line_index);
                        // Completes the line with spaces
                        printf("%*s", (spaces > 0 ? spaces : 0), "");
                        // Restores cursor position
                        printf("\033[%dG", (int)(position + command.length + 1));
                    }
                } break;
                case 'B':  // Arrow down
                {
                    String_Node_Index next_index;
                    if (!previous_direction) {
                        next_index = get_next_node(&input_stream.lines, line_index, auxiliar_index);
                    } else {
                        next_index = auxiliar_index;
                        previous_direction = false;
                    }
                    // If there is a next non-empty line
                    // const String_Node_Index next_index = get_next_node(&input_stream.lines, line_index, auxiliar_index);
                    if (next_index >= 0) {
                        auxiliar_index = line_index;
                        line_index = next_index;
                        position = get_node_from_index(&input_stream.lines, line_index)->length;
                        const int spaces = length - position;
                        // Print the line found
                        printf("\033[%dG", (command.length + 1));
                        print_string_at_index(&input_stream.lines, line_index);
                        // Completes the line with spaces
                        printf("%*s", (spaces > 0 ? spaces : 0), "");
                        // Restores cursor position
                        printf("\033[%dG", (int)(position + command.length + 1));
                    }
                } break;
                case 'C':  // Arrow right
                    if (position < length) {
                        printf("\033[C");
                        position++;
                    }
                    break;
                case 'D':  // Arrow left
                    if (position > 0) {
                        printf("\033[D");
                        position--;
                    }
                    break;
                case 'F':  // End key
                    printf("\033[%dG", (int)(length + command.length + 1));
                    position = length;
                    break;
                case 'H':  // Home key
                    printf("\033[%dG", (command.length + 1));
                    position = 0;
                    break;
                case '1': // Ctrl + key
                    if ((getchar() == ';') && (getchar() == '5')) {
                        switch (getchar()) {
                        case 'C':  // Arrow right
                            position = jump_words_right(line_index, position);
                            // Update the cursor position
                            printf("\033[%dG", (int)(position + command.length + 1));
                            break;
                        case 'D':  // Arrow left
                            position = jump_words_left(line_index, position);
                            // Update the cursor position
                            printf("\033[%dG", (int)(position + command.length + 1));
                            break;
                        }
                    }
                    break;
                }
            }
        } else {
            if (line_index != input_stream.lines.current_index) {
                // The user changed the line been displayed using the arrows
                // In this case, we must copy the line shown to the current line buffer
                // This way we don't lose the command history
                copy_string_at_index(&input_stream.lines, line_index);
                // The current line address may have changed when merging lines
                line_index = input_stream.lines.current_index;
                // Reset auxiliar variables used to iterate over the linked list of strings
                previous_direction = true;
                auxiliar_index = INVALID_STRING_INDEX;
            }
            struct String_Node *const node = get_current_node(&input_stream.lines);
            const String_Length length = node->length;
            switch (c) {
            case '\n':
            case '\r':
                putchar('\n');
                struct String str = get_string_from_node(node);
                if (!string_is_empty(&str)) {
                    update_current_string(&input_stream.lines);
                }
                return str;
                break;
            case 127:  // Backspace
            case '\b':
                if (position > 0) {
                    position--;
                    remove_char_at(&input_stream.lines, position);
                    // Go left one position with the cursor
                    printf("\033[D");
                    // Update line to the output
                    print_chars_at_index(&input_stream.lines, line_index, position);
                    // Override the last printed char with a space
                    putchar(' ');
                    // Restores cursor position
                    printf("\033[%dG", (int)(position + command.length + 1));
                }
                break;
            case '~':  // Delete
                if (position < length) {
                    remove_char_at(&input_stream.lines, position);
                    // Update line to the output
                    print_chars_at_index(&input_stream.lines, line_index, position);
                    // Override the last printed char with a space
                    putchar(' ');
                    // Restores cursor position
                    printf("\033[%dG", (int)(position + command.length + 1));
                }
                break;
            case '\t':  // Horizontal Tab
            {
                // We convert tab to four spaces, because it is easiear to handle
                unsigned int tab_to_spaces = 4;
                printf("%*s", tab_to_spaces, "");
                if (position < length) {
                    // Update line to the output
                    print_chars_at_index(&input_stream.lines, line_index, position);
                    // Restores cursor position
                    printf("\033[%dG", (int)(position + command.length + tab_to_spaces + 1));
                }
                while (tab_to_spaces--) {
                    if (add_char_at(&input_stream.lines, ' ', position)) {
                        putchar('\n');
                        print_error("You typed a expression that consumed all the input buffer! You may try again...\n\n");
                        print_string(command);
                        position = 0;
                    } else {
                        position++;
                    }
                }
                // The current line address may have changed when merging lines
                line_index = input_stream.lines.current_index;
            } break;
            default:
                putchar(c);  // Echo character
                if (position < length) {
                    // Update line to the output
                    print_chars_at_index(&input_stream.lines, line_index, position);
                    // Restores cursor position
                    printf("\033[%dG", (int)(position + command.length + 2));
                }
                if (add_char_at(&input_stream.lines, c, position)) {
                    putchar('\n');
                    print_error("You typed a expression that consumed all the input buffer! You may try again...\n\n");
                    print_string(command);
                    line_index = input_stream.lines.current_index;
                    position = 0;
                } else {
                    // The current line address may have changed when merging lines
                    line_index = input_stream.lines.current_index;
                    position++;
                }
            }
        }
    }
}

void print_previous_lines(void) {
    printf("Previous typed lines:\n");
    print_string_buffer(&input_stream.lines);
    printf("\n");
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
