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

#define INITIAL_VARIABLE_LIST_SIZE 8

static struct VariableList *variables = NULL;

void resize_variable_list(const size_t size) {
    struct Variable *newList = (struct Variable *)realloc(
        variables->list, size * sizeof(struct Variable));
    if (newList == NULL) {
        free_variables();
        print_crash_and_exit("Could't reallocate memory for the variables!\n");
    }
    variables->list = newList;
    variables->size = size;
}

void init_variables(void) {
    variables = (struct VariableList *)malloc(sizeof(struct VariableList));
    if (variables == NULL) {
        print_crash_and_exit("Could't allocate memory for the variables!\n");
    }
    variables->list = NULL;
    variables->size = 0;
    variables->last = 0;
    resize_variable_list(INITIAL_VARIABLE_LIST_SIZE);
}

void free_variables(void) {
    if (variables != NULL) {
        for (size_t i = 0; i < variables->last; i++) {
            free(variables->list[i].name);
            variables->list[i].name = NULL;
        }
        free(variables->list);
        free(variables);
        variables = NULL;
    }
}

size_t search_variable(const char *const name, const unsigned int length) {
    // Sequential search in variables list
    for (size_t i = 0; i < variables->last; i++) {
        if (!strncmp(variables->list[i].name, name, length)) {
            return i;
        }
    }
    return variables->last;
}

void new_variable(char *const name, const unsigned int length,
                  const double value) {
    // Allocates more memory if necessary
    if (variables->last >= variables->size) {
        resize_variable_list(2 * variables->size);
    }
    variables->list[variables->last].name = malloc((length + 1) * sizeof(char));
    if (variables->list[variables->last].name == NULL) {
        print_crash_and_exit("Could't allocate memory for the variable %.*s!\n",
                             length, name);
    }
    strncpy(variables->list[variables->last].name, name, length);
    variables->list[variables->last].name[length] = '\0';
    variables->list[variables->last].value = value;
    variables->last++;
}

double assign_variable(char *const name, const unsigned int length,
                       const double value) {
    const size_t index = search_variable(name, length);
    if (index >= variables->last) {
        new_variable(name, length, value);
    } else {
        variables->list[index].value = value;
    }
    return value;
}

double get_variable(const char *name, const unsigned int length) {
    const size_t index = search_variable(name, length);
    if (index >= variables->last) {
        return NAN;
    }
    return variables->list[index].value;
}

unsigned int max_uint(const unsigned int a, const unsigned int b) {
    return ((a > b) ? a : b);
}

unsigned int longest_name_variables(void) {
    unsigned int length = 0;
    for (size_t i = 0; i < variables->last; i++) {
        length = max_uint(length, strlen(variables->list[i].name));
    }
    return length;
}

void print_variables(void) {
    if (variables->last == 0) {
        return;
    }
    const char *const header = "Name";
    const unsigned int max_length =
        max_uint(longest_name_variables(), strlen(header));
    printf("%-*s Value\n", max_length, header);
    for (size_t i = 0; i < variables->last; i++) {
        printf("%-*s %lg\n", max_length, variables->list[i].name,
               variables->list[i].value);
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