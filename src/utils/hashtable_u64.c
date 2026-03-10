#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hashtable_u64.h"

size_t u64_default_hash(uint64_t key, size_t capacity) {
    // Mix bits (xorshift-like) then mod capacity
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return (size_t)(key % capacity);
}

static size_t round_up_pow2(size_t v) {
    size_t p = 1;
    while (p < v) p <<= 1;
    return p;
}

U64Hashtable* u64_hashtable_create(u64_hash_func func, size_t initial_capacity) {
    U64Hashtable* ht = (U64Hashtable*)calloc(1, sizeof(U64Hashtable));
    if (!ht) return NULL;

    ht->capacity = round_up_pow2(initial_capacity ? initial_capacity : 1024);
    ht->size = 0;
    ht->hash = func ? func : u64_default_hash;
    ht->buckets = (U64HashtableEntry**)calloc(ht->capacity, sizeof(U64HashtableEntry*));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    return ht;
}

void u64_hashtable_destroy(U64Hashtable* ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->capacity; i++) {
        U64HashtableEntry *entry = ht->buckets[i];
        while (entry) {
            U64HashtableEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

static void ensure_capacity(U64Hashtable* ht) {
    if ((double)ht->size / ht->capacity > 0.75) {
        size_t new_capacity = ht->capacity * 2;
        U64HashtableEntry **new_buckets = (U64HashtableEntry**)calloc(new_capacity, sizeof(U64HashtableEntry*));
        if (!new_buckets) return;

        for (size_t i = 0; i < ht->capacity; i++) {
            U64HashtableEntry *entry = ht->buckets[i];
            while (entry) {
                U64HashtableEntry *next = entry->next;
                size_t index = ht->hash(entry->key, new_capacity);
                entry->next = new_buckets[index];
                new_buckets[index] = entry;
                entry = next;
            }
        }

        free(ht->buckets);
        ht->buckets = new_buckets;
        ht->capacity = new_capacity;
    }
}

void *u64_hashtable_find(U64Hashtable* ht, uint64_t key) {
    if (!ht) return NULL;
    size_t index = ht->hash(key, ht->capacity);
    U64HashtableEntry *entry = ht->buckets[index];
    while (entry) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

int u64_hashtable_insert(U64Hashtable* ht, uint64_t key, void* value) {
    if (!ht) return -1;

    ensure_capacity(ht);
    size_t index = ht->hash(key, ht->capacity);

    U64HashtableEntry *entry = ht->buckets[index];
    while (entry) {
        if (entry->key == key) {
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }

    U64HashtableEntry *new_entry = (U64HashtableEntry*)malloc(sizeof(U64HashtableEntry));
    if (!new_entry) return -1;
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->size++;
    return 0;
}

static void may_shrink(U64Hashtable *ht) {
    if (ht->capacity > 1024 && (double)ht->size / ht->capacity < 0.25) {
        size_t new_capacity = ht->capacity / 2;
        U64HashtableEntry **new_buckets = (U64HashtableEntry**)calloc(new_capacity, sizeof(U64HashtableEntry*));
        if (!new_buckets) return;

        for (size_t i = 0; i < ht->capacity; i++) {
            U64HashtableEntry *entry = ht->buckets[i];
            while (entry) {
                U64HashtableEntry *next = entry->next;
                size_t index = ht->hash(entry->key, new_capacity);
                entry->next = new_buckets[index];
                new_buckets[index] = entry;
                entry = next;
            }
        }

        free(ht->buckets);
        ht->buckets = new_buckets;
        ht->capacity = new_capacity;
    }
}

int u64_hashtable_remove(U64Hashtable* ht, uint64_t key) {
    if (!ht) return -1;

    size_t index = ht->hash(key, ht->capacity);
    U64HashtableEntry *entry = ht->buckets[index];
    U64HashtableEntry *prev = NULL;

    int ret = -1;
    while (entry) {
        if (entry->key == key) {
            if (prev) {
                prev->next = entry->next;
            } else {
                ht->buckets[index] = entry->next;
            }
            free(entry);
            ht->size--;
            ret = 0;
            break;
        }
        prev = entry;
        entry = entry->next;
    }
    may_shrink(ht);
    return ret;
}
