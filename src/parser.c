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

#include "data-structures/dynamic_array.h"
#include "data-structures/sized_string.h"
#include "functions.h"
#include "lex.h"
#include "printing.h"
#include "variables.h"

#define INVALID_PARSER_INDEX ((size_t)-1)

struct Parser create_parser(struct Lexer *const lexer, struct Variables *const vars, const size_t initial_size) {
    if ((lexer == NULL) || (vars == NULL)) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    struct Parser parser = (struct Parser){
        .lexer = lexer,
        .vars = vars,
        .nodes = array_new(sizeof(struct Token_Node), initial_size),
    };
    if (parser.nodes == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the parser!\n");
    }
    return parser;
}

void destroy_parser(struct Parser *const parser) {
    if (parser != NULL) {
        array_del(parser->nodes);
    }
}

static size_t new_node(struct Parser *const parser, const struct Token tok) {
    struct Token_Node new_node = (struct Token_Node){
        .tok = tok,
        .left_idx = INVALID_PARSER_INDEX,
        .right_idx = INVALID_PARSER_INDEX,
    };
    array_push(parser->nodes, new_node);
    return(array_size(parser->nodes) - 1);
}

/*
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
*/

static int get_op_precedence(const char op) {
    const char precedence[] = { '^', '/', '*', '-', '+', };
    const int num_ops = sizeof(precedence) / sizeof(precedence[0]);
    for (int index = 0; index < num_ops; index++) {
        if (op == precedence[index]) {
            return index;
        }
    }
    return num_ops;
}

