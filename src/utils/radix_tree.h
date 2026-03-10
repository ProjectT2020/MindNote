
#ifndef RADIX_TREE_H
#define RADIX_TREE_H

#include <stdint.h>

typedef struct {
    void *entries[1u << 8];
} NodeTable;

typedef struct {
    uint64_t last_saved_lsn;
    void *base;
    NodeTable *root;
} RadixTree;

RadixTree* radix_tree_create();
void radix_tree_destroy(RadixTree *tree);

int radix_tree_insert(RadixTree *tree, uint64_t key, void *value);
int radix_tree_insert_offset(RadixTree *tree, uint64_t key, void *value);
int radix_tree_insert_mem_address(RadixTree *tree, uint64_t key, void *value);

void* radix_tree_lookup(RadixTree *tree, uint64_t key);
int radix_tree_delete(RadixTree *tree, uint64_t key);

uint8_t *radix_tree_serialize(RadixTree *tree, size_t *out_size);
RadixTree* radix_tree_deserialize(const uint8_t *data, size_t size);

#endif // RADIX_TREE_H
