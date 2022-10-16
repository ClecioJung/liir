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

#include "string_buffer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_string_to(FILE *const file, const struct String *const str) {
    fprintf(file, "%.*s", str->length, str->data);
}

void print_string(const struct String *const str) {
    print_string_to(stdout, str);
}

struct String create_string(char *const cstr) {
    return (struct String){
        .data = cstr,
        .length = strlen(cstr),
    };
}

bool string_is_empty(const struct String *const str) {
    for (String_Length i = 0; i < str->length; i++) {
        if (!isspace(str->data[i])) {
            return false;
        }
    }
    return true;
}

struct String_Node *get_node_from_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    return (struct String_Node *)&string_buffer->buf[node_index];
}

struct String_Node *get_current_node(const struct String_Buffer *const string_buffer) {
    return get_node_from_index(string_buffer, string_buffer->current_idx);
}

struct String_Node *get_last_node(const struct String_Buffer *const string_buffer) {
    return get_node_from_index(string_buffer, string_buffer->last_idx);
}

String_Node_Index next_node_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    return node->next_idx;
}

String_Node_Index previous_node_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    return node->previous_idx;
}

void set_next_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index next_node) {
    struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    node->next_idx = next_node;
}

void set_previous_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index previous_node) {
    struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    node->previous_idx = previous_node;
}

struct String string_from_node(struct String_Node *const node) {
    return (struct String){
        .length = node->length,
        .data = node->data,
    };
}

void reset_string_node(struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    node->next_idx = -1;
    node->previous_idx = -1;
    node->length = 0;
}

void reset_string_buffer(struct String_Buffer *const string_buffer) {
    string_buffer->current_idx = 0;
    string_buffer->last_idx = 0;
    reset_string_node(string_buffer, string_buffer->current_idx);
}

struct String_Buffer create_string_buffer(void) {
    struct String_Buffer string_buffer = (struct String_Buffer){0};
    reset_string_buffer(&string_buffer);
    return string_buffer;
}

void print_chars_at_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    if (node->length > position) {
        fprintf(file, "%.*s", (int)(node->length - position), &node->data[position]);
    }
}

void print_chars_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position) {
    print_chars_at_to(stdout, string_buffer, node_index, position);
}

void print_string_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    fprintf(file, "%.*s", node->length, node->data);
}

void print_string_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    print_string_at_index_to(stdout, string_buffer, node_index);
}

void print_string_buffer_to(FILE *const file, const struct String_Buffer *const string_buffer) {
    String_Node_Index node_index = string_buffer->last_idx;
    if ((node_index < 0) || (node_index == string_buffer->current_idx)) {
        return;
    }
    fprintf(file, "Address String\n");
    while ((node_index >= 0) && (node_index != string_buffer->current_idx)) {
        fprintf(file, "[%04d]  ", node_index);
        print_string_at_index_to(file, string_buffer, node_index);
        printf("\n");
        node_index = next_node_index(string_buffer, node_index);
    }
}

void print_string_buffer(const struct String_Buffer *const string_buffer) {
    print_string_buffer_to(stdout, string_buffer);
}

static const String_Node_Index string_node_header_size = offsetof(struct String_Node, data);

// This functions does not return an error
// If something wrong happens, this function resets the string_buffer
// This does not invalidate the last string provided, and can still be used, until it gets replaced
void update_current_string(struct String_Buffer *const string_buffer) {
    const String_Node_Index previous_index = string_buffer->current_idx;
    const String_Length length = get_current_node(string_buffer)->length;
    // Update current string
    string_buffer->current_idx += string_node_header_size + length + 1;
    // Check if we must merge to the next string
    if (previous_index < string_buffer->last_idx) {
        if ((string_buffer->current_idx + string_node_header_size + 1) >= string_buffer->last_idx) {
            // Merge current line to the next one (which must be the last string)
            string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
            set_previous_node(string_buffer, string_buffer->last_idx, -1);
        }
    }
    // Checks for buffer overflow
    if ((string_buffer->current_idx + string_node_header_size + 1) >= BUFFER_SIZE) {
        if (previous_index == 0) {
            // All the buffer was occupied and we cannot even instantiate a new string
            // So, we must do nothing. The current string can still be used
            // But it will be replaced the next time we store a string.
            reset_string_buffer(string_buffer);
            return;
        } else {
            // Move new line to the beginning of the buffer
            string_buffer->current_idx = 0;
            // The last string must be at the beginnning of the buffer,
            // so we move the last string to the next one.
            string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
            set_previous_node(string_buffer, string_buffer->last_idx, -1);
        }
    }
    set_next_node(string_buffer, previous_index, string_buffer->current_idx);
    struct String_Node *const current_node = get_current_node(string_buffer);
    current_node->next_idx = -1;
    current_node->previous_idx = previous_index;
    current_node->length = 0;
}

void remove_char_at(struct String_Buffer *const string_buffer, const String_Length position) {
    struct String_Node *const current_node = get_current_node(string_buffer);
    if (current_node->length > 0) {
        // Move characters left
        for (String_Length i = position; i < current_node->length; i++) {
            current_node->data[i] = current_node->data[i + 1];
        }
        current_node->length--;
    }
}