// This function returns true if found an error
static inline bool check_parser_right_error(struct Parser *const parser, const size_t previous_idx, const size_t current_idx) {
    struct Token previous_tok = parser->nodes[previous_idx].tok;
    struct Token current_tok = parser->nodes[current_idx].tok;
    if ((previous_tok.type == TOK_NUMBER) || (previous_tok.type == TOK_NAME)) {
        if (current_tok.type == TOK_NUMBER) {
            print_column(current_tok.column);
            print_error("Unexpected number! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
            return true;
        }
        if (current_tok.type == TOK_NAME) {
            print_column(current_tok.column);
            print_error("Unexpected name! Check for missing operator, missing or unbalanced delimiters, or other syntax error.\n");
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
            if ((current_tok.type == TOK_NUMBER) || (current_tok.type == TOK_NAME)) {
                print_column(current_tok.column);
                print_warning("The function \"%s\" accepts no argument!\n", functions[previous_tok.function_index].name);
            }
        }
    }
    if (previous_tok.type == TOK_OPERATOR) {
        if ((current_tok.type == TOK_FUNCTION) && (!functions[current_tok.function_index].return_value)) {
            print_column(current_tok.column);
            print_error("The function \"%s\" doesn't return a value, so it can't be used in a expression!\n", functions[current_tok.function_index].name);
            return true;
        }
    }
    return false;
}

// This function returns true if found an error
static bool insert_node_right(struct Parser *const parser, size_t *const head_idx, const size_t node_idx) {
    if (array_index_is_invalid(parser->nodes, *head_idx)) {
        *head_idx = node_idx;
    } else {
        size_t previous_idx = *head_idx;
        while (array_index_is_valid(parser->nodes, parser->nodes[previous_idx].right_idx)) {
            previous_idx = parser->nodes[previous_idx].right_idx;
        }
        if (check_parser_right_error(parser, previous_idx, node_idx)) {
            return true;
        }
        parser->nodes[previous_idx].right_idx = node_idx;
    }
    return false;
}

// This function returns true if found an error
static inline bool insert_new_op(struct Parser *const parser, size_t *const head_idx, const size_t node_idx, const size_t max_idx) {
    if (array_index_is_invalid(parser->nodes, *head_idx)) {
        *head_idx = node_idx;
    } else {
        size_t previous_idx = INVALID_PARSER_INDEX;
        {  // Search for the correct place to insert the node
            size_t next_idx = *head_idx;
            const int precedence = get_op_precedence(parser->nodes[node_idx].tok.op);
            while (array_index_is_valid(parser->nodes, next_idx) && (parser->nodes[next_idx].tok.type == TOK_OPERATOR)) {
                if ((next_idx == max_idx) || (get_op_precedence(parser->nodes[next_idx].tok.op) <= precedence)) {
                    break;
                }
                previous_idx = next_idx;
                next_idx = parser->nodes[next_idx].right_idx;
            }
        }
        // Insert new node
        if (array_index_is_invalid(parser->nodes, previous_idx)) {
            parser->nodes[node_idx].left_idx = *head_idx;
            *head_idx = node_idx;
        } else {
            parser->nodes[node_idx].left_idx = parser->nodes[previous_idx].right_idx;
            parser->nodes[previous_idx].right_idx = node_idx;
        }
    }
    return false;
}

// Function declaration to allow its use in parse_function
static bool parse_expression(struct Parser *const parser, size_t *const tk_idx, size_t *const head_idx);

// Functions may have none, one or two arguments. These are stored in the binary tree, with the
// first argument at the left node and the second at the right
// This function is an integrant part of parse_expression, but was factored out to clarify the code
// This function returns true if found an error
static inline bool parse_function(struct Parser *const parser, size_t *const tk_idx, size_t *const head_idx, const struct Token function_token) {
    const size_t function_node_idx = new_node(parser, function_token);
    if (insert_node_right(parser, head_idx, function_node_idx)) {
        return true;
    }
    if (functions[function_token.function_index].arity == 0) {
        // There is no argument to be parsed
        return false;
    }
    // The lexer guarantees for us that the next token after a function
    // with more than one argument is a parentheses '('
    (*tk_idx)++;
    const struct Token parentheses_token = parser->lexer->tokens[*tk_idx];
    (*tk_idx)++;
    if (*tk_idx >= array_size(parser->lexer->tokens)) {
        print_column(parentheses_token.column);
        print_error("Didn't found the closing parentheses for the function declaration!\n");
        return true;
    }
    size_t argument_idx = INVALID_PARSER_INDEX;
    if (parse_expression(parser, tk_idx, &argument_idx)) {
        return true;
    }
    if (array_index_is_invalid(parser->nodes, argument_idx)) {
        print_column(function_token.column);
        print_error("It was impossible to parse the first argument to this function!\n");
        return true;
    }
    // Insert first argument in the left
    parser->nodes[function_node_idx].left_idx = argument_idx;
    if (functions[function_token.function_index].arity >= 2) {
        // The arguments must be separated by comma
        const struct Token comma_token = parser->lexer->tokens[*tk_idx];
        if ((comma_token.type == TOK_DELIMITER) && (comma_token.op == ',')) {
            (*tk_idx)++;
            if ((*tk_idx) < array_size(parser->lexer->tokens)) {
                if (parse_expression(parser, tk_idx, &argument_idx)) {
                    return true;
                }
                if (array_index_is_invalid(parser->nodes, argument_idx)) {
                    print_column(function_token.column);
                    print_error("It was impossible to parse the second argument to this function!\n");
                    return true;
                }
                // Insert second argument to the right
                parser->nodes[function_node_idx].right_idx = argument_idx;
            }
        } else {
            print_column(function_token.column);
            print_error("It was expected a second argument to the function!\n");
            return true;
        }
    }
    if (*tk_idx >= array_size(parser->lexer->tokens)) {
        print_column(parentheses_token.column);
        print_error("Didn't found the closing parentheses for the function declaration!\n");
        return true;
    }
    const struct Token last_token = parser->lexer->tokens[*tk_idx];
    if ((last_token.type != TOK_DELIMITER) || (last_token.op != ')')) {
        print_column(last_token.column);
        print_error("Expected a closing parentheses \")\"!\n");
        return true;
    }
    return false;
}

// This function returns true if found an error
static bool parse_expression(struct Parser *const parser, size_t *const tk_idx, size_t *const head_idx) {
    size_t last_parentheses_idx = INVALID_PARSER_INDEX;
    *head_idx = INVALID_PARSER_INDEX;
    for (; *tk_idx < array_size(parser->lexer->tokens); (*tk_idx)++) {
        const struct Token current_token = parser->lexer->tokens[*tk_idx];
        if (current_token.type == TOK_DELIMITER) {
            switch (current_token.op) {
            case '(': {
                (*tk_idx)++;
                if (*tk_idx >= array_size(parser->lexer->tokens)) {
                    print_column(current_token.column);
                    print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                    return true;
                }
                if (parse_expression(parser, tk_idx, &last_parentheses_idx)) {
                    return true;
                }
                if (*tk_idx >= array_size(parser->lexer->tokens)) {
                    print_column(current_token.column);
                    print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                    return true;
                }
                const struct Token last_token = parser->lexer->tokens[*tk_idx];
                if ((last_token.type != TOK_DELIMITER) || (last_token.op != ')')) {
                    print_column(last_token.column);
                    print_error("Expected a closing parentheses \")\"!\n");
                    return true;
                }
                // If last_parentheses_idx is negative, there is no new node to insert
                if (array_index_is_valid(parser->nodes, last_parentheses_idx)) {
                    if (insert_node_right(parser, head_idx, last_parentheses_idx)) {
                        return true;
                    }
                }                    
            } break;
            case ')':
            case ',':
                return false;
            default:
                print_column(current_token.column);
                print_error("Unrecognized delimiter at parsing phase!\n");
                return true;
            }
        } else if (current_token.type == TOK_FUNCTION) {
            if (parse_function(parser, tk_idx, head_idx, current_token)) {
                return true;
            }
        } else if (current_token.type == TOK_OPERATOR) {
            size_t node_idx = new_node(parser, current_token);
            if (insert_new_op(parser, head_idx, node_idx, last_parentheses_idx)) {
                return true;
            }
        } else {
            size_t node_idx = new_node(parser, current_token);
            if (insert_node_right(parser, head_idx, node_idx)) {
                return true;
            }
        }
    }
    return false;
}

size_t parse(struct Parser *const parser) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (array_size(parser->lexer->tokens) == 0) {
        return INVALID_PARSER_INDEX;
    }
    array_free_all(parser->nodes);
    size_t tk_idx = 0;
    size_t head_idx = INVALID_PARSER_INDEX;
    if (parse_expression(parser, &tk_idx, &head_idx) || array_index_is_invalid(parser->nodes, head_idx)) {
        return INVALID_PARSER_INDEX;
    }
    // Couldn't parse all the expression
    if (tk_idx < array_size(parser->lexer->tokens)) {
        const struct Token next_token = parser->lexer->tokens[tk_idx];
        print_column(next_token.column);
        print_error("Unexpected %s at parsing phase!\n", get_token_type(next_token.type));
        return INVALID_PARSER_INDEX;
    }
    return head_idx;
}

