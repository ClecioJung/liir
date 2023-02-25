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

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data-structures/sized_string.h"
#include "functions.h"
#include "input_stream.h"
#include "lex.h"
#include "parser.h"
#include "printing.h"
#include "variables.h"

enum Actions {
    ACTION_EXIT = 1,
    ACTION_PRINT_TOKENS = 2,
    ACTION_PRINT_TREE = 4,
    ACTION_PRINT_GRAPH = 8,
    ACTION_PRINT_VARIABLES = 16,
    ACTION_PRINT_LINES = 32,
};

typedef void (*Arg_Function)(const char *const parameter);

// Table used to concentrate all the information related to the comand line arguments
struct Arg_Cmd {
    const char *const cmd;
    const Arg_Function function;
    const bool accept_parameter;
    const char *usage;
};

static void arguments_usage(const char *const parameter);
static void set_print_tokens(const char *const parameter);
static void set_print_tree(const char *const parameter);
static void set_print_graph(const char *const parameter);
static void set_print_variables(const char *const parameter);
static void set_print_functions(const char *const parameter);
static void set_print_lines(const char *const parameter);
static void set_expression_to_evaluate(const char *const parameter);
static void set_file_name_to_load(const char *const parameter);

static const struct Arg_Cmd arg_list[] = {
    {"--help", arguments_usage, false, "Display this help message."},
    {"--token", set_print_tokens, false, "Display the list of tokens generated by lexical analysis."},
    {"--ast", set_print_tree, false, "Display the abstract syntax tree (AST) generated by the parser."},
    {"--graph", set_print_graph, false, "Display a graph representation of the abstract syntax tree (AST)."},
    {"--variable", set_print_variables, false, "Display the variables list at each step."},
    {"--function", set_print_functions, false, "Display the built-in functions."},
    {"--input", set_print_lines, false, "Display the previous typed lines at each step."},
    {"--expr", set_expression_to_evaluate, true, "Evaluate a single expression passed by command line."},
    {"--load", set_file_name_to_load, true, "Load the variables from the specified file."},
};
static const int arg_num = (sizeof(arg_list) / sizeof(arg_list[0]));
static const char *software = NULL;
static enum Actions actions = 0;
static struct String command_line_expression = {0};
static const char *file_name_to_load = NULL;

unsigned int max_uint(const unsigned int a, const unsigned int b) {
    return ((a > b) ? a : b);
}

static void arguments_usage(const char *const parameter) {
    (void)parameter;
    unsigned int cmd_max_length = 0;
    printf("[Usage] %s [Options]\n", software);
    for (int arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        cmd_max_length = max_uint(cmd_max_length, (unsigned int)strlen(arg_list[arg_idx].cmd));
    }
    printf("[Options]:\n");
    for (int arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        printf("\t%-*s or -%c: %s\n", cmd_max_length, arg_list[arg_idx].cmd, arg_list[arg_idx].cmd[2], arg_list[arg_idx].usage);
    }
    actions |= ACTION_EXIT;
}

static void set_print_tokens(const char *const parameter) {
    (void)parameter;
    actions |= ACTION_PRINT_TOKENS;
}

static void set_print_tree(const char *const parameter) {
    (void)parameter;
    actions |= ACTION_PRINT_TREE;
}

static void set_print_graph(const char *const parameter) {
    (void)parameter;
    actions |= ACTION_PRINT_GRAPH;
}

static void set_print_variables(const char *const parameter) {
    (void)parameter;
    actions |= ACTION_PRINT_VARIABLES;
}

static void set_print_functions(const char *const parameter) {
    (void)parameter;
    print_functions();
    actions |= ACTION_EXIT;
}

static void set_print_lines(const char *const parameter) {
    (void)parameter;
    actions |= ACTION_PRINT_LINES;
}

