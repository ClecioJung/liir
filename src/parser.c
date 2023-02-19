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

#include "data-structures/allocator.h"
#include "data-structures/sized_string.h"
#include "functions.h"
#include "lex.h"
#include "print_errors.h"
#include "variables.h"

struct Parser create_parser(struct Lexer *const lexer, struct Variables *const vars, const size_t initial_size) {
    if ((lexer == NULL) || (vars == NULL)) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    struct Parser parser = (struct Parser){
        .lexer = lexer,
        .vars = vars,
        .nodes = allocator_construct(sizeof(struct Token_Node), initial_size),
    };
    if (parser.nodes.data == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the parser!\n");
    }
    return parser;
}

void destroy_parser(struct Parser *const parser) {
    if (parser != NULL) {
        allocator_delete(&parser->nodes);
    }
}

static int64_t new_node(struct Parser *const parser, const struct Token tok) {
    int64_t index = allocator_new(&parser->nodes);
    if (index < 0) {
        print_crash_and_exit("Couldn't allocate more memory for the parser!\n");
    }
    struct Token_Node *const node = allocator_get(parser->nodes, index);
    *node = (struct Token_Node){
        .tok = tok,
        .left_idx = -1,
        .right_idx = -1,
    };
    return index;
}

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

static inline struct Token *get_lex_tok(struct Parser *const parser, int64_t index) {
    return allocator_get(parser->lexer->tokens, index);
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

// This function returns true if found an error
static inline bool check_parser_right_error(struct Parser *const parser, const int64_t previous_idx, const int64_t current_idx) {
    struct Token previous_tok = get_tok(parser, previous_idx);
    struct Token current_tok = get_tok(parser, current_idx);
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
static bool insert_node_right(struct Parser *const parser, int64_t *const head_idx, const int64_t node_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = *head_idx;
        while (is_valid_idx(parser, get_right_idx(parser, previous_idx))) {
            previous_idx = get_right_idx(parser, previous_idx);
        }
        if (check_parser_right_error(parser, previous_idx, node_idx)) {
            return true;
        }
        get_node_ptr(parser, previous_idx)->right_idx = node_idx;
    }
    return false;
}

// This function returns true if found an error
static inline bool insert_new_op(struct Parser *const parser, int64_t *const head_idx, const int64_t node_idx, const int64_t max_idx) {
    if (*head_idx < 0) {
        *head_idx = node_idx;
    } else {
        int64_t previous_idx = -1;
        {  // Search for the correct place to insert the node
            int64_t next_idx = *head_idx;
            const int precedence = get_op_precedence(get_tok(parser, node_idx).op);
            while (is_valid_idx(parser, next_idx) && (get_tok(parser, next_idx).type == TOK_OPERATOR)) {
                if ((next_idx == max_idx) || (get_op_precedence(get_tok(parser, next_idx).op) <= precedence)) {
                    break;
                }
                previous_idx = next_idx;
                next_idx = get_right_idx(parser, next_idx);
            }
        }
        // Insert new node
        if (is_invalid_idx(parser, previous_idx)) {
            get_node_ptr(parser, node_idx)->left_idx = *head_idx;
            *head_idx = node_idx;
        } else {
            get_node_ptr(parser, node_idx)->left_idx = get_right_idx(parser, previous_idx);
            get_node_ptr(parser, previous_idx)->right_idx = node_idx;
        }
    }
    return false;
}

// Function declaration to allow its use in parse_function
static bool parse_expression(struct Parser *const parser, size_t *const tk_idx, int64_t *const head_idx);

// Functions may have none, one or two arguments. These are stored in the binary tree, with the
// first argument at the left node and the second at the right
// This function is an integrant part of parse_expression, but was factored out to clarify the code
// This function returns true if found an error
static inline bool parse_function(struct Parser *const parser, size_t *const tk_idx, int64_t *const head_idx, const struct Token function_token) {
    const int64_t function_node_idx = new_node(parser, function_token);
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
    const struct Token parentheses_token = *get_lex_tok(parser, (int64_t)*tk_idx);
    (*tk_idx)++;
    if (*tk_idx >= parser->lexer->tokens.size) {
        print_column(parentheses_token.column);
        print_error("Didn't found the closing parentheses for the function declaration!\n");
        return true;
    }
    int64_t argument_idx = -1;
    if (parse_expression(parser, tk_idx, &argument_idx)) {
        return true;
    }
    if (argument_idx < 0) {
        print_column(function_token.column);
        print_error("It was impossible to parse the first argument to this function!\n");
        return true;
    }
    // Insert first argument in the left
    get_node_ptr(parser, function_node_idx)->left_idx = argument_idx;
    if (functions[function_token.function_index].arity >= 2) {
        // The arguments must be separated by comma
        const struct Token comma_token = *get_lex_tok(parser, (int64_t)*tk_idx);
        if ((comma_token.type == TOK_DELIMITER) && (comma_token.op == ',')) {
            (*tk_idx)++;
            if ((*tk_idx) < parser->lexer->tokens.size) {
                if (parse_expression(parser, tk_idx, &argument_idx)) {
                    return true;
                }
                if (argument_idx < 0) {
                    print_column(function_token.column);
                    print_error("It was impossible to parse the second argument to this function!\n");
                    return true;
                }
                // Insert second argument to the right
                get_node_ptr(parser, function_node_idx)->right_idx = argument_idx;
            }
        } else {
            print_column(function_token.column);
            print_error("It was expected a second argument to the function!\n");
            return true;
        }
    }
    if (*tk_idx >= parser->lexer->tokens.size) {
        print_column(parentheses_token.column);
        print_error("Didn't found the closing parentheses for the function declaration!\n");
        return true;
    }
    const struct Token last_token = *get_lex_tok(parser, (int64_t)*tk_idx);
    if ((last_token.type != TOK_DELIMITER) || (last_token.op != ')')) {
        print_column(last_token.column);
        print_error("Expected a closing parentheses \")\"!\n");
        return true;
    }
    return false;
}

// This function returns true if found an error
static bool parse_expression(struct Parser *const parser, size_t *const tk_idx, int64_t *const head_idx) {
    int64_t last_parentheses_idx = -1;
    *head_idx = -1;
    for (; *tk_idx < parser->lexer->tokens.size; (*tk_idx)++) {
        const struct Token current_token = *get_lex_tok(parser, (int64_t)*tk_idx);
        if (current_token.type == TOK_DELIMITER) {
            switch (current_token.op) {
            case '(': {
                (*tk_idx)++;
                if (*tk_idx >= parser->lexer->tokens.size) {
                    print_column(current_token.column);
                    print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                    return true;
                }
                if (parse_expression(parser, tk_idx, &last_parentheses_idx)) {
                    return true;
                }
                if (*tk_idx >= parser->lexer->tokens.size) {
                    print_column(current_token.column);
                    print_error("Mismatched delimiters! Not all parentheses were closed!\n");
                    return true;
                }
                const struct Token last_token = *get_lex_tok(parser, (int64_t)*tk_idx);
                if ((last_token.type != TOK_DELIMITER) || (last_token.op != ')')) {
                    print_column(last_token.column);
                    print_error("Expected a closing parentheses \")\"!\n");
                    return true;
                }
                // If last_parentheses_idx is negative, there is no new node to insert
                if (last_parentheses_idx >= 0) {
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
            int64_t node_idx = new_node(parser, current_token);
            if (insert_new_op(parser, head_idx, node_idx, last_parentheses_idx)) {
                return true;
            }
        } else {
            int64_t node_idx = new_node(parser, current_token);
            if (insert_node_right(parser, head_idx, node_idx)) {
                return true;
            }
        }
    }
    return false;
}

int64_t parse(struct Parser *const parser) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (parser->lexer->tokens.size == 0) {
        return -1;
    }
    allocator_free_all(&parser->nodes);
    size_t tk_idx = 0;
    int64_t head_idx = -1;
    if (parse_expression(parser, &tk_idx, &head_idx) || (head_idx < 0)) {
        return -1;
    }
    // Couldn't parse all the expression
    if (tk_idx < parser->lexer->tokens.size) {
        const struct Token next_token = *get_lex_tok(parser, (int64_t)tk_idx);
        print_column(next_token.column);
        print_error("Unexpected %s at parsing phase!\n", get_token_type(next_token.type));
        return -1;
    }
    return head_idx;
}

static struct Fn_Arg build_fn_arg(struct Parser *const parser, const int64_t node_idx, enum Evaluation_Status *const status) {
    struct Fn_Arg arg = (struct Fn_Arg){
        .value = evaluate(parser, node_idx, status),
        // If the function argument is a name (possible a variable), pass it to the function to be used as a reference
        .name = (is_valid_idx(parser, node_idx) && (get_tok(parser, node_idx).type == TOK_NAME)) ?
                get_tok(parser, node_idx).name : (struct String){0},
    };
    return arg;
}

static inline double perform_function_call(struct Parser *const parser, const int64_t node_idx, enum Evaluation_Status *const status) {
    const struct Function function = functions[get_tok(parser, node_idx).function_index];
    if (function.fn == NULL) {
        print_column(get_tok(parser, node_idx).column);
        print_error("The function \"%s\" was not properly initialized!\n", function.name);
        *status = Eval_Error;
        return NAN;
    }
    const int64_t left_idx = get_left_idx(parser, node_idx);
    if ((function.arity >= 1) && is_invalid_idx(parser, left_idx)) {
        print_column(get_tok(parser, node_idx).column);
        print_warning("Did you forget to pass a argument to the function \"%s\"?\n", function.name);
    }
    const int64_t right_idx = get_right_idx(parser, node_idx);
    if ((function.arity >= 2) && is_invalid_idx(parser, right_idx)) {
        print_column(get_tok(parser, node_idx).column);
        print_warning("Did you forget to pass the second argument to the function \"%s\"?\n", function.name);
    }
    if ((!function.return_value) && (*status != Eval_Error)) {
        *status = Eval_Dont_Print;
    }
    struct Fn_Arg left_arg = build_fn_arg(parser, left_idx, status);
    // If got error at evaluation, don't call the function
    if (*status == Eval_Error) {
        return NAN;
    }
    struct Fn_Arg right_arg = build_fn_arg(parser, right_idx, status);
    // If got error at evaluation, don't call the function
    if (*status == Eval_Error) {
        return NAN;
    }
    return function.fn(parser->vars, left_arg, right_arg);
}

double evaluate(struct Parser *const parser, const int64_t node_idx, enum Evaluation_Status *const status) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    if (is_invalid_idx(parser, node_idx)) {
        return NAN;
    }
    if ((status == NULL) || (*status == Eval_Error)) {
        return NAN;
    }
    switch (get_tok(parser, node_idx).type) {
        case TOK_OPERATOR:
            switch (get_tok(parser, node_idx).op) {
                case '+':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) + evaluate(parser, get_right_idx(parser, node_idx), status));
                case '-':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) - evaluate(parser, get_right_idx(parser, node_idx), status));
                case '*':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) * evaluate(parser, get_right_idx(parser, node_idx), status));
                case '/':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (evaluate(parser, get_left_idx(parser, node_idx), status) / evaluate(parser, get_right_idx(parser, node_idx), status));
                case '^':
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return pow(evaluate(parser, get_left_idx(parser, node_idx), status), evaluate(parser, get_right_idx(parser, node_idx), status));
                case '=': {
                    if (is_invalid_idx(parser, get_left_idx(parser, node_idx)) || is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    if (get_tok(parser, get_left_idx(parser, node_idx)).type == TOK_FUNCTION) {
                        print_column(get_tok(parser, get_left_idx(parser, node_idx)).column);
                        print_error("Cannot create a variable named \"%s\", because already exists a function with this name!\n", functions[get_tok(parser, get_left_idx(parser, node_idx)).function_index].name);
                        *status = Eval_Error;
                        return NAN;
                    }
                    if (get_tok(parser, get_left_idx(parser, node_idx)).type != TOK_NAME) {
                        print_column(get_tok(parser, get_left_idx(parser, node_idx)).column);
                        print_error("Expected variable name for atribution!\n");
                        *status = Eval_Error;
                        return NAN;
                    }
                    const double result = evaluate(parser, get_right_idx(parser, node_idx), status);
                    if (*status != Eval_Error) {
                        return assign_variable(parser->vars, get_tok(parser, get_left_idx(parser, node_idx)).name, result);
                    } else {
                        return NAN;
                    }
                }
                default:
                    print_column(get_tok(parser, node_idx).column);
                    print_error("Invalid binary operator at evaluation phase: %c\n", get_tok(parser, node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_UNARY_OPERATOR:
            switch (get_tok(parser, node_idx).op) {
                case '-':
                    if (is_invalid_idx(parser, get_right_idx(parser, node_idx))) {
                        print_column(get_tok(parser, node_idx).column);
                        print_warning("Did you forget to include a operand for the operator \"%c\"?\n", get_tok(parser, node_idx).op);
                        return NAN;
                    }
                    return (-evaluate(parser, get_right_idx(parser, node_idx), status));
                default:
                    print_column(get_tok(parser, node_idx).column);
                    print_error("Invalid unary operator at evaluation phase: %c\n", get_tok(parser, node_idx).op);
                    *status = Eval_Error;
                    return NAN;
            }
        case TOK_NUMBER: {
            const int64_t right_idx = get_right_idx(parser, node_idx);
            if (is_valid_idx(parser, right_idx)) {
                print_column(get_tok(parser, right_idx).column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(get_tok(parser, right_idx).type));
            }
            const int64_t left_idx = get_left_idx(parser, node_idx);
            if (is_valid_idx(parser, left_idx)) {
                print_column(get_tok(parser, left_idx).column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(get_tok(parser, left_idx).type));
            }
            return get_tok(parser, node_idx).number;
        }
        case TOK_FUNCTION: 
            return perform_function_call(parser, node_idx, status);
        case TOK_NAME: {
            const int64_t right_idx = get_right_idx(parser, node_idx);
            if (is_valid_idx(parser, right_idx)) {
                print_column(get_tok(parser, right_idx).column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(get_tok(parser, right_idx).type));
            }
            const int64_t left_idx = get_left_idx(parser, node_idx);
            if (is_valid_idx(parser, left_idx)) {
                print_column(get_tok(parser, left_idx).column);
                print_warning("Invalid %s at evaluation phase\n", get_token_type(get_tok(parser, left_idx).type));
            }
            int64_t index;
            const struct String name = get_tok(parser, node_idx).name;
            if (search_variable(parser->vars, name, &index) != 0) {
                print_column(get_tok(parser, node_idx).column);
                print_error("Unrecognized name: \"%.*s\"!\n", name.length, name.data);
                *status = Eval_Error;
                return NAN;
            }
            return get_variable_value(parser->vars, index);
        }
        case TOK_DELIMITER:
            print_column(get_tok(parser, node_idx).column);
            print_error("Unexpected delimiter at evaluation phase: %c\n", get_tok(parser, node_idx).op);
            *status = Eval_Error;
            return NAN;
        default:
            print_column(get_tok(parser, node_idx).column);
            print_error("Invalid token at evaluation phase: %c\n", get_tok(parser, node_idx).op);
            *status = Eval_Error;
            return NAN;
    }
}

static void print_node(struct Parser *const parser, const int64_t node_idx, const unsigned int level) {
    if (is_invalid_idx(parser, node_idx)) {
        return;
    }
    print_token(get_tok(parser, node_idx));
    if (get_left_idx(parser, node_idx) >= 0) {
        printf("%*sLEFT:  ", level * 2, "");
        print_node(parser, get_left_idx(parser, node_idx), level + 1);
    }
    if (get_right_idx(parser, node_idx) >= 0) {
        printf("%*sRIGHT: ", level * 2, "");
        print_node(parser, get_right_idx(parser, node_idx), level + 1);
    }
}

void print_tree(struct Parser *const parser, const int64_t head_idx) {
    if (parser == NULL) {
        print_crash_and_exit("Invalid call to function \"%s()\"!\n", __func__);
    }
    printf("Abstract syntax tree (AST) generated by the parser:\n");
    printf("HEAD:  ");
    print_node(parser, head_idx, 0);
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