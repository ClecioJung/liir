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

static const size_t initial_tree_size = 1024;

static struct Tree tree = {0};

void resize_tree(const int size) {
    struct Token_Node *newTree = (struct Token_Node *)realloc(tree.nodes, size * sizeof(struct Token_Node));
    if (newTree == NULL) {
        free_parser();
        print_crash_and_exit("Could't reallocate memory for the parser!\n");
    }
    tree.nodes = newTree;
    tree.capacitity = size;
}

void init_parser(void) {
    resize_tree(initial_tree_size);
}

void free_parser(void) {
    free(tree.nodes);
    tree.nodes = NULL;
    tree.capacitity = 0;
    tree.size = 0;
}

int64_t new_node(const struct Token tok) {
    // Allocates more memory for the tree, if necessary
    if (tree.size >= tree.capacitity) {
        resize_tree(2 * tree.capacitity);
    }
    tree.nodes[tree.size] = (struct Token_Node){
        .tok = tok,
        .left_idx = -1,
        .right_idx = -1,
    };
    return tree.size++;
}

static inline struct Token_Node *get_node_ptr(int64_t idx) {
    return &tree.nodes[idx];
}

static inline struct Token get_tok(int64_t idx) {
    return tree.nodes[idx].tok;
}

static inline int64_t get_right_idx(int64_t idx) {
    return tree.nodes[idx].right_idx;
}

static inline int64_t get_left_idx(int64_t idx) {
    return tree.nodes[idx].left_idx;
}

static inline int64_t is_valid_idx(int64_t idx) {
    return ((0 <= idx) && (idx < tree.size));
}

static inline int64_t is_invalid_idx(int64_t idx) {
    return ((idx < 0) || (idx >= tree.size));
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

static inline bool check_parser_right_error(const int64_t previous_idx, const int64_t current_idx) {
    if (is_invalid_idx(previous_idx) || is_invalid_idx(current_idx)) {
        print_crash_and_exit("Invalid call to function \"check_parser_right_error()\"!\n");
        return true;
    }
    struct Token previous_tok = get_tok(previous_idx);
    struct Token current_tok = get_tok(current_idx);
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

bool insert_node_right(int64_t *const head_idx, const int64_t node_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = *head_idx;
        while (is_valid_idx(get_right_idx(previous_idx))) {
            previous_idx = get_right_idx(previous_idx);
        }
        if (check_parser_right_error(previous_idx, node_idx)) {
            return true;
        }
        get_node_ptr(previous_idx)->right_idx = node_idx;
    }
    return false;
}

bool insert_new_op(int64_t *const head_idx, const int64_t node_idx, const int64_t max_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = -1;
        {  // Search for the correct place to insert the node
            int64_t next_idx = *head_idx;
            const int precedence = get_op_precedence(get_tok(node_idx).op);
            while (is_valid_idx(next_idx) && (get_tok(next_idx).type == TOK_OPERATOR)) {
                if ((next_idx == max_idx) || (get_op_precedence(get_tok(next_idx).op) <= precedence)) {
                    break;
                }
                previous_idx = next_idx;
                next_idx = get_right_idx(next_idx);
            }
        }
        // Insert new node
        if (is_invalid_idx(previous_idx)) {
            get_node_ptr(node_idx)->left_idx = *head_idx;
            *head_idx = node_idx;
        } else {
            get_node_ptr(node_idx)->left_idx = get_right_idx(previous_idx);
            get_node_ptr(previous_idx)->right_idx = node_idx;
        }
    }
    return false;
}

int64_t parse_parentheses(size_t *const tk_idx) {
    if (tk_idx == NULL) {
        print_crash_and_exit("Invalid call to function \"parse_parentheses()\"!\n");
        return -1;
    }
    int64_t head_idx = -1;
    int64_t last_parentheses_idx = -1;
    while ((*tk_idx < tokens.size) && (tokens.list[*tk_idx].op != ')')) {
        const struct Token current_token = tokens.list[*tk_idx];
        if (current_token.op == '(') {
            (*tk_idx)++;
            last_parentheses_idx = parse_parentheses(tk_idx);
            if (insert_node_right(&head_idx, last_parentheses_idx)) {
                return -1;
            }
            if (*tk_idx >= tokens.size) {
                print_column(current_token.column);
                print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                return -1;
            }
        } else if (current_token.type == TOK_OPERATOR) {
            int64_t node_idx = new_node(current_token);
            if (insert_new_op(&head_idx, node_idx, last_parentheses_idx)) {
                return -1;
            }
        } else {
            int64_t node_idx = new_node(current_token);
            if (insert_node_right(&head_idx, node_idx)) {
                return -1;
            }
        }
        (*tk_idx)++;
    }
    return head_idx;
}

int64_t parser(void) {
    if (tokens.size == 0) {
        return -1;
    }
    size_t tk_idx = 0;
    tree.size = 0;
    int64_t head_idx = parse_parentheses(&tk_idx);
    if ((tk_idx < tokens.size) && (tokens.list[tk_idx].op == ')')) {
        print_column(tokens.list[tk_idx].column);
        print_error("Mismatched delimiters! Unexpected closing parentheses!\n");
        return -1;
    }
    return head_idx;
}

