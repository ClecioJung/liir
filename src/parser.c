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

#include "parser.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "lex.h"
#include "print_errors.h"
#include "variables.h"

struct Parser create_parser(struct Lexer *const lexer, struct Variables *const vars, const size_t initial_size) {
    if ((lexer == NULL) || (vars == NULL)) {
        print_crash_and_exit("Invalid call to function \"create_parser()\"!\n");
    }
    return (struct Parser){
        .lexer = lexer,
        .vars = vars,
        .nodes = allocator_construct(sizeof(struct Token_Node), initial_size),
    };
}

void destroy_parser(struct Parser *const parser) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"destroy_parser()\"!\n");
        return;
    }
    allocator_delete(&parser->nodes);
}

int64_t new_node(struct Parser *const parser, const struct Token tok) {
    int64_t index = allocator_new(&parser->nodes);
    struct Token_Node *const node = allocator_get(parser->nodes, index);
    *node = (struct Token_Node){
        .tok = tok,
        .left_idx = -1,
        .right_idx = -1,
    };
    return index;
}

static inline struct Token_Node *get_node_ptr(struct Parser *const parser, int64_t index) {
    return allocator_get(parser->nodes, index);
}

static inline struct Token get_tok(struct Parser *const parser, int64_t index) {
    return get_node_ptr(parser, index)->tok;
}

static inline int64_t get_right_idx(struct Parser *const parser, int64_t index) {
    return get_node_ptr(parser, index)->right_idx;
}

static inline int64_t get_left_idx(struct Parser *const parser, int64_t index) {
    return get_node_ptr(parser, index)->left_idx;
}

static inline int64_t is_valid_idx(struct Parser *const parser, int64_t index) {
    return allocator_is_valid(parser->nodes, index);
}

static inline int64_t is_invalid_idx(struct Parser *const parser, int64_t index) {
    return allocator_is_invalid(parser->nodes, index);
}

static int get_op_precedence(const char op) {
    const char precedence[] = {
        '^',
        '/',
        '*',
        '-',
        '+',
    };
    const int num_ops = sizeof(precedence) / sizeof(precedence[0]);
    for (int index = 0; index < num_ops; index++) {
        if (op == precedence[index]) {
            return index;
        }
    }
    return num_ops;
}

