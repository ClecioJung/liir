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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_errors.h"

#define INITIAL_TOKEN_LIST_SIZE 8

static struct TokenList *tokens = NULL;

void resize_token_list(const size_t size) {
    struct Token *newList =
        (struct Token *)realloc(tokens->list, size * sizeof(struct Token));
    if (newList == NULL) {
        free_lex();
        print_crash_and_exit("Could't reallocate memory for the lexer!\n");
    }
    tokens->list = newList;
    tokens->size = size;
}

void init_lex(void) {
    tokens = (struct TokenList *)malloc(sizeof(struct TokenList));
    if (tokens == NULL) {
        print_crash_and_exit("Could't allocate memory for the lexer!\n");
    }
    tokens->list = NULL;
    tokens->size = 0;
    tokens->last = 0;
    resize_token_list(INITIAL_TOKEN_LIST_SIZE);
}

void free_lex(void) {
    if (tokens != NULL) {
        free(tokens->list);
        free(tokens);
        tokens = NULL;
    }
}

void add_token(const struct Token tok) {
    // Allocates more memory if necessary
    if (tokens->last >= tokens->size) {
        resize_token_list(2 * tokens->size);
    }
    tokens->list[tokens->last] = tok;
    tokens->last++;
}

static inline void add_op(const char op) {
    struct Token tok = (struct Token){
        .type = TOK_OPERATOR,
        .op = op,
    };
    add_token(tok);
}

static inline char *add_number(const char *const first) {
    char *end = (char *)first;
    double number = strtod(first, &end);
    struct Token tok = {
        .type = TOK_NUMBER,
        .number = number,
    };
    add_token(tok);
    return end;
}

static inline char *add_name(const char *const first) {
    char *end = (char *)first;
    while (isalpha(*end) || isdigit(*end) || *end == '_') {
        end++;
    }
    struct Token tok = {
        .type = TOK_NAME,
        .name =
            {
                .string = (char *)first,
                .length = end - first,
            },
    };
    add_token(tok);
    return end;
}

struct TokenList *lex(const char *const line) {
    tokens->last = 0;
    const char *c = line;
    while (*c != '\0') {
        if (isdigit(*c) || *c == '.') {
            c = add_number(c);
            continue;
        } else if (isalpha(*c) || *c == '_') {
            c = add_name(c);
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
            case '^': {
                add_op(*c);
                c++;
                break;
            }
            default: {
                print_error("Unrecognized token: %s\n", c);
                return NULL;
            }
        }
    }
    return tokens;
}

void print_token(const struct Token tok) {
    switch (tok.type) {
        case TOK_OPERATOR:
            fputs("OPERATOR ", stdout);
            printf("%c\n", tok.op);
            break;
        case TOK_NUMBER:
            fputs("NUMBER   ", stdout);
            printf("%g\n", tok.number);
            break;
        case TOK_NAME:
            fputs("NAME     ", stdout);
            printf("%.*s\n", tok.name.length, tok.name.string);
            break;
    }
}

void print_tokens(const struct TokenList *tokens) {
    if (tokens->last == 0) return;
    fputs("Token    Value\n", stdout);
    for (size_t tkIndex = 0; tkIndex < tokens->last; tkIndex++) {
        print_token(tokens->list[tkIndex]);
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