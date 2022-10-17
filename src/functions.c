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
#include "sized_string.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.7182818284590452354
#endif

// Defined on main.c
extern double fn_exit(struct Variables *const vars, const double arg);

double fn_clear(struct Variables *const vars, const double arg) {
    (void)arg;
    clear_variables(vars);
    return NAN;
}

double fn_variables(struct Variables *const vars, const double arg) {
    (void)arg;
    if (variable_list_is_empty(vars)) {
        printf("The variables list is empty!\n");
    } else {
        print_variables(vars);
    }
    return NAN;
}

double fn_functions(struct Variables *const vars, const double arg) {
    (void)vars;
    (void)arg;
    print_functions();
    return NAN;
}

double fn_euler(struct Variables *const vars, const double arg) {
    (void)vars;
    (void)arg;
    return M_E;
}

double fn_pi(struct Variables *const vars, const double arg) {
    (void)vars;
    (void)arg;
    return M_PI;
}
double fn_sqrt(struct Variables *const vars, const double arg) {
    (void)vars;
    return sqrt(arg);
}

double fn_cbrt(struct Variables *const vars, const double arg) {
    (void)vars;
    return cbrt(arg);
}

double fn_exp(struct Variables *const vars, const double arg) {
    (void)vars;
    return exp(arg);
}

double fn_log(struct Variables *const vars, const double arg) {
    (void)vars;
    return log(arg);
}

double fn_log10(struct Variables *const vars, const double arg) {
    (void)vars;
    return log10(arg);
}

double fn_log2(struct Variables *const vars, const double arg) {
    (void)vars;
    return log2(arg);
}

double fn_sin(struct Variables *const vars, const double arg) {
    (void)vars;
    return sin(arg);
}

double fn_cos(struct Variables *const vars, const double arg) {
    (void)vars;
    return cos(arg);
}

double fn_tan(struct Variables *const vars, const double arg) {
    (void)vars;
    return tan(arg);
}

double fn_asin(struct Variables *const vars, const double arg) {
    (void)vars;
    return asin(arg);
}

double fn_acos(struct Variables *const vars, const double arg) {
    (void)vars;
    return acos(arg);
}

double fn_atan(struct Variables *const vars, const double arg) {
    (void)vars;
    return atan(arg);
}

const struct Function functions[] = {
    {
        .name = "exit",
        .description = "Closes this process",
        .arity = 0,
        .return_value = false,
        .fn = &fn_exit,
    },
    {
        .name = "clear",
        .description = "clear all variables from memory",
        .arity = 0,
        .return_value = false,
        .fn = &fn_clear,
    },
    {
        .name = "variables",
        .description = "List all current variables",
        .arity = 0,
        .return_value = false,
        .fn = &fn_variables,
    },
    {
        .name = "functions",
        .description = "List all built-in functions",
        .arity = 0,
        .return_value = false,
        .fn = &fn_functions,
    },
    {
        .name = "euler",
        .description = "Returns the euler constant",
        .arity = 0,
        .return_value = true,
        .fn = &fn_euler,
    },
    {
        .name = "pi",
        .description = "Returns the pi constant",
        .arity = 0,
        .return_value = true,
        .fn = &fn_pi,
    },
    {
        .name = "sqrt",
        .description = "Returns the square root of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_sqrt,
    },
    {
        .name = "cbrt",
        .description = "Returns the cubic root of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_cbrt,
    },
    {
        .name = "exp",
        .description = "Returns the exponential of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_exp,
    },
    {
        .name = "ln",
        .description = "Returns the natural logarithm of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_log,
    },
    {
        .name = "log10",
        .description = "Returns the logarithm base 10 of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_log10,
    },
    {
        .name = "log2",
        .description = "Returns the logarithm base 2 of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_log2,
    },
    {
        .name = "sin",
        .description = "Returns the sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_sin,
    },
    {
        .name = "cos",
        .description = "Returns the cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_cos,
    },
    {
        .name = "tan",
        .description = "Returns the tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_tan,
    },
    {
        .name = "asin",
        .description = "Returns the inverse sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_asin,
    },
    {
        .name = "acos",
        .description = "Returns the inverse cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_acos,
    },
    {
        .name = "atan",
        .description = "Returns the inverse tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_atan,
    },
};

const int functions_quantity = (sizeof(functions) / sizeof(functions[0]));

int search_function(const struct String name) {
    // Sequential search in the functions list
    for (int i = 0; i < functions_quantity; i++) {
        const int function_length = strlen(functions[i].name);
        if (name.length == function_length) {
            if (!strncmp(name.data, functions[i].name, name.length)) {
                return i;
            }
        }
    }
    // Didn't found the function in the list
    return functions_quantity;
}

// Defined on main.c
extern unsigned int max_uint(const unsigned int a, const unsigned int b);

static inline unsigned int longest_name_functions(void) {
    unsigned int length = 0;
    for (int i = 0; i < functions_quantity; i++) {
        length = max_uint(length, strlen(functions[i].name));
    }
    return length;
}

void print_functions(void) {
    const char *const header = "Name";
    const unsigned int max_length = max_uint(longest_name_functions(), strlen(header));
    printf("List of built-in functions:\n");
    printf("%-*s Description\n", max_length, header);
    for (int i = 0; i < functions_quantity; i++) {
        printf("%-*s %s\n", max_length, functions[i].name, functions[i].description);
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