static struct Fn_Arg build_fn_arg(struct Parser *const parser, const size_t node_idx, enum Evaluation_Status *const status) {
    struct Fn_Arg arg = (struct Fn_Arg){
        .value = evaluate(parser, node_idx, status),
        // If the function argument is a name (possible a variable), pass it to the function to be used as a reference
        .name = (array_index_is_valid(parser->nodes, node_idx) && (parser->nodes[node_idx].tok.type == TOK_NAME)) ?
                parser->nodes[node_idx].tok.name : (struct String){0},
    };
    return arg;
}

static inline double perform_function_call(struct Parser *const parser, const size_t node_idx, enum Evaluation_Status *const status) {
    const struct Function function = functions[parser->nodes[node_idx].tok.function_index];
    if (function.fn == NULL) {
        print_column(parser->nodes[node_idx].tok.column);
        print_error("The function \"%s\" was not properly initialized!\n", function.name);
        *status = Eval_Error;
        return NAN;
    }
    const size_t left_idx = parser->nodes[node_idx].left_idx;
    if ((function.arity >= 1) && array_index_is_invalid(parser->nodes, left_idx)) {
        print_column(parser->nodes[node_idx].tok.column);
        print_warning("Did you forget to pass a argument to the function \"%s\"?\n", function.name);
    }
    const size_t right_idx = parser->nodes[node_idx].right_idx;
    if ((function.arity >= 2) && array_index_is_invalid(parser->nodes, right_idx)) {
        print_column(parser->nodes[node_idx].tok.column);
        print_warning("Did you forget to pass the second argument to the function \"%s\"?\n", function.name);
    }
    if ((!function.return_value) && (*status != Eval_Error)) {
        *status = Eval_Dont_Print;
    }
    struct Fn_Arg left_arg = (function.arity >= 1) ? build_fn_arg(parser, left_idx, status) : (struct Fn_Arg){ 0 };
    // If got error at evaluation, don't call the function
    if (*status == Eval_Error) {
        return NAN;
    }
    struct Fn_Arg right_arg = (function.arity >= 2) ? build_fn_arg(parser, right_idx, status) : (struct Fn_Arg){ 0 };
    // If got error at evaluation, don't call the function
    if (*status == Eval_Error) {
        return NAN;
    }
    return function.fn(parser->vars, left_arg, right_arg);
}

