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

#define _GNU_SOURCE
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arguments.h"
#include "functions.h"
#include "lex.h"
#include "parser.h"
#include "print_errors.h"
#include "variables.h"

#define ACTION_EXIT 1
#define ACTION_PRINT_TOKENS 2
#define ACTION_PRINT_TREE 4
#define ACTION_PRINT_VARIABLES 8

static int actions = 0;

void exit_repl(void) {
    actions |= ACTION_EXIT;
}

void repl(const char *const line) {
    struct TokenList *tokens = lex(line);
    if (tokens == NULL) {
        return;
    }
    struct Token_Node *head = parser(tokens);
    if (head == NULL) {
        return;
    }
    enum Evaluation_Status status = Eval_OK;
    const double result = evaluate(head, &status);
    if (status == Eval_OK) {
        printf("%lg\n", result);
    }
    if (actions & ACTION_PRINT_TOKENS) {
        print_tokens(tokens);
    }
    if (actions & ACTION_PRINT_TREE) {
        print_tree(head);
    }
    if (actions & ACTION_PRINT_VARIABLES) {
        print_variables();
    }
    free_tree(head);
}

int print_usage(const char *const software) {
    printf("[Usage] %s [Options]\n", software);
    actions |= ACTION_EXIT;
    return EXIT_SUCCESS;
}

int unknown_argument(const char *const arg) {
    print_error("Unrecognized command line argument: %s\n", arg);
    return EXIT_FAILURE;
}

int set_print_tokens(const char *const arg) {
    (void)arg;
    actions |= ACTION_PRINT_TOKENS;
    return EXIT_SUCCESS;
}

int set_print_tree(const char *const arg) {
    (void)arg;
    actions |= ACTION_PRINT_TREE;
    return EXIT_SUCCESS;
}

int set_print_variables(const char *const arg) {
    (void)arg;
    actions |= ACTION_PRINT_VARIABLES;
    return EXIT_SUCCESS;
}

int set_print_functions(const char *const arg) {
    (void)arg;
    print_functions();
    actions |= ACTION_EXIT;
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

int main(const int argc, const char *const argv[]) {
    initArguments(print_usage, unknown_argument);
    addArgument("--token", "-t", set_print_tokens,
                "Display the list of tokens generated by lexical analysis.");
    addArgument(
        "--ast", "-a", set_print_tree,
        "Display the abstract syntax tree (AST) generated by the parser.");
    addArgument("--variable", "-v", set_print_variables,
                "Display the variables list at each step.");
    addArgument("--function", "-f", set_print_functions,
                "Display the built-in functions.");
    if (parseArguments(argc, argv) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    char *line = NULL;
    size_t len = 0;
    init_lex();
    init_variables();
    while ((actions & ACTION_EXIT) == 0) {
        printf("> ");
        if (getline(&line, &len, stdin) == -1) {
            print_crash_and_exit("Could't get line from stdin!\n");
        }
        repl(line);
    }
    free_lex();
    free_variables();
    free(line);
    return EXIT_SUCCESS;
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