static void set_expression_to_evaluate(const char *const parameter) {
    if (parameter == NULL) {
        actions |= ACTION_EXIT;
    } else {
        command_line_expression = (struct String){
            .data = (char *)parameter,
            .length = (String_Length)strlen(parameter),
        };
    }
}

static void set_file_name_to_load(const char *const parameter) {
    if (parameter != NULL) {
        file_name_to_load = parameter;
    }
}

static inline int find_argument(const char *const arg) {
    const size_t alias_length = 2;
    const size_t length = strlen(arg);
    for (int arg_idx = 0; arg_idx < arg_num; arg_idx++) {
        const size_t cmd_length = strlen(arg_list[arg_idx].cmd);
        if ((length == cmd_length && !strcmp(arg, arg_list[arg_idx].cmd))
            || (length == alias_length && !strncmp(arg, arg_list[arg_idx].cmd + 1, alias_length))) {
            return arg_idx;
        }
    }
    // No argument found
    return -1;
}

// This function returns true if found an error
static bool parse_arguments(const int argc, const char *const argv[]) {
    software = argv[0];
    for (int i = 1; i < argc; i++) {
        const int arg_idx = find_argument(argv[i]);
        if (arg_idx < 0) {
            print_error("Unrecognized command line argument: %s\n", argv[i]);
            arguments_usage(NULL);
            return true;
        }
        if (arg_list[arg_idx].accept_parameter) {
            // argv is a list of strings with (argc+1) elements (the last one is a null pointer)
            // Beacause of this, we can always access the (i+1)-th element of argv in the line below
            // without accessing any invalid region of memory
            arg_list[arg_idx].function(argv[++i]);
        } else {
            arg_list[arg_idx].function(NULL);
        }
    }
    return false;
}

double fn_exit(struct Variables *const vars, const struct Fn_Arg first_arg, const struct Fn_Arg second_arg) {
    (void)vars;
    (void)first_arg;
    (void)second_arg;
    actions |= ACTION_EXIT;
    return NAN;
}

void interpret(struct Parser *const parser, const struct String line) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (!lex(parser->lexer, line)) {
        // If didn't found an error while executing the lexer
        int64_t head_idx = parse(parser);
        if (head_idx >= 0) {
            enum Evaluation_Status status = Eval_OK;
            const double result = evaluate(parser, head_idx, &status);
            if (status == Eval_OK) {
                printf("%lg\n", result);
            }
        }
        printf("\n");
        if (actions & ACTION_PRINT_TOKENS) {
            print_tokens(parser->lexer);
        }
        if (actions & ACTION_PRINT_TREE) {
            print_tree(parser, head_idx);
        }
        if (actions & ACTION_PRINT_GRAPH) {
            print_graph(parser, head_idx);
        }
        if (actions & ACTION_PRINT_VARIABLES) {
            print_variables(parser->vars);
        }
    }
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

int main(const int argc, const char *const argv[]) {
    if (parse_arguments(argc, argv)) {
        return EXIT_FAILURE;
    }
    if ((actions & ACTION_EXIT) != 0) {
        return EXIT_SUCCESS;
    }
    initialize_input_stream();
    struct Lexer lexer = create_lex(64);
    struct Variables vars = create_variables(64);
    struct Parser parser = create_parser(&lexer, &vars, 1024);
    if (file_name_to_load != NULL) {
        printf("Attempting to load variables from file \"%s\"\n", file_name_to_load);
        load_variables_from_file(&vars, create_string((char *)file_name_to_load));
        putchar('\n');
    }
    if (command_line_expression.length > 0) {
        // If an expression was passed through the command line, then evaluate it and exit
        printf("> %.*s\n", command_line_expression.length, command_line_expression.data);
        interpret(&parser, command_line_expression);
    } else {
        while ((actions & ACTION_EXIT) == 0) {
            const struct String line = get_line_from_input();
            interpret(&parser, line);
            if (actions & ACTION_PRINT_LINES) {
                print_previous_lines();
            }
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
