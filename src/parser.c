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

#include "lex.h"
#include "print_errors.h"

void free_tree(const struct Token_Node *const node) {
    if (node == NULL) {
        return;
    }
    free_tree(node->left);
    free_tree(node->right);
    free((void *)node);
}

static size_t get_op_precedence(const char op) {
    const char precedence[] = {
        '^', '/', '*', '-', '+',
    };
    const size_t num_ops = sizeof(precedence) / sizeof(precedence[0]);
    for (size_t index = 0; index < num_ops; index++) {
        if (op == precedence[index]) {
            return index;
        }
    }
    return num_ops;
}

static struct Token_Node *new_node(const struct Token tok) {
    struct Token_Node *const node =
        (struct Token_Node *)malloc(sizeof(struct Token_Node));
    if (node != NULL) {
        node->tok = tok;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

static inline void insert_node_right(struct Token_Node **const head,
                                     struct Token_Node *const node) {
    if (*head == NULL) {
        *head = node;
    } else {
        struct Token_Node *previous = *head;
        while (previous->right != NULL) {
            previous = previous->right;
        }
        previous->right = node;
    }
}

static inline void insert_new_op(struct Token_Node **head,
                                 struct Token_Node *const node) {
    if (*head == NULL) {
        *head = node;
        return;
    }
    struct Token_Node *previous = NULL;
    {  // Search for the correct place to insert the node
        struct Token_Node *next = *head;
        const size_t precedence = get_op_precedence(node->tok.op);
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

struct Token_Node *parse_parentheses(const struct TokenList *const tokens,
                                     size_t *const tkIndex) {
    if (tkIndex == NULL) {
        return NULL;
    }
    struct Token_Node *head = NULL;
    while ((*tkIndex < tokens->last) && (tokens->list[*tkIndex].op != ')')) {
        if (tokens->list[*tkIndex].op == '(') {
            (*tkIndex)++;
            struct Token_Node *const node = parse_parentheses(tokens, tkIndex);
            insert_node_right(&head, node);
            if (*tkIndex >= tokens->last) {
                print_error("Too many opening parentheses!\n");
                free_tree(head);
                head = NULL;
                break;
            }
        } else if (tokens->list[*tkIndex].type == TOK_OPERATOR) {
            struct Token_Node *const node = new_node(tokens->list[*tkIndex]);
            insert_new_op(&head, node);
        } else {
            struct Token_Node *const node = new_node(tokens->list[*tkIndex]);
            insert_node_right(&head, node);
        }
        (*tkIndex)++;
    }
    return head;
}

struct Token_Node *parser(const struct TokenList *const tokens) {
    if (tokens->last == 0) {
        return NULL;
    }
    size_t tkIndex = 0;
    struct Token_Node *head = parse_parentheses(tokens, &tkIndex);
    if (tkIndex < tokens->last) {
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
        case TOK_NAME:
            print_error(
                "Variables and functions are not implemented yet: %.*s\n",
                node->tok.name.length, node->tok.name.string);
            return NAN;
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
    fputs("HEAD:  ", stdout);
    print_node(head, 0);
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