void copy_string_at_index(struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    const struct String_Node *const node_to_copy = get_node_from_index(string_buffer, node_index);
    const String_Length current_length = get_current_node(string_buffer)->length;
    const String_Length length = node_to_copy->length;
    // Check if we will need more space in the buffer to allocate the copied string
    if (length > current_length) {
        // Try to merge strings in loop, until gets the necessary buffer size
        while (true) {
            if (string_buffer->current_idx < string_buffer->last_idx) {
                if ((string_buffer->current_idx + string_node_header_size + length + 1) >= string_buffer->last_idx) {
                    // Merge current line to the next one (which must be the last string)
                    string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
                    set_previous_node(string_buffer, string_buffer->last_idx, -1);
                } else {
                    break;
                }
            } else if ((string_buffer->current_idx + string_node_header_size + length + 1) >= (BUFFER_SIZE - 1)) {
                const String_Node_Index old_index = string_buffer->current_idx;
                // Move new line to the beginning of the buffer
                string_buffer->current_idx = 0;
                // The last string must be at the beginnning of the buffer,
                // so we move the last string to the next one.
                string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
                if ((string_buffer->last_idx <= 0) || (string_buffer->last_idx == old_index)) {
                    // All the buffer must be allocated for the current string
                    reset_string_buffer(string_buffer);
                    break;
                }
                set_previous_node(string_buffer, string_buffer->last_idx, -1);
                // Restores the links between the nodes of the linked list
                const struct String_Node *const old_position = get_node_from_index(string_buffer, old_index);
                if (old_position->previous_idx >= 0) {
                    set_next_node(string_buffer, old_position->previous_idx, string_buffer->current_idx);
                }
                set_previous_node(string_buffer, string_buffer->current_idx, old_position->previous_idx);
                set_next_node(string_buffer, string_buffer->current_idx, -1);
            } else {
                break;
            }
        }
    }
    // Copy the contents to the new location
    // During the copy of the strings, the content of node_to_copy->length may be overwritten,
    // because of this, we store a copy of its content into the constant length
    struct String_Node *const current_node = get_current_node(string_buffer);
    for (String_Length i = 0; i < length; i++) {
        current_node->data[i] = node_to_copy->data[i];
    }
    current_node->length = length;
}

bool add_char_at(struct String_Buffer *const string_buffer, const char c, const String_Length position) {
    const String_Length length = get_current_node(string_buffer)->length;
    // Check if we must merge to the next string
    if (string_buffer->current_idx < string_buffer->last_idx) {
        if ((string_buffer->current_idx + string_node_header_size + length + 1) >= string_buffer->last_idx) {
            // Merge current line to the next one (which must be the last string)
            string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
            set_previous_node(string_buffer, string_buffer->last_idx, -1);
        }
    }
    // Checks for buffer overflow
    if ((string_buffer->current_idx + string_node_header_size + length + 1) >= (BUFFER_SIZE - 1)) {
        if (string_buffer->current_idx == 0) {
            // All the buffer was occupied and we cannot store nothing more.
            // In this case, we reset the buffer structure and report an error
            reset_string_buffer(string_buffer);
            return true;
        } else {
            const String_Node_Index old_index = string_buffer->current_idx;
            // Move new line to the beginning of the buffer
            string_buffer->current_idx = 0;
            // The last string must be at the beginnning of the buffer,
            // so we move the last string to the next one.
            // But the size gained may not be enough, so we must merge strings until we get enough space
            do {
                string_buffer->last_idx = next_node_index(string_buffer, string_buffer->last_idx);
                if ((string_buffer->last_idx <= 0) || (string_buffer->last_idx == old_index)) {
                    // All the buffer must be allocated for the current string
                    reset_string_buffer(string_buffer);
                    break;
                }
                set_previous_node(string_buffer, string_buffer->last_idx, -1);
            } while ((string_buffer->current_idx + string_node_header_size + length + 1) >= string_buffer->last_idx);
            // We have changed the position of the current line,
            // so we must copy the content to the new position
            struct String_Node *const current_node = get_current_node(string_buffer);
            const struct String_Node *const old_position = get_node_from_index(string_buffer, old_index);
            // During the copy of the strings, the content of old_position->length may be overwritten,
            // because of this, we store a copy of its content in this constant
            const String_Length length = old_position->length;
            for (String_Length i = 0; i < length; i++) {
                current_node->data[i] = old_position->data[i];
            }
            current_node->length = length;
            // Restores the links between the nodes of the linked list
            if (string_buffer->last_idx != string_buffer->current_idx) {
                if (old_position->previous_idx >= 0) {
                    set_next_node(string_buffer, old_position->previous_idx, string_buffer->current_idx);
                }
                set_previous_node(string_buffer, string_buffer->current_idx, old_position->previous_idx);
                set_next_node(string_buffer, string_buffer->current_idx, -1);
            }
        }
    }
    // Move characters right to make space for the new one
    struct String_Node *const current_node = get_current_node(string_buffer);
    for (String_Length i = current_node->length; i > position; i--) {
        current_node->data[i] = current_node->data[i - 1];
    }
    current_node->data[position] = c;
    current_node->length++;
    return false;
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