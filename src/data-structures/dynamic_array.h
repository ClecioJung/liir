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
// HEADER
//------------------------------------------------------------------------------

#include <stddef.h>
#include <string.h>

#ifndef __DYNAMIC_ARRAY
#define __DYNAMIC_ARRAY

// This file is a header only library. In order to include it's implementation,
// define the macro DYNAMIC_ARRAY_IMPLEMENTATION before including this file

// It is a classic implementation of dynamic arrays in C
// Be carefull, this implementation uses a lot of macros, and therefore,
// is not entirely type safe
// It can be used as a custom allocator, where the cunks of memory are all of the same size
// This approach is simple, fast, saves memory and avoids memory fragmentation

#define HEADER_SIZE           (2*sizeof(size_t))

#define array_size(array)     ((size_t *)(array))[-1]
#define array_capacity(array) ((size_t *)(array))[-2]

#define array_del(array) free((void *)((char *)(array) - HEADER_SIZE))
#define array_free_all(array) (array_size(array) = 0)

#define array_resize(array, new_capacity) \
    _array_resize((array), sizeof(*(array)), (new_capacity))

#define array_index_is_valid(array, index) ((index) < array_size(array))
#define array_index_is_invalid(array, index) ((index) >= array_size(array))
#define array_at(array, index) ((array)[(index)])
#define array_last(array) ((array)[array_size(array)-1])
#define array_pop(array) ((array)[array_size(array)--])

#define array_push(array, value) \
    do { \
        (array) = (array_size(array)+1) >= array_capacity(array) ? array_resize((array), 2*array_capacity(array)) : (array); \
        if ((array) != NULL) { \
            (array)[array_size(array)++] = (value); \
        } \
    } while (0)

#define array_insert_at(array, index, value) \
    do { \
        if (array_index_is_valid((array), (index))) { \
            (array) = (array_size(array)+1) >= array_capacity(array) ? array_resize((array), 2*array_capacity(array)) : (array); \
            if ((array) != NULL) { \
                memmove(&(array)[(index)+1], &(array)[(index)], (array_size(array) - (index))*sizeof(*(array))); \
                (array)[(index)] = (value); \
                array_size(array)++; \
            } \
        } else { \
            array_push((array), (value)); \
        } \
    } while (0)

#define array_remove_at(array, index) \
    do { \
        if ((array_size(array) >= 1) && array_index_is_valid((array), (index))) { \
            memmove(&(array)[(index)], &(array)[(index)+1], (array_size(array) - (index))*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

// Function prototypes
void *array_new(size_t base_size, size_t initial_capacity) __attribute__((warn_unused_result));
// This function shouldn't be called directly, insted use the macro array_resize
void *_array_resize(void *array, size_t base_size, size_t new_capacity) __attribute__((warn_unused_result));

#endif  // __DYNAMIC_ARRAY

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

#include <stdlib.h>

void *array_new(size_t base_size, size_t initial_capacity) {
    void *p = malloc(initial_capacity * base_size + HEADER_SIZE);
    if (p == NULL) {
        return NULL;
    }
    void *array = (char *)p + HEADER_SIZE;
    array_size(array) = 0;
    array_capacity(array) = initial_capacity;
    return(array);
}

// This function shouldn't be called directly, insted use the macro array_resize
void *_array_resize(void *array, size_t base_size, size_t new_capacity) {
    void *p = ((char *)array - HEADER_SIZE);
    void *new_p = realloc(p, new_capacity * base_size + HEADER_SIZE);
    if (new_p == NULL) {
        free(p);
        return NULL;
    }
    void *new_array = (char *)new_p + HEADER_SIZE;
    array_capacity(new_array) = new_capacity;
    return(new_array);
}

#endif // DYNAMIC_ARRAY_IMPLEMENTATION

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
