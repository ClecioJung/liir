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

#ifndef __VARIABLES
#define __VARIABLES

#include "data-structures/sized_string.h"

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(a)
#endif

struct Variable {
    struct String name;
    double value;
};

struct Variables {
    // Dynamic array used to store the list of variables
    struct Variable *list;
};

struct Variables create_variables(const size_t initial_list_size);
void destroy_variables(struct Variables *const vars)
    __attribute__((nonnull));
void clear_variables(struct Variables *const vars)
    __attribute__((nonnull));
int search_variable(struct Variables *const vars, const struct String name, size_t *const index)
    __attribute__((nonnull));
void new_variable(struct Variables *const vars, const size_t index, const struct String name, const double value)
    __attribute__((nonnull));
int delete_variable(struct Variables *const vars, const struct String name)
    __attribute__((nonnull));
double assign_variable(struct Variables *const vars, const struct String name, const double value)
    __attribute__((nonnull));
double get_variable_value(struct Variables *const vars, const size_t index)
    __attribute__((nonnull));
void print_variables(struct Variables *const vars)
    __attribute__((nonnull));
bool variable_list_is_empty(struct Variables *const vars)
    __attribute__((nonnull));

// The functions bellow can load and save the variables to file.
// The data is saved in the file using the structure "key = value" in each line.
// The key must be a valid variable name and the value must be a number.
void load_variables_from_file(struct Variables *const vars, const struct String file_name)
    __attribute__((nonnull));
void save_variables_to_file(struct Variables *const vars, const struct String file_name)
    __attribute__((nonnull));

#endif  // __VARIABLES

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