static inline bool check_parser_right_error(struct Parser *const parser, const int64_t previous_idx, const int64_t current_idx) {
    if (is_invalid_idx(parser, previous_idx) || is_invalid_idx(parser, current_idx)) {
        print_crash_and_exit("Invalid call to function \"check_parser_right_error()\"!\n");
        return true;
    }
    struct Token previous_tok = get_tok(parser, previous_idx);
    struct Token current_tok = get_tok(parser, current_idx);
    if ((previous_tok.type == TOK_NUMBER) || (previous_tok.type == TOK_VARIABLE)) {
        if (current_tok.type == TOK_NUMBER) {
            print_column(current_tok.column);
            print_error("Unexpected number! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
        if (current_tok.type == TOK_VARIABLE) {
            print_column(current_tok.column);
            print_error("Unexpected variable! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
        if (current_tok.type == TOK_FUNCTION) {
            print_column(current_tok.column);
            print_error("Unexpected function! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
    }
    if (previous_tok.type == TOK_FUNCTION) {
        if (functions[previous_tok.function_index].arity == 0) {
            if ((current_tok.type == TOK_NUMBER) || (current_tok.type == TOK_VARIABLE)) {
                print_column(current_tok.column);
                print_warning("The function \"%s\" accepts no argument!\n", functions[previous_tok.function_index].name);
            }
        }
        if (current_tok.type == TOK_OPERATOR) {
            if (!functions[previous_tok.function_index].return_value) {
                print_column(previous_tok.column);
                print_error("The function \"%s\" returns no value, so it can't be used in a expression!\n", functions[previous_tok.function_index].name);
                return true;
            }
        }
    }
    if (previous_tok.type == TOK_OPERATOR) {
        if ((current_tok.type == TOK_FUNCTION) && (!functions[current_tok.function_index].return_value)) {
            print_column(current_tok.column);
            print_error("The function \"%s\" returns no value, so it can't be used in a expression!\n", functions[current_tok.function_index].name);
            return true;
        }
    }
    return false;
}

bool insert_node_right(struct Parser *const parser, int64_t *const head_idx, const int64_t node_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = *head_idx;
        while (is_valid_idx(parser, get_right_idx(parser, previous_idx))) {
            previous_idx = get_right_idx(parser, previous_idx);
        }
        if (check_parser_right_error(parser, previous_idx, node_idx)) {
            return true;
        }
        get_node_ptr(parser, previous_idx)->right_idx = node_idx;
    }
    return false;
}

bool insert_new_op(struct Parser *const parser, int64_t *const head_idx, const int64_t node_idx, const int64_t max_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = -1;
        {  // Search for the correct place to insert the node
            int64_t next_idx = *head_idx;
            const int precedence = get_op_precedence(get_tok(parser, node_idx).op);
            while (is_valid_idx(parser, next_idx) && (get_tok(parser, next_idx).type == TOK_OPERATOR)) {
                if ((next_idx == max_idx) || (get_op_precedence(get_tok(parser, next_idx).op) <= precedence)) {
                    break;
                }
                previous_idx = next_idx;
                next_idx = get_right_idx(parser, next_idx);
            }
        }
        // Insert new node
        if (is_invalid_idx(parser, previous_idx)) {
            get_node_ptr(parser, node_idx)->left_idx = *head_idx;
            *head_idx = node_idx;
        } else {
            get_node_ptr(parser, node_idx)->left_idx = get_right_idx(parser, previous_idx);
            get_node_ptr(parser, previous_idx)->right_idx = node_idx;
        }
    }
    return false;
}

int64_t parse_parentheses(struct Parser *const parser, size_t *const tk_idx) {
    if (tk_idx == NULL) {
        print_crash_and_exit("Invalid call to function \"parse_parentheses()\"!\n");
        return -1;
    }
    int64_t head_idx = -1;
    int64_t last_parentheses_idx = -1;
    while ((*tk_idx < parser->lexer->tokens.size) && (((struct Token *)parser->lexer->tokens.data)[*tk_idx].op != ')')) {
        const struct Token current_token = ((struct Token *)parser->lexer->tokens.data)[*tk_idx];
        if ((current_token.type == TOK_DELIMITER) && (current_token.op == '(')) {
            (*tk_idx)++;
            last_parentheses_idx = parse_parentheses(parser, tk_idx);
            if (insert_node_right(parser, &head_idx, last_parentheses_idx)) {
                return -1;
            }
            if (*tk_idx >= parser->lexer->tokens.size) {
                print_column(current_token.column);
                print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                return -1;
            }
        } else if (current_token.type == TOK_OPERATOR) {
            int64_t node_idx = new_node(parser, current_token);
            if (insert_new_op(parser, &head_idx, node_idx, last_parentheses_idx)) {
                return -1;
            }
        } else {
            int64_t node_idx = new_node(parser, current_token);
            if (insert_node_right(parser, &head_idx, node_idx)) {
                return -1;
            }
        }
        (*tk_idx)++;
    }
    return head_idx;
}

int64_t parse(struct Parser *const parser) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"parse()\"!\n");
        return -1;
    }
    if (parser->lexer->tokens.size == 0) {
        return -1;
    }
    allocator_free_all(&parser->nodes);
    size_t tk_idx = 0;
    int64_t head_idx = parse_parentheses(parser, &tk_idx);
    if ((tk_idx < parser->lexer->tokens.size) && (((struct Token *)parser->lexer->tokens.data)[tk_idx].op == ')')) {
        print_column(((struct Token *)parser->lexer->tokens.data)[tk_idx].column);
        print_error("Mismatched delimiters! Unexpected closing parentheses!\n");
        return -1;
    }
    return head_idx;
}

double evaluate(struct Parser *const parser, const int64_t node_idx, enum Evaluation_Status *const status) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"evaluate()\"!\n");
        return -1;
    }
    if (is_invalid_idx(parser, node_idx)) {
        return NAN;
    }
    if ((status == NULL) || (*status == Eval_Error)) {
        return NAN;
    }
    switch (get_tok(parser, node_idx).type) {
        case TOK_OPERATOR:
            switch (get_tok(parser, node_idx).op) {
                case '+':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) + evaluate(parser, get_right_idx(parser, node_idx), status));
                case '-':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) - evaluate(parser, get_right_idx(parser, node_idx), status));
                case '*':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) * evaluate(parser, get_right_idx(parser, node_idx), status));
                case '/':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) / evaluate(parser, get_right_idx(parser, node_idx), status));
                case '^':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return pow(evaluate(parser, get_left_idx(parser, node_idx), status), evaluate(parser, get_right_idx(parser, node_idx), status));
                case '=': {
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    if (get_tok(parser, get_left_idx(parser, node_idx)).type == TOK_FUNCTION) {
                        print_column(get_tok(parser, get_left_idx(parser, node_idx)).column);
                        print_error("Cannot create a variable named \"%s\", because already exists a function with this name!\n", functions[get_tok(parser, get_left_idx(parser, node_idx)).function_index].name);
                        *status = Eval_Error;
                        return NAN;
                    }
                    if (get_tok(parser, get_left_idx(parser, node_idx)).type != TOK_VARIABLE) {
                        print_column(get_tok(parser, get_left_idx(parser, node_idx)).column);
                        print_error("Expected variable name for atribution!\n");
                        *status = Eval_Error;
                        return NAN;
                    }
                    const double result = evaluate(parser, get_right_idx(parser, node_idx), status);
                    if (*status != Eval_Error) {
                        return assign_variable(parser->vars, get_tok(parser, get_left_idx(parser, node_idx)).name.string, get_tok(parser, get_left_idx(parser, node_idx)).name.length, result);
                    } else {
                        return NAN;
                    }
                }
                default:
                    print_column(get_tok(parser, node_idx).column);
                    print_error("Invalid binary operator at evaluation phase: %c\n", get_tok(parser, node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (get_tok(parser, node_idx).op) {
                case '-':
                    if (is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (-evaluate(parser, get_right_idx(parser, node_idx), status));
                default:
                    print_column(get_tok(parser, node_idx).column);
                    print_error("Invalid unary operator at evaluation phase: %c\n", get_tok(parser, node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_NUMBER:
            return get_tok(parser, node_idx).number;
        case TOK_FUNCTION: {
            const struct Function function = functions[get_tok(parser, node_idx).function_index];
            if (function.fn == NULL) {
                print_column(get_tok(parser, node_idx).column);
                print_error("The function \"%s\" was not properly initialized!\n", function.name);
                *status = Eval_Error;
                return NAN;
            }
            if ((function.arity >= 1) && is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                print_column(get_tok(parser, node_idx).column);
                print_warning("Did you forget to pass a argument to the function \"%s\"?\n", function.name);
            }
            if ((!function.return_value) && (*status != Eval_Error)) {
                *status = Eval_Dont_Print;
            }
            return function.fn(parser->vars, evaluate(parser, get_right_idx(parser, node_idx), status));
        }
        case TOK_VARIABLE: {
            int index;
            if (search_variable(parser->vars, get_tok(parser, node_idx).name.string, get_tok(parser, node_idx).name.length, &index) != 0) {
                print_column(get_tok(parser, node_idx).column);
                print_error("Unrecognized name: \"%.*s\"!\n", get_tok(parser, node_idx).name.length, get_tok(parser, node_idx).name.string);
                *status = Eval_Error;
                return NAN;
            }
            return get_variable(parser->vars, index);
        }
        case TOK_DELIMITER:
            print_column(get_tok(parser, node_idx).column);
            print_error("Unexpected delimiter at evaluation phase: %c\n", get_tok(parser, node_idx).op);
            *status = Eval_Error;
            return NAN;
        default:
            print_column(get_tok(parser, node_idx).column);
            print_error("Invalid token at evaluation phase: %c\n", get_tok(parser, node_idx).op);
            *status = Eval_Error;
            return NAN;
    }
}

void print_node(struct Parser *const parser, const int64_t node_idx, const unsigned int level) {
    if (is_invalid_idx(parser, node_idx)) {
        return;
    }
    print_token(get_tok(parser, node_idx));
    if (get_left_idx(parser, node_idx) >= 0) {
        printf("%*sLEFT:  ", level * 2, "");
        print_node(parser, get_left_idx(parser, node_idx), level + 1);
    }
    if (get_right_idx(parser, node_idx) >= 0) {
        printf("%*sRIGHT: ", level * 2, "");
        print_node(parser, get_right_idx(parser, node_idx), level + 1);
    }
}

void print_tree(struct Parser *const parser, const int64_t head_idx) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"print_tree()\"!\n");
        return;
    }
    printf("Abstract syntax tree (AST) generated by the parser:\n");
    printf("HEAD:  ");
    print_node(parser, head_idx, 0);
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