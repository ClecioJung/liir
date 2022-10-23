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

#include "variables.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data-structures/sized_string.h"
#include "print_errors.h"

struct Variables create_variables(const size_t initial_list_size, const size_t initial_name_size) {
    struct Variables vars = (struct Variables){
        .list = allocator_construct(sizeof(struct Variable), initial_list_size),
        .names = allocator_construct(sizeof(char), initial_name_size),
    };
    if ((vars.list.data == NULL) || (vars.names.data == NULL)) {
        print_crash_and_exit("Couldn't allocate memory for the variables!\n");
    }
    return vars;
}

void destroy_variables(struct Variables *const vars) {
    if (vars != NULL) {
        allocator_delete(&vars->list);
        allocator_delete(&vars->names);
    }
}

void clear_variables(struct Variables *const vars) {
    if (vars == NULL) {
        print_crash_and_exit("Invalid call to function \"clear_variables()\"!\n");
    }
    allocator_free_all(&vars->list);
    allocator_free_all(&vars->names);
}

static inline struct Variable *variable(struct Variables *const vars, const int index) {
    return allocator_get(vars->list, index);
}

static inline char *variable_name(struct Variables *const vars, const int index) {
    struct Variable *var = variable(vars, index);
    return allocator_get(vars->names, var->name_idx);
}

static inline double variable_value(struct Variables *const vars, const int index) {
    struct Variable *var = variable(vars, index);
    return var->value;
}

// This function returns zero if found the variable in the list
int search_variable(struct Variables *const vars, const struct String name, int *const index) {
    if ((vars == NULL) || (index == NULL)) {
        print_crash_and_exit("Invalid call to function \"search_variable()\"!\n");
        return -1;
    }
    if (vars->list.size == 0) {
        *index = 0;
        return -1;
    }
    // Binary search in variables list
    *index = vars->list.size;  // middle
    int low = 0;
    int high = vars->list.size - 1;
    int comp = -1;
    while (low <= high) {
        *index = (low + high) / 2;
        comp = strncmp(name.data, variable_name(vars, *index), name.length);
        if (comp < 0) {
            high = *index - 1;
        } else if (comp > 0) {
            low = *index + 1;
        } else {
            break;
        }
    }
    return comp;
}

void new_variable(struct Variables *const vars, const int index, const struct String name, const double value) {
    if (vars == NULL) {
        print_crash_and_exit("Invalid call to function \"new_variable()\"!\n");
        return;
    }
    // Moves variables down in the list to make space for the new variable
    for (int i = allocator_new(&vars->list); i > index; i--) {
        struct Variable *current = variable(vars, i);
        struct Variable *previous = variable(vars, i - 1);
        current->name_idx = previous->name_idx;
        current->value = previous->value;
    }
    struct Variable *const new_var = variable(vars, index);
    new_var->name_idx = allocator_new_array(&vars->names, name.length + 1);
    new_var->value = value;
    char *const var_name = variable_name(vars, index);
    strncpy(var_name, name.data, name.length);
    var_name[name.length] = '\0';
}

double assign_variable(struct Variables *const vars, const struct String name, const double value) {
    if (vars == NULL) {
        print_crash_and_exit("Invalid call to function \"assign_variable()\"!\n");
        return NAN;
    }
    int index;
    const int search = search_variable(vars, name, &index);
    if (search != 0) {
        // Insert new variable in alphabetical order
        if (search > 0) {
            index++;
        }
        new_variable(vars, index, name, value);
    } else {
        // Variable already exist
        variable(vars, index)->value = value;
    }
    return value;
}

double get_variable_value(struct Variables *const vars, const int index) {
    if (vars == NULL) {
        print_crash_and_exit("Invalid call to function \"get_variable_value()\"!\n");
    }
    if (allocator_is_invalid(vars->list, index)) {
        return NAN;
    }
    return variable(vars, index)->value;
}

// Defined on main.c
extern unsigned int max_uint(const unsigned int a, const unsigned int b);

static inline unsigned int longest_variable_name(struct Variables *const vars) {
    unsigned int length = 0;
    for (size_t i = 0; i < vars->list.size; i++) {
        length = max_uint(length, strlen(variable_name(vars, i)));
    }
    return length;
}

void print_variables(struct Variables *const vars) {
    if (vars == NULL) {
        print_crash_and_exit("Invalid call to function \"print_variables()\"!\n");
    }
    if (vars->list.size == 0) {
        return;
    }
    const char *const header = "Name";
    const unsigned int max_length = max_uint(longest_variable_name(vars), strlen(header));
    printf("List of variables:\n");
    printf("%-*s Value\n", max_length, header);
    for (size_t i = 0; i < vars->list.size; i++) {
        printf("%-*s %lg\n", max_length, variable_name(vars, i), variable_value(vars, i));
    }
    printf("\n");
}

bool variable_list_is_empty(struct Variables *const vars) {
    return (vars->list.size == 0);
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