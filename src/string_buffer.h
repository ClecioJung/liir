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
#include <stdlib.h>

// Sized struct for storing and handling strings
// Unlike C-strings, this structure is not null terminated
// Since the strings used in this application are very small,
// we will never need more than 2 bytes to store its length

typedef uint16_t String_Length;

struct String {
    String_Length length;
    char *data;
};

void print_string_to(FILE *const file, const struct String *const str);
void print_string(const struct String *const str);
struct String create_string(char *const cstr);
bool string_is_empty(const struct String *const str);

// Data structure for storing strings sequentially in a static size buffer
// It is a double linked list, where each node is a string
// Since the buffers used in this application are relativelly small, we an use indexes of 2 bytes

typedef int16_t String_Node_Index;

struct String_Node {
    String_Node_Index previous_idx;
    String_Node_Index next_idx;
    // String definition
    String_Length length;
    char data[];
};

// It should be enougth for everyone ;)
#define BUFFER_SIZE 4096

struct String_Buffer {
    String_Node_Index current_idx;  // Index for the string currently being edited
    String_Node_Index last_idx;     // Index to the last known string
    uint8_t buf[BUFFER_SIZE];
};

struct String_Node *get_node_from_index(const struct String_Buffer *const string_buffer, const String_Node_Index string_index);
struct String_Node *get_current_node(const struct String_Buffer *const string_buffer);
struct String_Node *get_last_node(const struct String_Buffer *const string_buffer);
String_Node_Index next_node_index(const struct String_Buffer *const string_buffer, const String_Node_Index string_index);
String_Node_Index previous_node_index(const struct String_Buffer *const string_buffer, const String_Node_Index string_index);
struct String string_from_node(struct String_Node *const node);
void set_next_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index next_node);
void set_previous_node(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Node_Index previous_node);
void reset_string_node(struct String_Buffer *const string_buffer, const String_Node_Index node_index);

void reset_string_buffer(struct String_Buffer *const string_buffer);
struct String_Buffer create_string_buffer(void);
void print_chars_at_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position);
void print_chars_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index, const String_Length position);
void print_string_at_index_to(FILE *const file, const struct String_Buffer *const string_buffer, const String_Node_Index node_index);
void print_string_at_index(const struct String_Buffer *const string_buffer, const String_Node_Index node_index);
void print_string_buffer_to(FILE *const file, const struct String_Buffer *const string_buffer);
void print_string_buffer(const struct String_Buffer *const string_buffer);

// This functions allow us to edit the current string in the string_buffer
void update_current_string(struct String_Buffer *const string_buffer);
void remove_char_at(struct String_Buffer *const string_buffer, const String_Length position);
void copy_string_at_index(struct String_Buffer *const string_buffer, const String_Node_Index string_index);
// This function return true if an error occurred
bool add_char_at(struct String_Buffer *const string_buffer, const char c, const String_Length position);

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