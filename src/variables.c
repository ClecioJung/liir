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

#include "parser.h"
#include "print_errors.h"

static const size_t initial_variable_list_size = 64;

static struct Variable_List variables = {0};

void resize_variable_list(const int size) {
    struct Variable *newList = (struct Variable *)realloc(variables.list, size * sizeof(struct Variable));
    if (newList == NULL) {
        free_variables();
        print_crash_and_exit("Could't reallocate memory for the variables!\n");
    }
    variables.list = newList;
    variables.capacitity = size;
}

void init_variables(void) {
    resize_variable_list(initial_variable_list_size);
}

void free_variables(void) {
    clear_variables();
    free(variables.list);
    variables.list = NULL;
}

// Deletes all variables
void clear_variables(void) {
    for (int i = 0; i < variables.size; i++) {
        free(variables.list[i].name);
        variables.list[i].name = NULL;
    }
    variables.size = 0;
}

// This function returns zero if found the variable in the list
int search_variable(const char *const name, const unsigned int length, int *const index) {
    if (index == NULL) {
        return -1;
    }
    if (variables.size == 0) {
        *index = 0;
        return -1;
    }
    // Binary search in variables list
    *index = variables.size;  // middle
    int low = 0;
    int high = variables.size - 1;
    int comp = -1;
    while (low <= high) {
        *index = (low + high) / 2;
        comp = strncmp(name, variables.list[*index].name, length);
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

void new_variable(const int index, char *const name, const unsigned int length, const double value) {
    // Allocates more memory for the list, if necessary
    if (variables.size >= variables.capacitity) {
        resize_variable_list(2 * variables.capacitity);
    }
    // Moves variables down in the list to make space for the new variable
    for (int i = variables.size; i > index; i--) {
        variables.list[i].name = variables.list[i - 1].name;
        variables.list[i].value = variables.list[i - 1].value;
    }
    // Allocates memory for the name
    variables.list[index].name = malloc((length + 1) * sizeof(char));
    if (variables.list[index].name == NULL) {
        print_crash_and_exit("Could't allocate memory for the variable %.*s!\n", length, name);
    }
    strncpy(variables.list[index].name, name, length);
    variables.list[index].name[length] = '\0';
    variables.list[index].value = value;
    variables.size++;
}

double delete_variable(char *const name, const unsigned int length) {
    int index;
    if (search_variable(name, length, &index) != 0) {
        return NAN;
    }
    // Moves variables up in the list to cover space of the deleted variable
    const double value = variables.list[index].value;
    free(variables.list[index].name);
    variables.list[index].name = NULL;
    for (int i = index; (i + 1) < variables.size; i++) {
        variables.list[i].name = variables.list[i + 1].name;
        variables.list[i].value = variables.list[i + 1].value;
    }
    variables.size--;
    variables.list[variables.size].name = NULL;
    return value;
}

double assign_variable(char *const name, const unsigned int length, const double value) {
    int index;
    const int search = search_variable(name, length, &index);
    if (search != 0) {
        // Insert new variable in alphabetical order
        if (search > 0) {
            index++;
        }
        new_variable(index, name, length, value);
    } else {
        // Variable already exist
        variables.list[index].value = value;
    }
    return value;
}

double get_variable(const int index) {
    if ((index < 0) || (index > variables.size)) {
        return NAN;
    }
    return variables.list[index].value;
}

static inline unsigned int longest_name_variables(void) {
    unsigned int length = 0;
    for (int i = 0; i < variables.size; i++) {
        length = max_uint(length, strlen(variables.list[i].name));
    }
    return length;
}

void print_variables(void) {
    if (variables.size == 0) {
        return;
    }
    const char *const header = "Name";
    const unsigned int max_length = max_uint(longest_name_variables(), strlen(header));
    printf("List of variables:\n");
    printf("%-*s Value\n", max_length, header);
    for (int i = 0; i < variables.size; i++) {
        printf("%-*s %lg\n", max_length, variables.list[i].name, variables.list[i].value);
    }
    printf("\n");
}

bool variable_list_is_empty(void) {
    return (variables.size == 0);
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