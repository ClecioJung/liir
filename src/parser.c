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
        if (!is_operator(previous->tok) && !is_operator(node->tok)) {
            print_error("Invalid expression!\n");
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
    return true;
}

struct Token_Node *parse_parentheses(const struct TokenList *const tokens,
                                     size_t *const tkIndex) {
    if (tkIndex == NULL) {
        return NULL;
    }
    struct Token_Node *head = NULL;
    while ((*tkIndex < tokens->last) && (tokens->list[*tkIndex].op != ')')) {
        struct Token_Node *node = NULL;
        bool ok = true;
        if (tokens->list[*tkIndex].op == '(') {
            (*tkIndex)++;
            node = parse_parentheses(tokens, tkIndex);
            ok = insert_node_right(&head, node);
            if (*tkIndex >= tokens->last) {
                print_error("Too many opening parentheses!\n");
                ok = false;
            }
        } else if (tokens->list[*tkIndex].type == TOK_OPERATOR) {
            node = new_node(tokens->list[*tkIndex]);
            ok = insert_new_op(&head, node);
        } else {
            node = new_node(tokens->list[*tkIndex]);
            ok = insert_node_right(&head, node);
        }
        if (!ok) {
            free_tree(head);
            free(node);
            head = NULL;
            break;
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
    if ((tkIndex < tokens->last) && (tokens->list[tkIndex].op == ')')) {
        print_error("Too many closing parentheses!\n");
        free_tree(head);
        head = NULL;
    }
    return head;
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