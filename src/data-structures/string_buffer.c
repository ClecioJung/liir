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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sized_string.h"

static const String_Node_Index string_node_header_size = offsetof(struct String_Node, data);

struct String_Node *get_node_from_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    return (struct String_Node *)&string_buffer->buf[node_index];
}

struct String_Node *get_current_node(const struct String_Buffer *const string_buffer) {
    return get_node_from_index(string_buffer, string_buffer->current_index);
}

struct String_Node *get_last_node(const struct String_Buffer *const string_buffer) {
    return get_node_from_index(string_buffer, string_buffer->last_index);
}

String_Node_Index get_next_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index previous_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    return (previous_index ^ node->xor_index);
}

String_Node_Index get_previous_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index next_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    return (next_index ^ node->xor_index);
}

static void set_node_addresses(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index previous_index, const String_Node_Index next_index) {
    struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    node->xor_index = (previous_index ^ next_index);
}

static void advance_last_string(struct String_Buffer *const string_buffer) {
    const String_Node_Index next_index = get_next_node(string_buffer, string_buffer->last_index, INVALID_STRING_INDEX);
    const String_Node_Index after_next_index = get_next_node(string_buffer, next_index, string_buffer->last_index);
    string_buffer->last_index = next_index;
    set_node_addresses(string_buffer, string_buffer->last_index, INVALID_STRING_INDEX, after_next_index);
}

// Update the index that points at the current string to a new position
static void advance_current_string(struct String_Buffer *const string_buffer, const String_Node_Index new_index) {
    const String_Node_Index previous_index = get_previous_node(string_buffer, string_buffer->current_index, INVALID_STRING_INDEX);
    set_node_addresses(string_buffer, string_buffer->current_index, previous_index, new_index);
    set_node_addresses(string_buffer, new_index, string_buffer->current_index, INVALID_STRING_INDEX);
    string_buffer->current_index = new_index;
}

// Move the current string to a new position
static void move_current_string(struct String_Buffer *const string_buffer, const String_Node_Index new_index) {
    const String_Node_Index previous_index = get_previous_node(string_buffer, string_buffer->current_index, INVALID_STRING_INDEX);
    if (previous_index >= 0) {
        const String_Node_Index before_previous_index = get_previous_node(string_buffer, previous_index, string_buffer->current_index);
        set_node_addresses(string_buffer, previous_index, before_previous_index, new_index);
    } else {
        string_buffer->last_index = new_index;
    }
    string_buffer->current_index = new_index;
    set_node_addresses(string_buffer, string_buffer->current_index, previous_index, INVALID_STRING_INDEX);
}

static void clear_string_at_index(struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    node->length = 0;
}

static void reset_string_buffer(struct String_Buffer *const string_buffer) {
    string_buffer->current_index = 0;
    string_buffer->last_index = 0;
    set_node_addresses(string_buffer, string_buffer->current_index, INVALID_STRING_INDEX, INVALID_STRING_INDEX);
    clear_string_at_index(string_buffer, string_buffer->current_index);
}

struct String_Buffer create_string_buffer(void) {
    struct String_Buffer string_buffer = (struct String_Buffer){0};
    reset_string_buffer(&string_buffer);
    return string_buffer;
}

struct String get_string_from_node(struct String_Node *const node) {
    return (struct String){
        .length = node->length,
        .data = node->data,
    };
}