double evaluate(const int64_t node_idx, enum Evaluation_Status *const status) {
    if (is_invalid_idx(node_idx)) {
        return NAN;
    }
    if ((status == NULL) || (*status == Eval_Error)) {
        return NAN;
    }
    switch (get_tok(node_idx).type) {
        case TOK_OPERATOR:
            switch (get_tok(node_idx).op) {
                case '+':
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return (evaluate(get_left_idx(node_idx), status) + evaluate(get_right_idx(node_idx), status));
                case '-':
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return (evaluate(get_left_idx(node_idx), status) - evaluate(get_right_idx(node_idx), status));
                case '*':
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return (evaluate(get_left_idx(node_idx), status) * evaluate(get_right_idx(node_idx), status));
                case '/':
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return (evaluate(get_left_idx(node_idx), status) / evaluate(get_right_idx(node_idx), status));
                case '^':
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return pow(evaluate(get_left_idx(node_idx), status), evaluate(get_right_idx(node_idx), status));
                case '=': {
                    if (is_invalid_idx(get_left_idx(node_idx)) || is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    if (get_tok(get_left_idx(node_idx)).type == TOK_FUNCTION) {
                        print_column(get_tok(get_left_idx(node_idx)).column);
                        print_error("Cannot create a variable named \"%s\", because already exists a function with this name!\n", functions[get_tok(get_left_idx(node_idx)).function_index].name);
                        *status = Eval_Error;
                        return NAN;
                    }
                    if (get_tok(get_left_idx(node_idx)).type != TOK_VARIABLE) {
                        print_column(get_tok(get_left_idx(node_idx)).column);
                        print_error("Expected variable name for atribution!\n");
                        *status = Eval_Error;
                        return NAN;
                    }
                    const double result = evaluate(get_right_idx(node_idx), status);
                    if (*status != Eval_Error) {
                        return assign_variable(get_tok(get_left_idx(node_idx)).name.string, get_tok(get_left_idx(node_idx)).name.length, result);
                    } else {
                        return NAN;
                    }
                }
                default:
                    print_column(get_tok(node_idx).column);
                    print_error("Invalid binary operator at evaluation phase: %c\n", get_tok(node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (get_tok(node_idx).op) {
                case '-':
                    if (is_invalid_idx(get_right_idx(node_idx))) {
                        print_column(get_tok(node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(node_idx).op);
                        return NAN;
                    }
                    return (-evaluate(get_right_idx(node_idx), status));
                default:
                    print_column(get_tok(node_idx).column);
                    print_error("Invalid unary operator at evaluation phase: %c\n", get_tok(node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_NUMBER:
            return get_tok(node_idx).number;
        case TOK_FUNCTION: {
            const struct Function function = functions[get_tok(node_idx).function_index];
            if (function.fn == NULL) {
                print_column(get_tok(node_idx).column);
                print_error("The function \"%s\" was not properly initialized!\n", function.name);
                *status = Eval_Error;
                return NAN;
            }
            if ((function.arity >= 1) && is_invalid_idx(get_right_idx(node_idx))) {
                print_column(get_tok(node_idx).column);
                print_warning("Did you forget to pass a argument to the function \"%s\"?\n", function.name);
            }
            if ((!function.return_value) && (*status != Eval_Error)) {
                *status = Eval_Dont_Print;
            }
            return function.fn(evaluate(get_right_idx(node_idx), status));
        }
        case TOK_VARIABLE: {
            int index;
            if (search_variable(get_tok(node_idx).name.string, get_tok(node_idx).name.length, &index) != 0) {
                print_column(get_tok(node_idx).column);
                print_error("Unrecognized name: \"%.*s\"!\n", get_tok(node_idx).name.length, get_tok(node_idx).name.string);
                *status = Eval_Error;
                return NAN;
            }
            return get_variable(index);
        }
        case TOK_DELIMITER:
            print_column(get_tok(node_idx).column);
            print_error("Unexpected delimiter at evaluation phase: %c\n", get_tok(node_idx).op);
            *status = Eval_Error;
            return NAN;
        default:
            print_column(get_tok(node_idx).column);
            print_error("Invalid token at evaluation phase: %c\n", get_tok(node_idx).op);
            *status = Eval_Error;
            return NAN;
    }
}

void print_node(const int64_t node_idx, const unsigned int level) {
    if ((node_idx < 0) || (node_idx > tree.size)) {
        return;
    }
    print_token(get_tok(node_idx));
    if (get_left_idx(node_idx) >= 0) {
        printf("%*sLEFT:  ", level * 2, "");
        print_node(get_left_idx(node_idx), level + 1);
    }
    if (get_right_idx(node_idx) >= 0) {
        printf("%*sRIGHT: ", level * 2, "");
        print_node(get_right_idx(node_idx), level + 1);
    }
}

void print_tree(const int64_t head_idx) {
    printf("Abstract syntax tree (AST) generated by the parser:\n");
    printf("HEAD:  ");
    print_node(head_idx, 0);
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