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

#include "allocator.h"
#include "functions.h"
#include "print_errors.h"
#include "string_buffer.h"

struct Lexer create_lex(const size_t initial_size) {
    return (struct Lexer){
        .tokens = allocator_construct(sizeof(struct Token), initial_size),
    };
}

void destroy_lex(struct Lexer *const lexer) {
    if (lexer == NULL) {
        print_crash_and_exit("Invalid call to function \"destroy_lex()\"!\n");
        return;
    }
    allocator_delete(&lexer->tokens);
}

void add_token(struct Lexer *const lexer, const struct Token tok) {
    int64_t index = allocator_new(&lexer->tokens);
    struct Token *const token = allocator_get(lexer->tokens, index);
    *token = tok;
}

static inline void add_op(struct Lexer *const lexer, const char op, const int column) {
    struct Token tok = (struct Token){
        .type = TOK_OPERATOR,
        .column = column,
        .op = op,
    };
    // Unary operator
    if ((op == '-') && (lexer->tokens.size > 0)) {
        const struct Token last_token = ((struct Token *)lexer->tokens.data)[lexer->tokens.size - 1];
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
    add_token(lexer, tok);
}

static inline void add_delimiter(struct Lexer *const lexer, const char delimiter, const int column) {
    struct Token tok = (struct Token){
        .type = TOK_DELIMITER,
        .column = column,
        .op = delimiter,
    };
    add_token(lexer, tok);
}

static inline int add_number(struct Lexer *const lexer, const struct String line, const int column) {
    char *first = (char *)&line.data[column];
    char *end = first;
    double number = strtod(first, &end);
    struct Token tok = {
        .type = TOK_NUMBER,
        .column = column,
        .number = number,
    };
    add_token(lexer, tok);
    return (end - first);
}

static inline void add_function(struct Lexer *const lexer, const int index, const int column) {
    struct Token tok = {
        .type = TOK_FUNCTION,
        .column = column,
        .function_index = index,
    };
    add_token(lexer, tok);
}

static inline void add_variable(struct Lexer *const lexer, const char *const name, const int length, const int column) {
    struct Token tok = {
        .type = TOK_VARIABLE,
        .column = column,
        .name = {
            .data = (char *)name,
            .length = length,
        },
    };
    add_token(lexer, tok);
}

static inline int add_name(struct Lexer *const lexer, const struct String line, const int column) {
    int index = column;
    char c = line.data[index];
    while ((index < line.length) && (isalpha(c) || isdigit(c) || (c == '_'))) {
        index++;
        c = line.data[index];
    }
    const int length = index - column;
    const int function_index = search_function(&line.data[column], length);
    if (function_index < functions_quantity) {
        add_function(lexer, function_index, column);
    } else {
        add_variable(lexer, &line.data[column], length, column);
    }
    return length;
}

int lex(struct Lexer *const lexer, const struct String line) {
    if (lexer == NULL) {
        print_crash_and_exit("Invalid call to function \"lex()\"!\n");
        return EXIT_FAILURE;
    }
    allocator_free_all(&lexer->tokens);
    for (int i = 0; i < line.length;) {
        const char c = line.data[i];
        if (isdigit(c) || c == '.') {
            i += add_number(lexer, line, i);
            continue;
        } else if (isalpha(c) || c == '_') {
            i += add_name(lexer, line, i);
            continue;
        } else if (isspace(c)) {
            i++;
            continue;
        }
        switch (c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '=': {
                add_op(lexer, c, i);
                i++;
                break;
            }
            case '(':
            case ')': {
                add_delimiter(lexer, c, i);
                i++;
                break;
            }
            default: {
                print_column(i);
                print_error("Unrecognized character at lexical analysis: %c\n", c);
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
            printf("%.*s\n", tok.name.length, tok.name.data);
            break;
        case TOK_FUNCTION:
            printf("FUNCTION  ");
            printf("%s\n", functions[tok.function_index].name);
            break;
    }
}

void print_tokens(struct Lexer *const lexer) {
    if (lexer == NULL) {
        print_crash_and_exit("Invalid call to function \"print_tokens()\"!\n");
        return;
    }
    if (lexer->tokens.size == 0) {
        return;
    }
    printf("List of tokens generated by lexical analysis:\n");
    printf("Token     Value\n");
    for (size_t tk_idx = 0; tk_idx < lexer->tokens.size; tk_idx++) {
        print_token(((struct Token *)lexer->tokens.data)[tk_idx]);
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