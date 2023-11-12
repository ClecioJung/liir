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
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data-structures/sized_string.h"
#include "functions.h"
#include "printing.h"
#include "platform.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "data-structures/dynamic_array.h"

struct Lexer create_lex(const size_t initial_size) {
    struct Lexer lexer = (struct Lexer){0};
    lexer.tokens = array_new(sizeof(struct Token), initial_size);
    if (lexer.tokens == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the lexer!\n");
    }
    return lexer;
}

void destroy_lex(struct Lexer *const lexer) {
    array_del(lexer->tokens);
}

static void advance_line(struct String *const line, size_t *const column, String_Length value) {
    if (value > line->length) {
        value = line->length;
    }
    line->data += value;
    line->length = line->length - value;
    *column += value;
}

// This function returns true if found an error
bool lex(struct Lexer *const lexer, struct String line) {
    array_free_all(lexer->tokens);
    for (size_t column = 0; line.length > 0;) {
        const char c = *line.data;
        struct Token tok = (struct Token){
            .column = column,
        };
        if (isspace(c)) {
            advance_line(&line, &column, 1);
            continue;
        } else if (isdigit(c) || (c == '.')) {
            String_Length length;
            tok.number = parse_number(line, &length);
            advance_line(&line, &column, length);
            tok.type = TOK_NUMBER;
        } else if (isalpha(c) || c == '_') {
            const struct String name = parse_name(line);
            const size_t function_index = search_function(name);
            if (function_index < functions_quantity) {
                tok.type = TOK_FUNCTION;
                tok.function_index = function_index;
            } else {
                tok.type = TOK_NAME;
                tok.name = name;
            }
            advance_line(&line, &column, name.length);
        } else {
            switch (c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '=': {
                tok.type = TOK_OPERATOR;
                tok.op = c;
                // Check for unary operator
                if (c == '-') {
                    if (array_size(lexer->tokens) == 0) {
                        tok.type = TOK_UNARY_OPERATOR;
                    } else {
                        const enum Tok_Types tok_type = array_last(lexer->tokens).type;
                        if ((tok_type == TOK_OPERATOR) || (tok_type == TOK_UNARY_OPERATOR) || (tok_type == TOK_DELIMITER)) {
                            tok.type = TOK_UNARY_OPERATOR;
                        }
                    }
                }
                advance_line(&line, &column, 1);
                break;
            }
            case '(':
            case ')':
            case ',': {
                tok.type = TOK_DELIMITER;
                tok.op = c;
                advance_line(&line, &column, 1);
                break;
            }
            default: {
                print_column(column);
                print_error("Unrecognized character at lexical analysis: %c\n", c);
                return true;
            }
            }
        }
        // Check for errors
        if (array_size(lexer->tokens) > 0) {
            const struct Token last_token = array_last(lexer->tokens);
            if ((last_token.type == TOK_FUNCTION) && (functions[last_token.function_index].arity >= 1)) {
                if ((tok.type == TOK_DELIMITER) && (tok.op != '(')) {
                    print_column(last_token.column);
                    print_error("Functions that accept one or more argument, must be followed by parentheses \'(\'!\n");
                    return true;
                }
            }
        }
        // Add the token
        array_push(lexer->tokens, tok);
    }
    // Check for errors
    if (array_size(lexer->tokens) > 0) {
        const struct Token last_token = array_last(lexer->tokens);
        if ((last_token.type == TOK_FUNCTION) && (functions[last_token.function_index].arity >= 1)) {
            print_column(last_token.column);
            print_error("Functions with one or more argument must be followed by parentheses \'(\'!\n");
            return true;
        }
    }
    return false;
}

char *get_token_type(const enum Tok_Types type) {
    switch (type) {
    case TOK_OPERATOR:
        return "operator";
    case TOK_UNARY_OPERATOR:
        return "unary operator";
    case TOK_DELIMITER:
        return "delimiter";
    case TOK_NUMBER:
        return "number";
    case TOK_NAME:
        return "name";
    case TOK_FUNCTION:
        return "function";
    }
    return NULL;
}

void print_token_string(const struct Token tok) {
    switch (tok.type) {
    case TOK_OPERATOR:
        printf("%c", tok.op);
        break;
    case TOK_UNARY_OPERATOR:
        printf("%c", tok.op);
        break;
    case TOK_DELIMITER:
        printf("%c", tok.op);
        break;
    case TOK_NUMBER:
        printf("%g", tok.number);
        break;
    case TOK_NAME:
        printf("%.*s", tok.name.length, tok.name.data);
        break;
    case TOK_FUNCTION:
        printf("%s", functions[tok.function_index].name);
        break;
    }
}

void print_token(const struct Token tok) {
    switch (tok.type) {
    case TOK_OPERATOR:
        foreground_color(stdout, WHITE_FOREGROUND);
        printf("OPERATOR  %c\n", tok.op);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    case TOK_UNARY_OPERATOR:
        foreground_color(stdout, WHITE_FOREGROUND);
        printf("UNARY OP. %c\n", tok.op);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    case TOK_DELIMITER:
        foreground_color(stdout, WHITE_FOREGROUND);
        printf("DELIMITER %c\n", tok.op);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    case TOK_NUMBER:
        foreground_color(stdout, YELLOW_FOREGROUND);
        printf("NUMBER    %g\n", tok.number);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    case TOK_NAME:
        foreground_color(stdout, MAGENTA_FOREGROUND);
        printf("NAME      %.*s\n", tok.name.length, tok.name.data);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    case TOK_FUNCTION:
        foreground_color(stdout, CYAN_FOREGROUND);
        printf("FUNCTION  %s\n", functions[tok.function_index].name);
        foreground_color(stdout, DEFAULT_FOREGROUND);
        break;
    }
}

void print_tokens(struct Lexer *const lexer) {
    if (array_size(lexer->tokens) == 0) {
        return;
    }
    printf("List of tokens generated by lexical analysis:\n");
    printf("Token     Value\n");
    for (size_t tk_idx = 0; tk_idx < array_size(lexer->tokens); tk_idx++) {
        print_token(lexer->tokens[tk_idx]);
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
