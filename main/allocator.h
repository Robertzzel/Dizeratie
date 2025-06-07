#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>   // size_t, alignof
#include <stdint.h>   // uintptr_t
#include <string.h>   // memset
#include <stdalign.h> // alignof, max_align_t

#define GET_PADDING(ptr, alignment) ((-(uintptr_t)(ptr)) & ((alignment) - 1))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
typedef struct {
    uintptr_t free_memory_start;
    uintptr_t memory_end;
    uintptr_t memory_start;
} allocator_t;

void allocator_init(allocator_t *a, void *buffer, size_t size) {
    a->free_memory_start = a->memory_start = (uintptr_t)buffer + GET_PADDING(buffer, alignof(uintptr_t));
    a->memory_end = (uintptr_t)buffer + size;
    memset(buffer, 0, size);
}

void allocator_reset(allocator_t *a) {
    a->free_memory_start = a->memory_start;
}

static inline size_t allocator_used(allocator_t* allocator) {
    return allocator->free_memory_start - allocator->memory_start;
}

static inline size_t allocator_available(allocator_t* allocator) {
    size_t diff = allocator->memory_end - allocator->free_memory_start;
    size_t rem = diff - GET_PADDING(allocator->free_memory_start, alignof(uintptr_t)) - 2 * sizeof(uintptr_t);
    if (rem > diff) return 0;
    return rem;
}

static inline uintptr_t _allocator_get_last_block(allocator_t *a) {
    return a->free_memory_start == a->memory_start ? 0 : *(uintptr_t*)(a->free_memory_start - sizeof(uintptr_t));
}

void* _allocator_alloc_internal(allocator_t *a, size_t size, size_t align, size_t count) {
    size_t label1_padding = GET_PADDING(a->free_memory_start, alignof(uintptr_t));
    size_t label_size = sizeof(uintptr_t);

    size_t alloc_padding_size = GET_PADDING(a->free_memory_start + label1_padding + label_size, align);
    size_t alloc_size = size * count;

    size_t label2_padding = GET_PADDING(a->free_memory_start + label1_padding + label_size + alloc_padding_size + alloc_size, align);

    size_t total = label1_padding + label_size + alloc_padding_size + alloc_size + label2_padding + label_size;
    size_t available = a->memory_end - a->free_memory_start;
    if (available < total) {
        return NULL;
    }

    if (label1_padding > 0) {
        uintptr_t label_ptr = _allocator_get_last_block(a);
        if (label_ptr != 0) {
            *(uintptr_t*)label_ptr = label_ptr + label1_padding;
        }
    }

    uintptr_t x = a->free_memory_start + total;
    memcpy((uint8_t*)a->free_memory_start + label1_padding, &x, label_size);
    memcpy((uint8_t*)a->free_memory_start + total - label_size, &a->free_memory_start, label_size);

    void *ptr = (void*)(a->free_memory_start + label1_padding + label_size + alloc_padding_size);
    a->free_memory_start = x;
    return ptr;
}

void allocator_free(allocator_t *a) {
    uintptr_t last_label = _allocator_get_last_block(a);
    if (last_label == 0) {
        return;
    }
    a->free_memory_start = last_label;
}

void allocator_free_n(allocator_t *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        uintptr_t last_label = _allocator_get_last_block(a);
        if (last_label == 0) break;

        a->free_memory_start = last_label;
    }
}

void _allocator_realloc(allocator_t *a, size_t new_total_size) {
    uintptr_t beg_label_ptr = _allocator_get_last_block(a);
    if (beg_label_ptr == 0) return;
    uintptr_t end_label_ptr = a->free_memory_start - sizeof(uintptr_t);

    size_t current_size = end_label_ptr - beg_label_ptr - sizeof(uintptr_t);
    int64_t size_diff = new_total_size - current_size;

    if (size_diff > 0 && allocator_available(a) <= size_diff) {
        return;
    }

    uint8_t endpad = GET_PADDING((void*)(end_label_ptr + size_diff), alignof(uintptr_t));
    *(uintptr_t*)beg_label_ptr = (*(uintptr_t*)beg_label_ptr) + size_diff + endpad;
    *(uintptr_t*)(end_label_ptr + size_diff + endpad) = *(uintptr_t*)end_label_ptr;
    a->free_memory_start += size_diff + endpad;
}

#define allocator_alloc_type(arena_ptr, type, count) \
    ((type *)_allocator_alloc_internal((arena_ptr), sizeof(type), alignof(type), (count)))

#define allocator_realloc_type(arena_ptr, type, count) \
    _allocator_realloc((arena_ptr), sizeof(type) * (count))

char* allocator_strndup(allocator_t* allocator, const char *src, size_t n) {
    if (!src || n == 0) return NULL;

    size_t len = strnlen(src, n);
    char *dup = allocator_alloc_type(allocator, char, len + 1);
    if (!dup) return NULL;

    strncpy(dup, src, len);
    dup[len] = '\0'; // Null-terminate the string
    return dup;
}


#endif