// This function doesn't return an error
// If something wrong happens, this function resets the string_buffer
// This does not invalidate the last string provided, and can still be used, until it gets replaced
void update_current_string(struct String_Buffer *const string_buffer) {
    const String_Length length = get_current_node(string_buffer)->length;
    // Calculate new index for the current string
    String_Node_Index new_index = (String_Node_Index)(string_buffer->current_index + string_node_header_size + length + 1);
    // Check if we must merge to the next string
    if (string_buffer->current_index < string_buffer->last_index) {
        if ((new_index + string_node_header_size + 1) >= string_buffer->last_index) {
            // Merge current line to the next one (which must be the last string)
            advance_last_string(string_buffer);
        }
    }
    // Checks for buffer overflow
    if ((new_index + string_node_header_size + 1) >= BUFFER_SIZE) {
        if (string_buffer->current_index == 0) {
            // All the buffer was occupied and we cannot even instantiate a new string
            // So, we must do nothing. The current string can still be used
            // But it will be replaced the next time we store a string.
            reset_string_buffer(string_buffer);
            return;
        } else {
            // Move new line to the beginning of the buffer
            new_index = 0;
            // The last string must be at the beginnning of the buffer,
            // so we move the last string to the next one.
            advance_last_string(string_buffer);
        }
    }
    advance_current_string(string_buffer, new_index);
    clear_string_at_index(string_buffer, string_buffer->current_index);
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
            if (string_buffer->current_index < string_buffer->last_index) {
                if ((string_buffer->current_index + string_node_header_size + length + 1) >= string_buffer->last_index) {
                    // Merge current line to the next one (which must be the last string)
                    advance_last_string(string_buffer);
                } else {
                    break;
                }
            } else if ((string_buffer->current_index + string_node_header_size + length + 1) >= (BUFFER_SIZE - 1)) {
                // The last string must be at the beginnning of the buffer,
                // so we move the last string to the next one.
                advance_last_string(string_buffer);
                if ((string_buffer->last_index <= 0) || (string_buffer->last_index == string_buffer->current_index)) {
                    // All the buffer must be allocated for the current string
                    reset_string_buffer(string_buffer);
                    break;
                } else {
                    // Move new line to the beginning of the buffer
                    move_current_string(string_buffer, 0);
                }
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
    if (string_buffer->current_index < string_buffer->last_index) {
        if ((string_buffer->current_index + string_node_header_size + length + 1) >= string_buffer->last_index) {
            // Merge current line to the next one (which must be the last string)
            advance_last_string(string_buffer);
        }
    }
    // Checks for buffer overflow
    if ((string_buffer->current_index + string_node_header_size + length + 1) >= (BUFFER_SIZE - 1)) {
        if (string_buffer->current_index == 0) {
            // All the buffer was occupied and we cannot store nothing more.
            // In this case, we reset the buffer structure and report an error
            reset_string_buffer(string_buffer);
            return true;
        } else {
            // New index for the current string
            const String_Length new_index = 0;
            // The last string must be at the beginnning of the buffer,
            // so we move the last string to the next one.
            // But the size gained may not be enough, so we must merge strings until we get enough space
            do {
                advance_last_string(string_buffer);
                if ((string_buffer->last_index <= 0) || (string_buffer->last_index == string_buffer->current_index)) {
                    // All the buffer must be allocated for the current string
                    string_buffer->last_index = new_index;
                    set_node_addresses(string_buffer, new_index, INVALID_STRING_INDEX, INVALID_STRING_INDEX);
                    break;
                }
            } while ((new_index + string_node_header_size + length + 1) >= string_buffer->last_index);
            // We have changed the position of the current line,
            // so we must copy the content to the new position
            struct String_Node *const node = get_node_from_index(string_buffer, new_index);
            const struct String_Node *const node_to_copy = get_current_node(string_buffer);
            // During the copy of the strings, the content of node_to_copy->length may be overwritten,
            // because of this, we store a copy of its content in this constant
            const String_Length length = node_to_copy->length;
            for (String_Length i = 0; i < length; i++) {
                node->data[i] = node_to_copy->data[i];
            }
            node->length = length;
            // Move new line to the beginning of the buffer
            if (string_buffer->last_index != string_buffer->current_index) {
                move_current_string(string_buffer, new_index);
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

void print_chars_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    if (node->length > position) {
        fprintf(file, "%.*s", (int)(node->length - position), &node->data[position]);
    }
}

void print_chars_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position) {
    print_chars_at_index_to(stdout, string_buffer, node_index, position);
}

void print_string_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    const struct String_Node *const node = get_node_from_index(string_buffer, node_index);
    fprintf(file, "%.*s", node->length, node->data);
}

void print_string_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index) {
    print_string_at_index_to(stdout, string_buffer, node_index);
}

void print_string_buffer_to(FILE *const file, const struct String_Buffer *const string_buffer) {
    String_Node_Index previous_index = INVALID_STRING_INDEX;
    String_Node_Index node_index = string_buffer->last_index;
    if ((node_index < 0) || (node_index == string_buffer->current_index)) {
        return;
    }
    fprintf(file, "Address String\n");
    while ((node_index >= 0) && (node_index != string_buffer->current_index)) {
        fprintf(file, "[%04d]  ", node_index);
        print_string_at_index_to(file, string_buffer, node_index);
        printf("\n");
        String_Node_Index next_index = get_next_node(string_buffer, node_index, previous_index);
        previous_index = node_index;
        node_index = next_index;
    }
}

void print_string_buffer(const struct String_Buffer *const string_buffer) {
    print_string_buffer_to(stdout, string_buffer);
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