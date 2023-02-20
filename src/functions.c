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

#include "data-structures/sized_string.h"
#include "printing.h"
#include "variables.h"
#include "input_stream.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.7182818284590452354
#endif

double fn_load(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)first_arg;
    (void)second_arg;
    printf("Please insert the name of the file from which the variables should be loaded?\n");
    const struct String file_name = get_line_from_input();
    load_variables_from_file(vars, file_name);
    return NAN;
}

double fn_save(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)first_arg;
    (void)second_arg;
    printf("Please insert the name of the file in which the variables should be saved?\n");
    const struct String file_name = get_line_from_input();
    save_variables_to_file(vars, file_name);
    return NAN;
}

double fn_clear(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)first_arg;
    (void)second_arg;
    clear_variables(vars);
    return NAN;
}

double fn_delete(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)second_arg;
    if ((first_arg.name.data == NULL) || (first_arg.name.length == 0)) {
        print_error("The function \"delete\" expects a reference to a variable as argument\n");
        return NAN;
    }
    if (delete_variable(vars, first_arg.name) == EXIT_FAILURE) {
        print_error("The variable %.*s does not exist!\n", first_arg.name.length, first_arg.name.data);
    }
    return NAN;
}

double fn_variables(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)first_arg;
    (void)second_arg;
    if (variable_list_is_empty(vars)) {
        printf("The variables list is empty!\n");
    } else {
        print_variables(vars);
    }
    return NAN;
}

double fn_functions(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)first_arg;
    (void)second_arg;
    print_functions();
    return NAN;
}

double fn_euler(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)first_arg;
    (void)second_arg;
    return M_E;
}

double fn_pi(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)first_arg;
    (void)second_arg;
    return M_PI;
}

double fn_ceil(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return ceil(first_arg.value);
}

double fn_floor(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return floor(first_arg.value);
}

double fn_trunc(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return trunc(first_arg.value);
}

double fn_round(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return round(first_arg.value);
}

double fn_abs(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return fabs(first_arg.value);
}

double fn_sqrt(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return sqrt(first_arg.value);
}

double fn_cbrt(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return cbrt(first_arg.value);
}

double fn_exp(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return exp(first_arg.value);
}

double fn_exp2(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return exp2(first_arg.value);
}

double fn_log(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return log(first_arg.value);
}

double fn_log10(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return log10(first_arg.value);
}

double fn_log2(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return log2(first_arg.value);
}

double fn_erf(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return erf(first_arg.value);
}

double fn_gamma(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return tgamma(first_arg.value);
}

double fn_sin(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return sin(first_arg.value);
}

double fn_cos(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return cos(first_arg.value);
}

double fn_tan(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return tan(first_arg.value);
}

double fn_asin(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return asin(first_arg.value);
}

double fn_acos(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return acos(first_arg.value);
}

double fn_atan(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return atan(first_arg.value);
}

double fn_sinh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return sinh(first_arg.value);
}

double fn_cosh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return cosh(first_arg.value);
}

double fn_tanh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return tanh(first_arg.value);
}

double fn_asinh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return asinh(first_arg.value);
}

double fn_acosh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return acosh(first_arg.value);
}

double fn_atanh(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)second_arg;
    return atanh(first_arg.value);
}

double fn_pow(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    return pow(first_arg.value, second_arg.value);
}

double fn_atan2(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    return atan2(first_arg.value, second_arg.value);
}

double fn_hypot(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    return hypot(first_arg.value, second_arg.value);
}

double fn_mod(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    return fmod(first_arg.value, second_arg.value);
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
        .name = "load",
        .description = "Load variables from a file",
        .arity = 0,
        .return_value = false,
        .fn = &fn_load,
    },
    {
        .name = "save",
        .description = "Save variables to a file",
        .arity = 0,
        .return_value = false,
        .fn = &fn_save,
    },
    {
        .name = "clear",
        .description = "Clear all variables from memory",
        .arity = 0,
        .return_value = false,
        .fn = &fn_clear,
    },
    {
        .name = "delete",
        .description = "Deletes a variable from memory",
        .arity = 1,
        .return_value = false,
        .fn = &fn_delete,
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
        .name = "ceil",
        .description = "Return the smallest integral value that is not less than it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_ceil,
    },
    {
        .name = "floor",
        .description = "Returns the largest integral value that is not greather than it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_floor,
    },
    {
        .name = "trunc",
        .description = "Round it's argument to the nearest integer value that is not larger in magnitude than it",
        .arity = 1,
        .return_value = true,
        .fn = &fn_trunc,
    },
    {
        .name = "round",
        .description = "Returns the integral value that is nearest to it's argument, with halfway cases rounded away from zero",
        .arity = 1,
        .return_value = true,
        .fn = &fn_round,
    },
    {
        .name = "abs",
        .description = "Returns the absolute value of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_abs,
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
        .name = "exp2",
        .description = "Returns the exponential base 2 of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_exp2,
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
        .name = "erf",
        .description = "Returns the error function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_erf,
    },
    {
        .name = "gamma",
        .description = "Returns the gamma function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_gamma,
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
    {
        .name = "sinh",
        .description = "Returns the hyperbolic sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_sinh,
    },
    {
        .name = "cosh",
        .description = "Returns the hyperbolic cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_cosh,
    },
    {
        .name = "tanh",
        .description = "Returns the hyperbolic tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_tanh,
    },
    {
        .name = "asinh",
        .description = "Returns the inverse hyperbolic sine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_asinh,
    },
    {
        .name = "acosh",
        .description = "Returns the inverse hyperbolic cosine function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_acosh,
    },
    {
        .name = "atanh",
        .description = "Returns the inverse hyperbolic tangent function of it's argument",
        .arity = 1,
        .return_value = true,
        .fn = &fn_atanh,
    },
    {
        .name = "pow",
        .description = "Returns it's first argument raised to the power of the second",
        .arity = 2,
        .return_value = true,
        .fn = &fn_pow,
    },
    {
        .name = "atan2",
        .description = "Returns the inverse tangent function, considering the signs of the arguments to determine the quadrant of the result",
        .arity = 2,
        .return_value = true,
        .fn = &fn_atan2,
    },
    {
        .name = "hypot",
        .description = "Returns the square root of the sum of the squares of it's arguments",
        .arity = 2,
        .return_value = true,
        .fn = &fn_hypot,
        
    },
    {
        .name = "mod",
        .description = "Returns the remainder of the division of it's arguments (rounded towards zero)",
        .arity = 2,
        .return_value = true,
        .fn = &fn_mod,
        
    },
};

const int functions_quantity = (sizeof(functions) / sizeof(functions[0]));

int search_function(const struct String name) {
    // Sequential search in the functions list
    for (int i = 0; i < functions_quantity; i++) {
        const String_Length function_length = (String_Length)strlen(functions[i].name);
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
        length = max_uint(length, (unsigned int)strlen(functions[i].name));
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