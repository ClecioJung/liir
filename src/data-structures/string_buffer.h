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
// HEADER
//------------------------------------------------------------------------------

#ifndef __STRING_BUFFER
#define __STRING_BUFFER

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sized_string.h"

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(a)
#endif

// Data structure for storing strings sequentially in a static size buffer
// It is a XOR double linked list, where each node is a string
// Since the buffers used in this application are relativelly small, we an use indexes of 2 bytes

typedef int16_t String_Node_Index;

#define INVALID_STRING_INDEX ((String_Node_Index)(-1))

struct String_Node {
    String_Node_Index xor_index;
    // String definition
    String_Length length;
    char data[];
};

// It should be enougth for everyone ;)
#define BUFFER_SIZE 4096

struct String_Buffer {
    String_Node_Index current_index;  // Index for the string currently being edited
    String_Node_Index last_index;     // Index to the last known string
    uint8_t buf[BUFFER_SIZE];
};

struct String_Node *get_node_from_index(const struct String_Buffer *const string_buffer, const String_Node_Index string_index)
    __attribute__((nonnull));
struct String_Node *get_current_node(const struct String_Buffer *const string_buffer)
    __attribute__((nonnull));
struct String_Node *get_last_node(const struct String_Buffer *const string_buffer)
    __attribute__((nonnull));
String_Node_Index get_next_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index previous_index)
    __attribute__((nonnull));
String_Node_Index get_previous_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index next_index)
    __attribute__((nonnull));
struct String_Buffer create_string_buffer(void);
struct String get_string_from_node(struct String_Node *const node)
    __attribute__((nonnull));
void update_current_string(struct String_Buffer *const string_buffer)
    __attribute__((nonnull));
void remove_char_at(struct String_Buffer *const string_buffer, const String_Length position)
    __attribute__((nonnull));
void copy_string_at_index(struct String_Buffer *const string_buffer, const String_Node_Index string_index)
    __attribute__((nonnull));
// This function return true if an error occurred
bool add_char_at(struct String_Buffer *const string_buffer, const char c, const String_Length position)
    __attribute__((nonnull));

void print_chars_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position)
    __attribute__((nonnull));
void print_chars_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position)
    __attribute__((nonnull));
void print_string_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index)
    __attribute__((nonnull));
void print_string_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index)
    __attribute__((nonnull));
void print_string_buffer_to(FILE *const file, const struct String_Buffer *const string_buffer)
    __attribute__((nonnull));
void print_string_buffer(const struct String_Buffer *const string_buffer)
    __attribute__((nonnull));

#endif  // __STRING_BUFFER

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
