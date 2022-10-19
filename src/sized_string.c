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

#include "sized_string.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_buffer.h"

void print_string_to(FILE *const file, const struct String str) {
    fprintf(file, "%.*s", str.length, str.data);
}

void print_string(const struct String str) {
    print_string_to(stdout, str);
}

struct String create_string(char *const cstr) {
    return (struct String){
        .data = cstr,
        .length = strlen(cstr),
    };
}

struct String create_sized_string(char *const str, const String_Length length) {
    return (struct String){
        .data = str,
        .length = length,
    };
}

bool string_is_empty(const struct String *const str) {
    for (String_Length i = 0; i < str->length; i++) {
        if (!isspace(str->data[i])) {
            return false;
        }
    }
    return true;
}

long int string_to_integer(const struct String string, String_Length *const length) {
    const int base = 10;
    long int number = 0;
    String_Length index = 0;
    for (; index < string.length; index++) {
        const char c = string.data[index];
        if (isdigit(c)) {
            const int digit = c - '0';
            if (number <= LONG_MAX / base) {
                number = base * number + digit;
            } else {
                // Overflow detected
                // In this case we return the maximum positive number that a long int can store
                // We continue the loop, in order to parse all the number
                number = LONG_MAX;
            }
        } else {
            break;
        }
    }
    if (length != NULL) {
        *length = index;
    }
    return number;
}

// Function used by string_to_double
// Computes x * base^exponent
static inline double scale_radix_exp(double x, const int radix, long int exponent) {
    if (x == 0.0) {
        return x;
    }
    if (exponent < 0) {
        while (exponent++ != 0) {
            x /= radix;
        }
    } else {
        while (exponent-- != 0) {
            if (x < -DBL_MAX / radix) {
                return -HUGE_VAL;
            } else if (DBL_MAX / radix < x) {
                return HUGE_VAL;
            }
            x *= radix;
        }
    }
    return x;
}

// libc does not have a function to convert sized strings to floating point,
// so I had to implement my own.
// Based on: https://github.com/gagern/gnulib/blob/master/lib/strtod.c
double string_to_double(const struct String string, String_Length *const length) {
    const int base = 10;
    double number = 0.0;
    bool dotted = false;
    long int exponent = 0;
    String_Length index = 0;
    for (; index < string.length; index++) {
        const char c = string.data[index];
        if (isdigit(c)) {
            const int digit = c - '0';
            if (number <= DBL_MAX / base) {
                number = number * base + digit;
            } else {
                exponent++;
            }
            if (dotted) {
                exponent--;
            }
        } else if (c == '.') {
            if (dotted) {
                break;
            }
            dotted = true;
        } else {
            break;
        }
    }
    if (((index + 1) < string.length) && tolower(string.data[index]) == 'e') {
        const char c = string.data[index + 1];
        if (isdigit(c)) {
            String_Length int_len;
            exponent += string_to_integer(create_sized_string(&string.data[index + 1], (string.length - index - 1)), &int_len);
            index += int_len + 1;
        } else if ((c == '+') || (c == '-')) {
            if ((index + 2 < string.length) && isdigit(string.data[index + 2])) {
                String_Length int_len;
                exponent += (c == '-' ? -1 : 1) * string_to_integer(create_sized_string(&string.data[index + 2], (string.length - index - 2)), &int_len);
                index += int_len + 2;
            }
        }
    }
    if (length != NULL) {
        *length = index;
    }
    return scale_radix_exp(number, base, exponent);
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