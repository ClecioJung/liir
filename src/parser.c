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
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "lex.h"
#include "print_errors.h"
#include "variables.h"

void free_tree(const struct Token_Node *const node) {
    if (node == NULL) {
        return;
    }
    free_tree(node->left);
    free_tree(node->right);
    free((void *)node);
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

struct Token_Node *new_node(const struct Token tok) {
    struct Token_Node *const node = (struct Token_Node *)malloc(sizeof(struct Token_Node));
    if (node != NULL) {
        node->tok = tok;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

static inline bool check_parser_right_error(const struct Token_Node *const previous, const struct Token_Node *const current_node) {
    if ((previous->tok.type == TOK_NUMBER) || (previous->tok.type == TOK_VARIABLE)) {
        if (current_node->tok.type == TOK_NUMBER) {
            print_column(current_node->tok.column);
            print_error("Unexpected number! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
        if (current_node->tok.type == TOK_VARIABLE) {
            print_column(current_node->tok.column);
            print_error("Unexpected variable! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
        if (current_node->tok.type == TOK_FUNCTION) {
            print_column(current_node->tok.column);
            print_error("Unexpected function! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
    }
    if (previous->tok.type == TOK_FUNCTION) {
        if (functions[previous->tok.function_index].arity == 0) {
            if ((current_node->tok.type == TOK_NUMBER) || (current_node->tok.type == TOK_VARIABLE)) {
                print_column(current_node->tok.column);
                print_warning("The function \"%s\" accepts no argument!\n", functions[previous->tok.function_index].name);
            }
        }
        if (current_node->tok.type == TOK_OPERATOR) {
            if (!functions[previous->tok.function_index].return_value) {
                print_column(previous->tok.column);
                print_error("The function \"%s\" returns no value, so it can't be used in a expression!\n", functions[previous->tok.function_index].name);
                return true;
            }
        }
    }
    if (previous->tok.type == TOK_OPERATOR) {
        if ((current_node->tok.type == TOK_FUNCTION) &&
            (!functions[current_node->tok.function_index].return_value)) {
            print_column(current_node->tok.column);
            print_error("The function \"%s\" returns no value, so it can't be used in a expression!\n", functions[current_node->tok.function_index].name);
            return true;
        }
    }
    return false;
}

bool insert_node_right(struct Token_Node **const head, struct Token_Node *const node) {
    if (*head == NULL) {
        *head = node;
    } else {
        struct Token_Node *previous = *head;
        while (previous->right != NULL) {
            previous = previous->right;
        }
        if (check_parser_right_error(previous, node)) {
            return false;
        }
        previous->right = node;
    }
    return true;
}

bool insert_new_op(struct Token_Node **head, struct Token_Node *const node) {
    if (*head == NULL) {
        *head = node;
    } else {
        struct Token_Node *previous = NULL;
        {  // Search for the correct place to insert the node
            struct Token_Node *next = *head;
            const int precedence = get_op_precedence(node->tok.op);
            while ((next != NULL) && (next->tok.type == TOK_OPERATOR)) {
                if (get_op_precedence(next->tok.op) <= precedence) {
                    break;
                }
                // Division has higher left precedence
                if (next->tok.op == '/') {
                    break;
                }
                previous = next;
                next = next->right;
            }
        }
        // Insert new node
        if (previous == NULL) {
            node->left = *head;
            *head = node;
        } else {
            node->left = previous->right;
            previous->right = node;
        }
    }
    return true;
}

struct Token_Node *parse_parentheses(size_t *const tk_idx) {
    if (tk_idx == NULL) {
        return NULL;
    }
    struct Token_Node *head = NULL;
    while ((*tk_idx < tokens.size) && (tokens.list[*tk_idx].op != ')')) {
        bool ok = true;
        const struct Token current_token = tokens.list[*tk_idx];
        if (current_token.op == '(') {
            (*tk_idx)++;
            struct Token_Node *const node = parse_parentheses(tk_idx);
            ok = insert_node_right(&head, node);
            if (!ok) {
                free(node);
            }
            if (*tk_idx >= tokens.size) {
                print_column(current_token.column);
                print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                ok = false;
            }
        } else if (current_token.type == TOK_OPERATOR) {
            struct Token_Node *const node = new_node(current_token);
            ok = insert_new_op(&head, node);
            if (!ok) {
                free(node);
            }
        } else {
            struct Token_Node *const node = new_node(current_token);
            ok = insert_node_right(&head, node);
            if (!ok) {
                free(node);
            }
        }
        if (!ok) {
            free_tree(head);
            head = NULL;
            break;
        }
        (*tk_idx)++;
    }
    return head;
}

struct Token_Node *parser(void) {
    if (tokens.size == 0) {
        return NULL;
    }
    size_t tk_idx = 0;
    struct Token_Node *head = parse_parentheses(&tk_idx);
    if ((tk_idx < tokens.size) && (tokens.list[tk_idx].op == ')')) {
        print_column(tokens.list[tk_idx].column);
        print_error("Mismatched delimiters! Unexpected closing parentheses!\n");
        free_tree(head);
        head = NULL;
    }
    return head;
}

double evaluate(const struct Token_Node *const node, enum Evaluation_Status *const status) {
    if (node == NULL) {
        return NAN;
    }
    if ((status == NULL) || (*status == Eval_Error)) {
        return NAN;
    }
    switch (node->tok.type) {
        case TOK_OPERATOR:
            switch (node->tok.op) {
                case '+':
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return (evaluate(node->left, status) + evaluate(node->right, status));
                case '-':
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return (evaluate(node->left, status) - evaluate(node->right, status));
                case '*':
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return (evaluate(node->left, status) * evaluate(node->right, status));
                case '/':
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return (evaluate(node->left, status) / evaluate(node->right, status));
                case '^':
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return pow(evaluate(node->left, status), evaluate(node->right, status));
                case '=': {
                    if ((node->left == NULL) || (node->right == NULL)) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    if (node->left->tok.type == TOK_FUNCTION) {
                        print_column(node->left->tok.column);
                        print_error("Cannot create a variable named \"%s\", because already exists a function with this name!\n", functions[node->left->tok.function_index].name);
                        *status = Eval_Error;
                        return NAN;
                    }
                    if (node->left->tok.type != TOK_VARIABLE) {
                        print_column(node->left->tok.column);
                        print_error("Expected variable name for atribution!\n");
                        *status = Eval_Error;
                        return NAN;
                    }
                    const double result = evaluate(node->right, status);
                    if (*status != Eval_Error) {
                        return assign_variable(node->left->tok.name.string, node->left->tok.name.length, result);
                    } else {
                        return NAN;
                    }
                }
                default:
                    print_column(node->tok.column);
                    print_error("Invalid binary operator at evaluation phase: %c\n", node->tok.op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (node->tok.op) {
                case '-':
                    if (node->right == NULL) {
                        print_column(node->tok.column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", node->tok.op);
                        return NAN;
                    }
                    return (-evaluate(node->right, status));
                default:
                    print_column(node->tok.column);
                    print_error("Invalid unary operator at evaluation phase: %c\n", node->tok.op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_NUMBER:
            return node->tok.number;
        case TOK_FUNCTION: {
            const struct Function function = functions[node->tok.function_index];
            if (function.fn == NULL) {
                print_column(node->tok.column);
                print_error("The function \"%s\" was not properly initialized!\n", function.name);
                *status = Eval_Error;
                return NAN;
            }
            if ((function.arity >= 1) && (node->right == NULL)) {
                print_column(node->tok.column);
                print_warning("Did you forget to pass a argument to the function \"%s\"?\n", function.name);
            }
            if ((!function.return_value) && (*status != Eval_Error)) {
                *status = Eval_Dont_Print;
            }
            return function.fn(evaluate(node->right, status));
        }
        case TOK_VARIABLE: {
            int index;
            if (search_variable(node->tok.name.string, node->tok.name.length, &index) != 0) {
                print_column(node->tok.column);
                print_error("Unrecognized name: \"%.*s\"!\n", node->tok.name.length, node->tok.name.string);
                *status = Eval_Error;
                return NAN;
            }
            return get_variable(index);
        }
        case TOK_DELIMITER:
            print_column(node->tok.column);
            print_error("Unexpected delimiter at evaluation phase: %c\n", node->tok.op);
            *status = Eval_Error;
            return NAN;
        default:
            print_column(node->tok.column);
            print_error("Invalid token at evaluation phase: %c\n", node->tok.op);
            *status = Eval_Error;
            return NAN;
    }
}

void print_node(const struct Token_Node *const node, const unsigned int level) {
    if (node == NULL) {
        return;
    }
    print_token(node->tok);
    if (node->left) {
        printf("%*sLEFT:  ", level * 2, "");
        print_node(node->left, level + 1);
    }
    if (node->right) {
        printf("%*sRIGHT: ", level * 2, "");
        print_node(node->right, level + 1);
    }
}

void print_tree(const struct Token_Node *const head) {
    printf("Abstract syntax tree (AST) generated by the parser:\n");
    printf("HEAD:  ");
    print_node(head, 0);
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