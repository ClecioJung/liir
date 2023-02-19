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

#include "data-structures/allocator.h"
#include "data-structures/sized_string.h"
#include "functions.h"
#include "printing.h"

struct Lexer create_lex(const size_t initial_size) {
    struct Lexer lexer = (struct Lexer){
        .tokens = allocator_construct(sizeof(struct Token), initial_size),
    };
    if (lexer.tokens.data == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the lexer!\n");
    }
    return lexer;
}

void destroy_lex(struct Lexer *const lexer) {
    if (lexer != NULL) {
        allocator_delete(&lexer->tokens);
    }
}

static inline struct Token get_token_at(struct Lexer *const lexer, size_t index) {
    return ((struct Token *)lexer->tokens.data)[index];
}

// This function returns true if found an error
static inline bool add_token(struct Lexer *const lexer, const struct Token tok) {
    // Check for errors
    if (lexer->tokens.size > 0) {
        const struct Token last_token = ((struct Token *)lexer->tokens.data)[lexer->tokens.size - 1];
        if ((last_token.type == TOK_FUNCTION) && (functions[last_token.function_index].arity >= 1)) {
            if ((tok.type == TOK_DELIMITER) && (tok.op != '(')) {
                print_column(last_token.column);
                print_error("Functions that accept one or more argument, must be followed by parentheses \'(\'!\n");
                return true;
            }
        }
    }
    // Add the token
    int64_t index = allocator_new(&lexer->tokens);
    if (index < 0) {
        print_crash_and_exit("Couldn't allocate more memory for the lexer!\n");
    }
    struct Token *const token = allocator_get(lexer->tokens, index);
    *token = tok;
    return false;
}

static inline struct String parse_name(const struct String string) {
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

static void advance_line(struct String *const line, int *const column, String_Length value) {
    if (value > line->length) {
        value = line->length;
    }
    line->data += value;
    line->length = line->length - value;
    *column += value;
}

// This function returns true if found an error
bool lex(struct Lexer *const lexer, struct String line) {
    if (lexer == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    allocator_free_all(&lexer->tokens);
    for (int column = 0; line.length > 0;) {
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
            const int function_index = search_function(name);
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
                        if (lexer->tokens.size == 0) {
                            tok.type = TOK_UNARY_OPERATOR;
                        } else {
                            const enum Tok_Types tok_type = get_token_at(lexer, lexer->tokens.size - 1).type;
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
        if (add_token(lexer, tok)) {
            return true;
        }
    }
    // Check for errors
    if (lexer->tokens.size > 0) {
        const struct Token last_token = ((struct Token *)lexer->tokens.data)[lexer->tokens.size - 1];
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

void print_token(const struct Token tok) {
    switch (tok.type) {
        case TOK_OPERATOR:
            printf(WHITE_FOREGROUND "OPERATOR  %c\n" RESET_FONT, tok.op);
            break;
        case TOK_UNARY_OPERATOR:
            printf(WHITE_FOREGROUND "UNARY OP. %c\n" RESET_FONT, tok.op);
            break;
        case TOK_DELIMITER:
            printf(WHITE_FOREGROUND "DELIMITER %c\n" RESET_FONT, tok.op);
            break;
        case TOK_NUMBER:
            printf(YELLOW_FOREGROUND "NUMBER    %g\n" RESET_FONT, tok.number);
            break;
        case TOK_NAME:
            printf(MAGENTA_FOREGROUND "NAME      %.*s\n" RESET_FONT, tok.name.length, tok.name.data);
            break;
        case TOK_FUNCTION:
            printf(CYAN_FOREGROUND "FUNCTION  %s\n" RESET_FONT, functions[tok.function_index].name);
            break;
    }
}

void print_tokens(struct Lexer *const lexer) {
    if (lexer == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (lexer->tokens.size == 0) {
        return;
    }
    printf("List of tokens generated by lexical analysis:\n");
    printf("Token     Value\n");
    for (size_t tk_idx = 0; tk_idx < lexer->tokens.size; tk_idx++) {
        print_token(get_token_at(lexer, tk_idx));
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