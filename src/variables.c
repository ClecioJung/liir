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

#include "variables.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "data-structures/dynamic_array.h"
#include "data-structures/sized_string.h"
#include "printing.h"

struct Variables create_variables(const size_t initial_list_size) {
    struct Variables vars = (struct Variables){ 0 };
    vars.list = array_new(sizeof(struct Variable), initial_list_size);
    if (vars.list == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the variables!\n");
    }
    return vars;
}

void destroy_variables(struct Variables *const vars) {
    clear_variables(vars);
    array_del(vars->list);
}

void clear_variables(struct Variables *const vars) {
    // Deallocate the memory used to store the variable name
    for (size_t i = 0; i < array_size(vars->list); i++) {
        free((void *)(vars->list[i].name.data));
    }
    array_free_all(vars->list);
}

// This function returns EXIT_SUCCESS if found the variable in the list
// It was implemented this way in order to do a optimization in the function assign_variable
// In this function, if the variable was not found, we already know the correct index to insert it,
// preserving the list of variables sorted
int search_variable(struct Variables *const vars, const struct String name, size_t *const index) {
    // Binary search in variables list
    size_t low = 0;
    size_t high = array_size(vars->list) - 1;
    while ((low <= high) && array_index_is_valid(vars->list, high)) {
        *index = (low + high) / 2; // middle
        const int comp = string_compare(name, vars->list[*index].name);
        if (comp < 0) {
            high = *index - 1;
        } else if (comp > 0) {
            low = *index + 1;
        } else {
            return EXIT_SUCCESS;
        }
    }
    *index = low;
    return EXIT_FAILURE;
}

void new_variable(struct Variables *const vars, const size_t index, const struct String name, const double value) {
    struct Variable new_var = (struct Variable){ 0 };
    new_var.name.data = malloc(name.length*sizeof(char));
    if (new_var.name.data == NULL) {
        print_crash_and_exit("Couldn't allocate memory for the new variable!\n");
    }
    new_var.name.length = name.length;
    new_var.value = value;
    strncpy(new_var.name.data, name.data, name.length);
    array_insert_at(vars->list, index, new_var);
}

int delete_variable(struct Variables *const vars, const struct String name) {
    size_t index;
    if (search_variable(vars, name, &index) != EXIT_SUCCESS) {
        // Didn't found the variable in the list
        return EXIT_FAILURE;
    }
    // Deallocate the memory used to store the variable name
    free((void *)(vars->list[index].name.data));
    array_remove_at(vars->list, index);
    return EXIT_SUCCESS;
}

double assign_variable(struct Variables *const vars, const struct String name, const double value) {
    size_t index;
    if (search_variable(vars, name, &index) != EXIT_SUCCESS) {
        // Insert new variable in alphabetical order
        new_variable(vars, index, name, value);
    } else {
        // Variable already exist
        vars->list[index].value = value;
    }
    return value;
}

double get_variable_value(struct Variables *const vars, const size_t index) {
    if (array_index_is_invalid(vars->list, index)) {
        return NAN;
    }
    return vars->list[index].value;
}

// Defined on main.c
extern unsigned int max_uint(const unsigned int a, const unsigned int b);

static inline unsigned int longest_variable_name(struct Variables *const vars) {
    unsigned int length = 0;
    for (size_t i = 0; i < array_size(vars->list); i++) {
        length = max_uint(length, vars->list[i].name.length);
    }
    return length;
}

void print_variables(struct Variables *const vars) {
    if (array_size(vars->list) == 0) {
        return;
    }
    const char *const header = "Name";
    const unsigned int max_length = max_uint(longest_variable_name(vars), (unsigned int)strlen(header));
    printf("List of variables:\n");
    printf("%-*s Value \n", max_length, header);
    for (size_t i = 0; i < array_size(vars->list); i++) {
        struct String name = vars->list[i].name;
        print_string(name);
        // Fills with empty space, in order to align the variable names
        printf("%*s", (max_length + 1 - (unsigned int)name.length), "");
        printf("%lg\n", vars->list[i].value);
    }
    printf("\n");
}

