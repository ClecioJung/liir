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

#include "allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct Allocator allocator_construct(const size_t base_size, const size_t initial_capacity) {
    struct Allocator allocator = {
        .base_size = base_size,
        .size = 0,
        .capacity = 0,
        .data = NULL,
    };
    allocator.data = (uint8_t *)malloc(initial_capacity * base_size);
    if (allocator.data != NULL) {
        allocator.capacity = initial_capacity;
    }
    return allocator;
}

void allocator_delete(struct Allocator *const allocator) {
    if (allocator != NULL) {
        free(allocator->data);
        allocator->size = 0;
        allocator->capacity = 0;
        allocator->data = NULL;
    }
}

void allocator_free_all(struct Allocator *const allocator) {
    allocator->size = 0;
}

void allocator_free_last(struct Allocator *const allocator) {
    allocator->size--;
}

int64_t allocator_new(struct Allocator *const allocator) {
    if (allocator == NULL) {
        return -1;
    }
    while ((allocator->size + 1) >= allocator->capacity) {
        allocator->capacity *= 2;
        uint8_t *new_data = (uint8_t *)realloc(allocator->data, allocator->capacity * allocator->base_size);
        if (new_data != NULL) {
            allocator->data = new_data;
        } else {
            // Could't allocate more memory!
            return -1;
        }
    }
    return ((int64_t)(allocator->size++));
}

int64_t allocator_new_array(struct Allocator *const allocator, const size_t size) {
    if (allocator == NULL) {
        return -1;
    }
    while ((allocator->size + size) >= allocator->capacity) {
        allocator->capacity *= 2;
        uint8_t *new_data = (uint8_t *)realloc(allocator->data, allocator->capacity * allocator->base_size);
        if (new_data != NULL) {
            allocator->data = new_data;
        } else {
            // Could't allocate more memory!
            return -1;
        }
    }
    int64_t ret = (int64_t)allocator->size;
    allocator->size += size;
    return ret;
}

bool allocator_is_valid(const struct Allocator allocator, const int64_t index) {
    return ((index >= 0) && ((size_t)index < allocator.size));
}

bool allocator_is_invalid(const struct Allocator allocator, const int64_t index) {
    return ((index < 0) || ((size_t)index >= allocator.size));
}

void *allocator_get(const struct Allocator allocator, const int64_t index) {
    if (allocator_is_invalid(allocator, index)) {
        return NULL;
    }
    return (&allocator.data[0] + allocator.base_size * (size_t)index);
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