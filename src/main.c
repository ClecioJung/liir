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

#include "functions.h"
#include "input_stream.h"
#include "lex.h"
#include "parser.h"
#include "print_errors.h"
#include "sized_string.h"
#include "variables.h"

enum Actions {
    ACTION_EXIT = 1,
    ACTION_PRINT_TOKENS = 2,
    ACTION_PRINT_TREE = 4,
    ACTION_PRINT_VARIABLES = 8,
    ACTION_PRINT_LINES = 16,
};

typedef void (*Arg_Function)(void);

// Table used to concentrate all the information related to the comand line arguments
struct Arg_Cmd {
    const char *cmd;
    Arg_Function function;
    const char *usage;
};

void arguments_usage(void);
void set_print_tokens(void);
void set_print_tree(void);
void set_print_variables(void);
void set_print_functions(void);
void set_print_lines(void);

static const struct Arg_Cmd arg_list[] = {
    {"--help", arguments_usage, "Display this help message."},
    {"--token", set_print_tokens, "Display the list of tokens generated by lexical analysis."},
    {"--ast", set_print_tree, "Display the abstract syntax tree (AST) generated by the parser."},
    {"--variable", set_print_variables, "Display the variables list at each step."},
    {"--function", set_print_functions, "Display the built-in functions."},
    {"--input", set_print_lines, "Display the previous typed lines at each step."},
};
static const size_t arg_num = (sizeof(arg_list) / sizeof(arg_list[0]));
static const char *software = NULL;
static enum Actions actions = 0;

unsigned int max_uint(const unsigned int a, const unsigned int b) {
    return ((a > b) ? a : b);
}

void arguments_usage(void) {
    unsigned int cmd_max_length = 0;
    printf("[Usage] %s [Options]\n", software);
    for (size_t arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        cmd_max_length = max_uint(cmd_max_length, strlen(arg_list[arg_idx].cmd));
    }
    printf("[Options]:\n");
    for (size_t arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        printf("\t%-*s or -%c: %s\n", cmd_max_length, arg_list[arg_idx].cmd, arg_list[arg_idx].cmd[2], arg_list[arg_idx].usage);
    }
    actions |= ACTION_EXIT;
}

void set_print_tokens(void) {
    actions |= ACTION_PRINT_TOKENS;
}

void set_print_tree(void) {
    actions |= ACTION_PRINT_TREE;
}

void set_print_variables(void) {
    actions |= ACTION_PRINT_VARIABLES;
}

void set_print_functions(void) {
    print_functions();
    actions |= ACTION_EXIT;
}

void set_print_lines(void) {
    actions |= ACTION_PRINT_LINES;
}

static inline int parse_argument(const char *const arg) {
    const size_t alias_length = 2;
    const size_t length = strlen(arg);
    for (size_t arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        const size_t cmd_length = strlen(arg_list[arg_idx].cmd);
        if ((length == cmd_length && !strcmp(arg, arg_list[arg_idx].cmd)) || (length == alias_length && !strncmp(arg, arg_list[arg_idx].cmd + 1, alias_length))) {
            arg_list[arg_idx].function();
            return EXIT_SUCCESS;
        }
    }
    // No argument found
    print_error("Unrecognized command line argument: %s\n", arg);
    arguments_usage();
    return EXIT_FAILURE;
}

int parse_arguments(const int argc, const char *const argv[]) {
    software = argv[0];
    for (int arg_idx = 1; arg_idx < argc; arg_idx++) {
        if (parse_argument(argv[arg_idx])) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

double fn_exit(struct Variables *const vars, const double arg) {
    (void)vars;
    (void)arg;
    actions |= ACTION_EXIT;
    return NAN;
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

int main(const int argc, const char *const argv[]) {
    if (parse_arguments(argc, argv) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    if ((actions & ACTION_EXIT) != 0) {
        return EXIT_SUCCESS;
    }
    struct Input_Stream input_stream = create_input_stream();
    struct Lexer lexer = create_lex(64);
    struct Variables vars = create_variables(64, 1024);
    struct Parser parser = create_parser(&lexer, &vars, 1024);
    while ((actions & ACTION_EXIT) == 0) {
        struct String line = get_line_from_input(&input_stream);
        if (lex(&lexer, line) != EXIT_FAILURE) {
            int64_t head_idx = parse(&parser);
            if (head_idx >= 0) {
                enum Evaluation_Status status = Eval_OK;
                const double result = evaluate(&parser, head_idx, &status);
                if (status == Eval_OK) {
                    printf("%lg\n", result);
                }
            }
            printf("\n");
            if (actions & ACTION_PRINT_TOKENS) {
                print_tokens(&lexer);
            }
            if ((head_idx >= 0) && (actions & ACTION_PRINT_TREE)) {
                print_tree(&parser, head_idx);
            }
            if (actions & ACTION_PRINT_VARIABLES) {
                print_variables(&vars);
            }
        }
        if (actions & ACTION_PRINT_LINES) {
            print_previous_lines(&input_stream);
        }
    }
    destroy_lex(&lexer);
    destroy_variables(&vars);
    destroy_parser(&parser);
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