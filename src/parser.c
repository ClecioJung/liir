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
        '^', '/', '*', '-', '+',
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
    struct Token_Node *const node =
        (struct Token_Node *)malloc(sizeof(struct Token_Node));
    if (node != NULL) {
        node->tok = tok;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

bool insert_node_right(struct Token_Node **const head,
                       struct Token_Node *const node) {
    if (*head == NULL) {
        *head = node;
    } else {
        struct Token_Node *previous = *head;
        while (previous->right != NULL) {
            previous = previous->right;
        }
        if ((previous->tok.type == TOK_NUMBER) ||
            (previous->tok.type == TOK_VARIABLE)) {
            if ((node->tok.type == TOK_NUMBER) ||
                (node->tok.type == TOK_VARIABLE) ||
                (node->tok.type == TOK_FUNCTION)) {
                print_error("Invalid expression!\n");
                return false;
            }
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

struct Token_Node *parse_parentheses(const struct TokenList *const tokens,
                                     int *const tkIndex) {
    if (tkIndex == NULL) {
        return NULL;
    }
    struct Token_Node *head = NULL;
    while ((*tkIndex < tokens->size) && (tokens->list[*tkIndex].op != ')')) {
        bool ok = true;
        if (tokens->list[*tkIndex].op == '(') {
            (*tkIndex)++;
            struct Token_Node *const node = parse_parentheses(tokens, tkIndex);
            ok = insert_node_right(&head, node);
            if (!ok) {
                free(node);
            }
            if (*tkIndex >= tokens->size) {
                print_error("Too many opening parentheses!\n");
                ok = false;
            }
        } else if (tokens->list[*tkIndex].type == TOK_OPERATOR) {
            struct Token_Node *const node = new_node(tokens->list[*tkIndex]);
            ok = insert_new_op(&head, node);
            if (!ok) {
                free(node);
            }
        } else {
            struct Token_Node *const node = new_node(tokens->list[*tkIndex]);
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
        (*tkIndex)++;
    }
    return head;
}

struct Token_Node *parser(const struct TokenList *const tokens) {
    if (tokens->size == 0) {
        return NULL;
    }
    int tkIndex = 0;
    struct Token_Node *head = parse_parentheses(tokens, &tkIndex);
    if ((tkIndex < tokens->size) && (tokens->list[tkIndex].op == ')')) {
        print_error("Too many closing parentheses!\n");
        free_tree(head);
        head = NULL;
    }
    return head;
}

double evaluate(const struct Token_Node *const node) {
    if (node == NULL) {
        return NAN;
    }
    switch (node->tok.type) {
        case TOK_OPERATOR:
            switch (node->tok.op) {
                case '+':
                    return (evaluate(node->left) + evaluate(node->right));
                case '-':
                    return (evaluate(node->left) - evaluate(node->right));
                case '*':
                    return (evaluate(node->left) * evaluate(node->right));
                case '/':
                    return (evaluate(node->left) / evaluate(node->right));
                case '^':
                    return pow(evaluate(node->left), evaluate(node->right));
                case '=': {
                    if (node->left->tok.type != TOK_VARIABLE) {
                        print_error("Expected variable name for atribution!\n");
                        return NAN;
                    }
                    return assign_variable(node->left->tok.name.string,
                                           node->left->tok.name.length,
                                           evaluate(node->right));
                }
                default:
                    print_error("Invalid operator at evaluation phase: %c\n",
                                node->tok.op);
                    return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (node->tok.op) {
                case '-':
                    return (-evaluate(node->right));
                default:
                    print_error(
                        "Invalid unary operator at evaluation phase: %c\n",
                        node->tok.op);
                    return NAN;
            }
        case TOK_NUMBER:
            return node->tok.number;
        case TOK_FUNCTION: {
            const struct Function function =
                functions[node->tok.function_index];
            if (function.fn == NULL) {
                return NAN;
            }
            switch (function.arity) {
                case 1:
                    return function.fn(evaluate(node->right));
                case 0:
                default:
                    return function.fn(NAN);
            }
        }
        case TOK_VARIABLE:
            return get_variable(node->tok.name.string, node->tok.name.length);
        case TOK_DELIMITER:
            print_error("Unexpected delimiter at evaluation phase: %c\n",
                        node->tok.op);
            return NAN;
        default:
            print_error("Invalid token at evaluation phase: %c\n",
                        node->tok.op);
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