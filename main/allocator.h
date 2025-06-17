#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <stddef.h>  
#include <stdint.h>  
#include <string.h>  
#include <stdalign.h> 
#define MAX_ALIGNMENT alignof(max_align_t)
#define ALIGN_UP(ptr, alignment) ((-(uintptr_t)(ptr)) & ((alignment) - 1))

typedef struct {
    uint8_t* buffer;
    size_t size;
    size_t used;
} allocator_t;

void allocator_init(allocator_t *a, void *buffer, size_t size) {
    a->buffer = buffer + ALIGN_UP(buffer, MAX_ALIGNMENT);
    a->size = size;
    a->used = 0;
}

void allocator_reset(allocator_t *a) {
    a->used = 0;
}

static inline size_t allocator_used(allocator_t* allocator) {
    return allocator->used;
}

static inline size_t allocator_available(allocator_t* allocator) {
    return allocator->size - allocator->used;
}

void* _allocator_alloc_internal(allocator_t *a, size_t size, size_t align, size_t count) {
    uint8_t alloc_padding = ALIGN_UP(a->buffer + a->used, align);
    size_t alloc_size = size * count;

    size_t total = alloc_padding + alloc_size;
    size_t available = allocator_available(a);
    if (available < total) {
        return NULL;
    }

    a->used += total;
    return a->buffer + a->used - alloc_size;
}

void allocator_free(allocator_t *a, void* ptr) {
    if (allocator_used(a) == 0) {
        return;
    }
    a->used = (uint8_t*)ptr - a->buffer;
}

#define allocator_alloc_type(arena_ptr, type, count) \
    ((type *)_allocator_alloc_internal((arena_ptr), sizeof(type), alignof(type), (count)))

void* _allocator_realloc(allocator_t *a, void* ptr, size_t new_total_size) {
    size_t current_end = a->used;
    allocator_free(a, ptr);
    if (allocator_alloc_type(a, uint8_t, new_total_size) == NULL) {
        a->used = current_end;
        return NULL;
    }
    return ptr;
}

#define allocator_realloc_type(arena_ptr, ptr, type, count) \
    _allocator_realloc((arena_ptr), ptr, sizeof(type) * (count))

char* allocator_strndup(allocator_t* allocator, const char *src, size_t n) {
    if (!src || n == 0) return NULL;

    size_t len = strnlen(src, n);
    char *dup = allocator_alloc_type(allocator, char, len + 1);
    if (!dup) return NULL;

    strncpy(dup, src, len);
    dup[len] = '\0';
    return dup;
}


#endif