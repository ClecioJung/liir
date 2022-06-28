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

#include "interpreter.h"

#include <math.h>
#include <string.h>

#include "parser.h"
#include "print_errors.h"

#define INITIAL_VARIABLE_LIST_SIZE 8

static struct VariableList *variables = NULL;

void resize_variable_list(const size_t size) {
    struct Variable *newList = (struct Variable *)realloc(
        variables->list, size * sizeof(struct Variable));
    if (newList == NULL) {
        free_variables();
        print_crash_and_exit("Could't reallocate memory for the variables!\n");
    }
    variables->list = newList;
    variables->size = size;
}

void init_variables(void) {
    variables = (struct VariableList *)malloc(sizeof(struct VariableList));
    if (variables == NULL) {
        print_crash_and_exit("Could't allocate memory for the variables!\n");
    }
    variables->list = NULL;
    variables->size = 0;
    variables->last = 0;
    resize_variable_list(INITIAL_VARIABLE_LIST_SIZE);
}

void free_variables(void) {
    if (variables != NULL) {
        for (size_t i = 0; i < variables->last; i++) {
            free(variables->list[i].name);
            variables->list[i].name = NULL;
        }
        free(variables->list);
        free(variables);
        variables = NULL;
    }
}

size_t search_variable(const char *const name, const unsigned int length) {
    // Sequential search in variables list
    for (size_t i = 0; i < variables->last; i++) {
        if (!strncmp(variables->list[i].name, name, length)) {
            return i;
        }
    }
    return variables->last;
}

void new_variable(char *const name, const unsigned int length,
                  const double value) {
    // Allocates more memory if necessary
    if (variables->last >= variables->size) {
        resize_variable_list(2 * variables->size);
    }
    variables->list[variables->last].name = malloc((length + 1) * sizeof(char));
    if (variables->list[variables->last].name == NULL) {
        print_crash_and_exit("Could't allocate memory for the variable %.*s!\n",
                             length, name);
    }
    strncpy(variables->list[variables->last].name, name, length);
    variables->list[variables->last].name[length] = '\0';
    variables->list[variables->last].value = value;
    variables->last++;
}

double assign_variable(char *const name, const unsigned int length,
                       const double value) {
    const size_t index = search_variable(name, length);
    if (index >= variables->last) {
        new_variable(name, length, value);
    } else {
        variables->list[index].value = value;
    }
    return value;
}

double get_variable(const char *name, const unsigned int length) {
    const size_t index = search_variable(name, length);
    if (index >= variables->last) {
        return NAN;
    }
    return variables->list[index].value;
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
                    if (node->left->tok.type != TOK_NAME) {
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
        case TOK_NAME:
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