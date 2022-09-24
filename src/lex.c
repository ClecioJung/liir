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

#include "lex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "print_errors.h"

static const size_t initial_token_list_size = 64;

struct Token_List tokens = {0};

void resize_token_list(const size_t size) {
    struct Token *newList = (struct Token *)realloc(tokens.list, size * sizeof(struct Token));
    if (newList == NULL) {
        free_lex();
        print_crash_and_exit("Could't reallocate memory for the lexer!\n");
    }
    tokens.list = newList;
    tokens.capacity = size;
}

void init_lex(void) {
    resize_token_list(initial_token_list_size);
}

void free_lex(void) {
    free(tokens.list);
    tokens.list = NULL;
}

void add_token(const struct Token tok) {
    // Allocates more memory if necessary
    if (tokens.size >= tokens.capacity) {
        resize_token_list(2 * tokens.capacity);
    }
    tokens.list[tokens.size] = tok;
    tokens.size++;
}

static inline void add_op(const char op, const int column) {
    struct Token tok = (struct Token){
        .type = TOK_OPERATOR,
        .column = column,
        .op = op,
    };
    // Unary operator
    if ((op == '-') && (tokens.size > 0)) {
        const struct Token last_token = tokens.list[tokens.size - 1];
        if ((last_token.type == TOK_OPERATOR) ||
            (last_token.type == TOK_UNARY_OPERATOR) ||
            (last_token.type == TOK_DELIMITER)) {
            tok.type = TOK_UNARY_OPERATOR;
        } else if ((last_token.type == TOK_FUNCTION) && (functions[last_token.function_index].arity >= 1)) {
            tok.type = TOK_UNARY_OPERATOR;
            print_column(last_token.column);
            print_warning("Consider using parentheses to pass arguments to functions, so ambiguities are avoided!\n");
        }
    }
    add_token(tok);
}

static inline void add_delimiter(const char delimiter, const int column) {
    struct Token tok = (struct Token){
        .type = TOK_DELIMITER,
        .column = column,
        .op = delimiter,
    };
    add_token(tok);
}

static inline char *add_number(const char *const first, const int column) {
    char *end = (char *)first;
    double number = strtod(first, &end);
    struct Token tok = {
        .type = TOK_NUMBER,
        .column = column,
        .number = number,
    };
    add_token(tok);
    return end;
}

static inline void add_function(const int index, const int column) {
    struct Token tok = {
        .type = TOK_FUNCTION,
        .column = column,
        .function_index = index,
    };
    add_token(tok);
}

static inline void add_variable(const char *const name, const int length, const int column) {
    struct Token tok = {
        .type = TOK_VARIABLE,
        .column = column,
        .name = {
            .string = (char *)name,
            .length = length,
        },
    };
    add_token(tok);
}

static inline char *add_name(const char *const first, const int column) {
    char *end = (char *)first;
    while (isalpha(*end) || isdigit(*end) || *end == '_') {
        end++;
    }
    const int length = (end - first);
    const int function_index = search_function(first, length);
    if (function_index < functions_quantity) {
        add_function(function_index, column);
    } else {
        add_variable(first, length, column);
    }
    return end;
}

int lex(const char *const line) {
    tokens.size = 0;
    const char *c = line;
    while (*c != '\0') {
        if (isdigit(*c) || *c == '.') {
            c = add_number(c, (c - line));
            continue;
        } else if (isalpha(*c) || *c == '_') {
            c = add_name(c, (c - line));
            continue;
        } else if (isspace(*c)) {
            c++;
            continue;
        }
        switch (*c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '=': {
                add_op(*c, (c - line));
                c++;
                break;
            }
            case '(':
            case ')': {
                add_delimiter(*c, (c - line));
                c++;
                break;
            }
            default: {
                print_column(c - line);
                print_error("Unrecognized character at lexical analysis: %s\n", c);
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

void print_token(const struct Token tok) {
    switch (tok.type) {
        case TOK_OPERATOR:
            printf("OPERATOR  ");
            printf("%c\n", tok.op);
            break;
        case TOK_UNARY_OPERATOR:
            printf("UNARY OP. ");
            printf("%c\n", tok.op);
            break;
        case TOK_DELIMITER:
            printf("DELIMITER ");
            printf("%c\n", tok.op);
            break;
        case TOK_NUMBER:
            printf("NUMBER    ");
            printf("%g\n", tok.number);
            break;
        case TOK_VARIABLE:
            printf("VARIABLE  ");
            printf("%.*s\n", tok.name.length, tok.name.string);
            break;
        case TOK_FUNCTION:
            printf("FUNCTION  ");
            printf("%s\n", functions[tok.function_index].name);
            break;
    }
}

void print_tokens(void) {
    if (tokens.size == 0) {
        return;
    }
    printf("List of tokens generated by lexical analysis:\n");
    printf("Token     Value\n");
    for (size_t tk_idx = 0; tk_idx < tokens.size; tk_idx++) {
        print_token(tokens.list[tk_idx]);
    }
    printf("\n");
}

unsigned int max_uint(const unsigned int a, const unsigned int b) {
    return ((a > b) ? a : b);
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