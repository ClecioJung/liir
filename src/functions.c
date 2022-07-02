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

#include "functions.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "variables.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.7182818284590452354
#endif

// Defined on main.c
void exit_repl(void);

double exit_func(const double arg) {
    (void)arg;
    exit_repl();
    return NAN;
}

double clear_func(const double arg) {
    (void)arg;
    clear_variables();
    return NAN;
}

double variables_func(const double arg) {
    (void)arg;
    if (variable_list_is_empty()) {
        printf("The variables list is empty!\n");
    } else {
        print_variables();
    }
    return NAN;
}

double functions_func(const double arg) {
    (void)arg;
    print_functions();
    return NAN;
}

double euler(const double arg) {
    (void)arg;
    return M_E;
}

double pi(const double arg) {
    (void)arg;
    return M_PI;
}

const struct Function functions[] = {
    {
        .name = "exit",
        .description = "Closes this process",
        .arity = 0,
        .return_value = false,
        .fn = &exit_func,
    },
    {
        .name = "clear",
        .description = "clear all variables from memory",
        .arity = 0,
        .return_value = false,
        .fn = &clear_func,
    },
    {
        .name = "variables",
        .description = "List all current variables",
        .arity = 0,
        .return_value = false,
        .fn = &variables_func,
    },
    {
        .name = "functions",
        .description = "List all built-in functions",
        .arity = 0,
        .return_value = false,
        .fn = &functions_func,
    },
    {
        .name = "euler",
        .description = "Returns the euler constant",
        .arity = 0,
        .return_value = true,
        .fn = &euler,
    },
    {
        .name = "pi",
        .description = "Returns the pi constant",
        .arity = 0,
        .return_value = true,
        .fn = &pi,
    },
    {
        .name = "sqrt",
        .description = "Returns the square root of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &sqrt,
    },
    {
        .name = "cbrt",
        .description = "Returns the cubic root of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &cbrt,
    },
    {
        .name = "exp",
        .description = "Returns the exponential of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &exp,
    },
    {
        .name = "ln",
        .description = "Returns the natural logarithm of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &log,
    },
    {
        .name = "log10",
        .description = "Returns the logarithm base 10 of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &log10,
    },
    {
        .name = "log2",
        .description = "Returns the logarithm base 2 of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &log2,
    },
    {
        .name = "sin",
        .description = "Returns the sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &sin,
    },
    {
        .name = "cos",
        .description = "Returns the cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &cos,
    },
    {
        .name = "tan",
        .description = "Returns the tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &tan,
    },
    {
        .name = "asin",
        .description = "Returns the inverse sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &asin,
    },
    {
        .name = "acos",
        .description = "Returns the inverse cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &acos,
    },
    {
        .name = "atan",
        .description = "Returns the inverse tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &atan,
    },
};

const int functions_quantity = (sizeof(functions) / sizeof(functions[0]));

int search_function(const char *const name, const int length) {
    // Sequential search in the functions list
    for (int i = 0; i < functions_quantity; i++) {
        const int function_length = strlen(functions[i].name);
        if (length == function_length) {
            if (!strncmp(name, functions[i].name, length)) {
                return i;
            }
        }
    }
    // Didn't found the function in the list
    return functions_quantity;
}

static inline unsigned int longest_name_functions(void) {
    unsigned int length = 0;
    for (int i = 0; i < functions_quantity; i++) {
        length = max_uint(length, strlen(functions[i].name));
    }
    return length;
}

void print_functions(void) {
    const char *const header = "Name";
    const unsigned int max_length =
        max_uint(longest_name_functions(), strlen(header));
    printf("List of built-in functions:\n");
    printf("%-*s Description\n", max_length, header);
    for (int i = 0; i < functions_quantity; i++) {
        printf("%-*s %s\n", max_length, functions[i].name,
               functions[i].description);
    }
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