bool variable_list_is_empty(struct Variables *const vars) {
    return (array_size(vars->list) == 0);
}

// This function was developed during some testing, but is currently unused
// I leave it here because it may be useful in the future
// Remember to free the memory allocated for the returned string
char *get_content_from_file(const char *const file_name) {
	char *buffer = NULL;
	FILE *const file = fopen(file_name, "rb");
	if (file != NULL) {
        // Get file size
        fseek(file, 0, SEEK_END);
        const size_t file_size = (size_t)ftell(file);
        fseek(file, 0, SEEK_SET);
		// Allocate memory to store the entire file
		buffer = (char *)malloc((file_size + 1)*sizeof(char));
		if (buffer != NULL) {
			// Copy the contents of the file to the buffer
			const size_t result = fread(buffer, sizeof(char), file_size, file);
			buffer[file_size] = '\0';
			if (result != file_size) {
				// Reading file error, free dinamically allocated memory
				free((void *)buffer);
				buffer = NULL;
			}
		}
		fclose(file);
	}
	return buffer;
}

// Remove leading and trailing whitespace from a string
void remove_whitespaces(char **str) {
    while (isspace((unsigned char) **str)) {
        (*str)++;
    }
    char *end = *str + strlen(*str) - 1;
    while (end > *str && isspace((unsigned char) *end)) {
        end--;
    }
    end[1] = '\0';
}

void load_variables_from_file(struct Variables *const vars, const struct String file_name) {
    // Convert the file name to a C-string
    char file_name_str[file_name.length + 1];
    strncpy(file_name_str, file_name.data, file_name.length);
    file_name_str[file_name.length] = '\0';
    FILE *const file = fopen(file_name_str, "rb");
	if (file == NULL) {
        print_error("Couldn't read the variables from the file \"%.*s\", because of the following error: %s\n",
            file_name.length, file_name.data, strerror(errno));
        return;
    }
    // Most systems do not allow for a line greather than 4 kbytes
    char line[4096];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *key = strtok(line, "=\n");
        if (key == NULL) {
            continue;
        }
        remove_whitespaces(&key);
        const struct String string_key = create_string(key);
        const struct String name = parse_name(string_key);
        if (name.length != string_key.length) {
            // The key is not a valid name
            print_error("\"%.*s\" is not a valid name!\n", string_key.length, string_key.data);
            continue;
        }
        char *value = strtok(NULL, "=\n");
        if (value == NULL) {
            continue;
        }
        remove_whitespaces(&value);
        const struct String string_value = create_string(value);
        String_Length length;
        double number = parse_number(string_value, &length);
        if (length != string_value.length) {
            // The value is not a valid number
            print_error("\"%.*s\" is not a valid value!\n", string_value.length, string_value.data);
            continue;
        }
        assign_variable(vars, string_key, number);
        printf("%s = %g\n", key, number);
    }
    fclose(file);
}

void save_variables_to_file(struct Variables *const vars, const struct String file_name) {
    if (array_size(vars->list) == 0) {
        return;
    }
    // Convert the file name to a C-string
    char file_name_str[file_name.length + 1];
    strncpy(file_name_str, file_name.data, file_name.length);
    file_name_str[file_name.length] = '\0';
    FILE *const file = fopen(file_name_str, "wb");
	if (file == NULL) {
        print_error("Couldn't save the variables to the file \"%.*s\", because of the following error: %s\n",
            file_name.length, file_name.data, strerror(errno));
        return;
    }
    for (size_t i = 0; i < array_size(vars->list); i++) {
        fprintf(file, "%.*s = %f\n", vars->list[i].name.length, vars->list[i].name.data, vars->list[i].value);
    }
    fclose(file);
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
