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

void allocator_init(allocator_t *a, void *buffer, size_t size);

void allocator_reset(allocator_t *a);

size_t allocator_used(allocator_t* allocator);

size_t allocator_available(allocator_t* allocator);

void* _allocator_alloc_internal(allocator_t *a, size_t size, size_t align, size_t count);

void allocator_free(allocator_t *a, void* ptr);

#define allocator_alloc_type(arena_ptr, type, count) \
    ((type *)_allocator_alloc_internal((arena_ptr), sizeof(type), alignof(type), (count)))

void* _allocator_realloc(allocator_t *a, void* ptr, size_t new_total_size);

#define allocator_realloc_type(arena_ptr, ptr, type, count) \
    _allocator_realloc((arena_ptr), ptr, sizeof(type) * (count))

char* allocator_strndup(allocator_t* allocator, const char *src, size_t n);


#endif