double evaluate(struct Parser *const parser, const size_t node_idx, enum Evaluation_Status *const status) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if ((status == NULL) || (*status == Eval_Error)) {
        return NAN;
    }
    if (array_index_is_invalid(parser->nodes, node_idx)) {
        *status = Eval_Dont_Print;
        return NAN;
    }
    switch (parser->nodes[node_idx].tok.type) {
        case TOK_OPERATOR:
            switch (parser->nodes[node_idx].tok.op) {
            case '+':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return (evaluate(parser, parser->nodes[node_idx].left_idx, status) + evaluate(parser, parser->nodes[node_idx].right_idx, status));
            case '-':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return (evaluate(parser, parser->nodes[node_idx].left_idx, status) - evaluate(parser, parser->nodes[node_idx].right_idx, status));
            case '*':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return (evaluate(parser, parser->nodes[node_idx].left_idx, status) * evaluate(parser, parser->nodes[node_idx].right_idx, status));
            case '/':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return (evaluate(parser, parser->nodes[node_idx].left_idx, status) / evaluate(parser, parser->nodes[node_idx].right_idx, status));
            case '^':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return pow(evaluate(parser, parser->nodes[node_idx].left_idx, status), evaluate(parser, parser->nodes[node_idx].right_idx, status));
            case '=': {
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].left_idx) || array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                if (parser->nodes[parser->nodes[node_idx].left_idx].tok.type == TOK_FUNCTION) {
                    print_column(parser->nodes[parser->nodes[node_idx].left_idx].tok.column);
                    print_error("Cannot create a variable named \"%s\", because already exists a function with this name!\n", functions[parser->nodes[parser->nodes[node_idx].left_idx].tok.function_index].name);
                    *status = Eval_Error;
                    return NAN;
                }
                if (parser->nodes[parser->nodes[node_idx].left_idx].tok.type != TOK_NAME) {
                    print_column(parser->nodes[parser->nodes[node_idx].left_idx].tok.column);
                    print_error("Expected variable name for atribution!\n");
                    *status = Eval_Error;
                    return NAN;
                }
                const double result = evaluate(parser, parser->nodes[node_idx].right_idx, status);
                if (*status != Eval_Error) {
                    return assign_variable(parser->vars, parser->nodes[parser->nodes[node_idx].left_idx].tok.name, result);
                } else {
                    return NAN;
                }
            }
            default:
                print_column(parser->nodes[node_idx].tok.column);
                print_error("Invalid binary operator at evaluation phase: %c\n", parser->nodes[node_idx].tok.op);
                *status = Eval_Error;
                return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (parser->nodes[node_idx].tok.op) {
            case '-':
                if (array_index_is_invalid(parser->nodes, parser->nodes[node_idx].right_idx)) {
                    print_column(parser->nodes[node_idx].tok.column);
                    print_warning("Did you forget to include a operand for the operator \"%c\"?\n", parser->nodes[node_idx].tok.op);
                    return NAN;
                }
                return (-evaluate(parser, parser->nodes[node_idx].right_idx, status));
            default:
                print_column(parser->nodes[node_idx].tok.column);
                print_error("Invalid unary operator at evaluation phase: %c\n", parser->nodes[node_idx].tok.op);
                *status = Eval_Error;
                return NAN;
            }
        case TOK_NUMBER: {
            const size_t right_idx = parser->nodes[node_idx].right_idx;
            if (array_index_is_valid(parser->nodes, right_idx)) {
                print_column(parser->nodes[right_idx].tok.column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(parser->nodes[right_idx].tok.type));
            }
            const size_t left_idx = parser->nodes[node_idx].left_idx;
            if (array_index_is_valid(parser->nodes, left_idx)) {
                print_column(parser->nodes[left_idx].tok.column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(parser->nodes[left_idx].tok.type));
            }
            return parser->nodes[node_idx].tok.number;
        }
        case TOK_FUNCTION: 
            return perform_function_call(parser, node_idx, status);
        case TOK_NAME: {
            const size_t right_idx = parser->nodes[node_idx].right_idx;
            if (array_index_is_valid(parser->nodes, right_idx)) {
                print_column(parser->nodes[right_idx].tok.column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(parser->nodes[right_idx].tok.type));
            }
            const size_t left_idx = parser->nodes[node_idx].left_idx;
            if (array_index_is_valid(parser->nodes, left_idx)) {
                print_column(parser->nodes[left_idx].tok.column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(parser->nodes[left_idx].tok.type));
            }
            size_t index;
            const struct String name = parser->nodes[node_idx].tok.name;
            if (search_variable(parser->vars, name, &index) != 0) {
                print_column(parser->nodes[node_idx].tok.column);
                print_error("Unrecognized name: \"%.*s\"!\n", name.length, name.data);
                *status = Eval_Error;
                return NAN;
            }
            return get_variable_value(parser->vars, index);
        }
        case TOK_DELIMITER:
            print_column(parser->nodes[node_idx].tok.column);
            print_error("Unexpected delimiter at evaluation phase: %c\n", parser->nodes[node_idx].tok.op);
            *status = Eval_Error;
            return NAN;
        default:
            print_column(parser->nodes[node_idx].tok.column);
            print_error("Invalid token at evaluation phase: %c\n", parser->nodes[node_idx].tok.op);
            *status = Eval_Error;
            return NAN;
    }
}

