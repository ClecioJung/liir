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
        .arity = 0,
        .return_value = false,
        .fn = &exit_func,
    },
    {
        .name = "clear",
        .arity = 0,
        .return_value = false,
        .fn = &clear_func,
    },
    {
        .name = "variables",
        .arity = 0,
        .return_value = false,
        .fn = &variables_func,
    },
    {
        .name = "functions",
        .arity = 0,
        .return_value = false,
        .fn = &functions_func,
    },
    {
        .name = "euler",
        .arity = 0,
        .return_value = true,
        .fn = &euler,
    },
    {
        .name = "pi",
        .arity = 0,
        .return_value = true,
        .fn = &pi,
    },
    {
        .name = "sqrt",
        .arity = 1,
        .return_value = true,
        .fn = &sqrt,
    },
    {
        .name = "cbrt",
        .arity = 1,
        .return_value = true,
        .fn = &cbrt,
    },
    {
        .name = "exp",
        .arity = 1,
        .return_value = true,
        .fn = &exp,
    },
    {
        .name = "ln",
        .arity = 1,
        .return_value = true,
        .fn = &log,
    },
    {
        .name = "log10",
        .arity = 1,
        .return_value = true,
        .fn = &log10,
    },
    {
        .name = "log2",
        .arity = 1,
        .return_value = true,
        .fn = &log2,
    },
    {
        .name = "sin",
        .arity = 1,
        .return_value = true,
        .fn = &sin,
    },
    {
        .name = "cos",
        .arity = 1,
        .return_value = true,
        .fn = &cos,
    },
    {
        .name = "tan",
        .arity = 1,
        .return_value = true,
        .fn = &tan,
    },
    {
        .name = "asin",
        .arity = 1,
        .return_value = true,
        .fn = &asin,
    },
    {
        .name = "acos",
        .arity = 1,
        .return_value = true,
        .fn = &acos,
    },
    {
        .name = "atan",
        .arity = 1,
        .return_value = true,
        .fn = &atan,
    },
    {
        .name = "sinh",
        .arity = 1,
        .return_value = true,
        .fn = &sinh,
    },
    {
        .name = "cosh",
        .arity = 1,
        .return_value = true,
        .fn = &cosh,
    },
    {
        .name = "tanh",
        .arity = 1,
        .return_value = true,
        .fn = &tanh,
    },
    {
        .name = "asinh",
        .arity = 1,
        .return_value = true,
        .fn = &asinh,
    },
    {
        .name = "acosh",
        .arity = 1,
        .return_value = true,
        .fn = &acosh,
    },
    {
        .name = "atanh",
        .arity = 1,
        .return_value = true,
        .fn = &atanh,
    },
};

const int functions_quantity = (sizeof(functions) / sizeof(functions[0]));

int search_function(const char *const name) {
    // Sequential search in the functions list
    for (int i = 0; i < functions_quantity; i++) {
        if (!strncmp(name, functions[i].name, strlen(functions[i].name))) {
            return i;
        }
    }
    // Didn't found the function in the list
    return functions_quantity;
}

void print_functions(void) {
    printf("List of built-in functions:\n");
    for (int i = 0; i < functions_quantity; i++) {
        printf("%s\n", functions[i].name);
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