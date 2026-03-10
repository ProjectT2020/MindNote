#ifndef HASHTABLE_U64_H
#define HASHTABLE_U64_H

#include <stddef.h>
#include <stdint.h>

typedef struct U64HashtableEntry {
    uint64_t key;
    void *value;
    struct U64HashtableEntry *next;
} U64HashtableEntry;

typedef size_t (*u64_hash_func)(uint64_t key, size_t capacity);

typedef struct {
    size_t capacity;
    size_t size;
    U64HashtableEntry **buckets;
    u64_hash_func hash;
} U64Hashtable;

U64Hashtable* u64_hashtable_create(u64_hash_func func, size_t initial_capacity);
void u64_hashtable_destroy(U64Hashtable* ht);

void *u64_hashtable_find(U64Hashtable* ht, uint64_t key);
int u64_hashtable_insert(U64Hashtable* ht, uint64_t key, void* value);
int u64_hashtable_remove(U64Hashtable* ht, uint64_t key);

// Default hash function if caller passes NULL
size_t u64_default_hash(uint64_t key, size_t capacity);

#endif // HASHTABLE_U64_H