static void print_node(struct Parser *const parser, const size_t node_idx, const unsigned int level) {
    if (array_index_is_invalid(parser->nodes, node_idx)) {
        return;
    }
    print_token(parser->nodes[node_idx].tok);
    const size_t left_idx = parser->nodes[node_idx].left_idx;
    if (array_index_is_valid(parser->nodes, left_idx)) {
        printf("%*sLEFT:  ", level * 2, "");
        print_node(parser, left_idx, level + 1);
    }
    const size_t right_idx = parser->nodes[node_idx].right_idx;
    if (array_index_is_valid(parser->nodes, right_idx)) {
        printf("%*sRIGHT: ", level * 2, "");
        print_node(parser, right_idx, level + 1);
    }
}

void print_tree(struct Parser *const parser, const size_t head_idx) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (array_index_is_invalid(parser->nodes, head_idx)) {
        return;
    }
    printf("Abstract syntax tree (AST) generated by the parser:\n");
    printf("HEAD:  ");
    print_node(parser, head_idx, 0);
    printf("\n");
}

#define GRAPH_IDENTATION 4

static void print_graph_node(struct Parser *const parser, const size_t node_idx) {
    if (array_index_is_invalid(parser->nodes, node_idx)) {
        return;
    }
    printf("%*snode%03zu  [ label = \"", GRAPH_IDENTATION, "", node_idx);
    print_token_string(parser->nodes[node_idx].tok);
    printf("\" ];\n");
    // Making connections between nodes
    const size_t left_idx = parser->nodes[node_idx].left_idx;
    if (array_index_is_valid(parser->nodes, left_idx)) {
        print_graph_node(parser, left_idx);
        printf("%*snode%03zu -> node%03zu;\n", GRAPH_IDENTATION, "", node_idx, left_idx);
    }
    const size_t right_idx = parser->nodes[node_idx].right_idx;
    if (array_index_is_valid(parser->nodes, right_idx)) {
        print_graph_node(parser, right_idx);
        printf("%*snode%03zu -> node%03zu;\n", GRAPH_IDENTATION, "", node_idx, right_idx);
    }
}

void print_graph(struct Parser *const parser, const size_t head_idx) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (array_index_is_invalid(parser->nodes, head_idx)) {
        return;
    }
    printf("digraph AST {\n");
    printf("%*snode [ fontname=\"Arial\" ];\n", GRAPH_IDENTATION, "");
    print_graph_node(parser, head_idx);
    printf("}\n\n");
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
