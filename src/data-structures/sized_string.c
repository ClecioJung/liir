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

void print_string_to(FILE *const file, const struct String str) {
    fprintf(file, "%.*s", str.length, str.data);
}

void print_string(const struct String str) {
    print_string_to(stdout, str);
}

struct String create_string(char *const cstr) {
    return (struct String){
        .data = cstr,
        .length = (String_Length)strlen(cstr),
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

int string_compare(const struct String str1, const struct String str2) {
    const String_Length min_len = str1.length < str2.length ? str1.length : str2.length;
    const int comp = strncmp(str1.data, str2.data, min_len);
    if (!comp) {
        return ((int)str1.length - (int)str2.length);
    }
    return comp;
}

// Parses a integer number using base 10, 16, 8 or 2
static long int string_to_int_base(const char *const data, const String_Length length, const int base, String_Length *const num_len) {
    long int number = 0;
    String_Length index = 0;
    for (; index < length; index++) {
        const char c = data[index];
        int digit = 0;
        if (isdigit(c)) {
            digit = c - '0';
            if (digit >= base) {  // Check if we have a invalid digit for the specified base
                break;
            }
        } else if ((base == 16) && isxdigit(c)) {
            digit = tolower(c) - 'a' + 10;
        } else {
            break;
        }
        if (number <= LONG_MAX / base) {
            number = base * number + digit;
        } else {
            // Overflow detected
            // In this case we return the maximum positive number that a long int can store
            // We continue the loop, in order to parse all the number
            number = LONG_MAX;
        }
    }
    if (num_len != NULL) {
        *num_len = index;
    }
    return number;
}

long int string_to_integer(const struct String string, String_Length *const num_len) {
    return string_to_int_base(string.data, string.length, 10, num_len);
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
double string_to_double(const struct String string, String_Length *const num_len) {
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
    // Parses the exponent
    if (((index + 1) < string.length) && tolower(string.data[index]) == 'e') {
        bool negative = false;
        String_Length temp_index = index + 1;
        if (string.data[temp_index] == '+') {
            temp_index++;
        } else if (string.data[temp_index] == '-') {
            temp_index++;
            negative = true;
        }
        String_Length int_len;
        long int value = string_to_int_base(&string.data[temp_index], (string.length - temp_index), 10, &int_len);
        if (negative) {
            value = -value;
        }
        if (int_len != 0) {
            index = temp_index + int_len;
            // Check for possible overflows
            if (exponent > 0) {
                exponent = ((LONG_MAX - exponent) < value) ? LONG_MAX : (exponent + value);
            } else {
                exponent = (value < (LONG_MIN - exponent)) ? LONG_MIN : (exponent + value);
            }
        }
    }
    if (num_len != NULL) {
        *num_len = index;
    }
    return scale_radix_exp(number, base, exponent);
}

// Parses a generic number
double parse_number(const struct String string, String_Length *const num_len) {
    // Check if is a integer and parses it
    if (string.data[0] == '0') {
        if (string.length >= 3) {
            String_Length int_len = 0;
            double number = NAN;
            const char c = (char)tolower(string.data[1]);
            switch (c) {
                case 'x':  // It is a hexadecimal integer because it begins with "0x"
                    number = (double)string_to_int_base(&string.data[2], (string.length - 2), 16, &int_len);
                    break;
                case 'o':  // It is a octal integer because it begins with "0o"
                    number = (double)string_to_int_base(&string.data[2], (string.length - 2), 8, &int_len);
                    break;
                case 'b':  // It is a binary integer because it begins with "0b"
                    number = (double)string_to_int_base(&string.data[2], (string.length - 2), 2, &int_len);
                    break;
            }
            if (int_len != 0) {
                if (num_len != NULL) {
                    *num_len = int_len + 2;
                }
                return number;
            }
        }
    }
    // If wasn't a integer, parses it as a double
    return string_to_double(string, num_len);
}

struct String parse_name(const struct String string) {
    if ((string.length == 0) || isdigit(string.data[0])) {
        return (struct String){0};
    }
    String_Length index = 0;
    for (; index < string.length; index++) {
        const char c = string.data[index];
        if (!isalnum(c) && (c != '_')) {
            break;
        }
    }
    return (struct String){
        .data = string.data,
        .length = index,
    };
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