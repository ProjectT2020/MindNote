
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "radix_tree.h"
#include "logging.h"

// Forward decl for offset decoding used before its definition
static NodeTable *radix_tree_offset_or_pointer_to_nodetable(RadixTree *tree, void *offset_or_pointer);

// Encode a NodeTable* as memory pointer (set high bit)
static inline void *radix_encode_ptr(NodeTable *ptr) {
    return (void *)((uint64_t)(uintptr_t)ptr | (1ULL << 63));
}

RadixTree* radix_tree_create(){
    RadixTree *tree = (RadixTree*)malloc(sizeof(RadixTree));
    tree->base = NULL;
    tree->root = (NodeTable*)calloc(1, sizeof(NodeTable));
    return tree;
}

void radix_tree_destroy(RadixTree *tree){
    if(!tree) return;

    // Free L6 nodes
    for(int i5 = 0; i5 < (1u << 8); i5++){
        NodeTable *l2 = radix_tree_offset_or_pointer_to_nodetable(tree, tree->root->entries[i5]);
        if(l2){
            for(int i4 = 0; i4 < (1u << 8); i4++){
                NodeTable *l3 = radix_tree_offset_or_pointer_to_nodetable(tree, l2->entries[i4]);
                if(l3){
                    for(int i3 = 0; i3 < (1u << 8); i3++){
                        NodeTable *l4 = radix_tree_offset_or_pointer_to_nodetable(tree, l3->entries[i3]);
                        if(l4){
                            for(int i2 = 0; i2 < (1u << 8); i2++){
                                NodeTable *l5 = radix_tree_offset_or_pointer_to_nodetable(tree, l4->entries[i2]);
                                if(l5){
                                    for(int i1 = 0; i1 < (1u << 8); i1++){
                                        NodeTable *l6 = radix_tree_offset_or_pointer_to_nodetable(tree, l5->entries[i1]);
                                        if(l6){
                                            free(l6);
                                        }
                                    }
                                    free(l5);
                                }
                            }
                            free(l4);
                        }
                    }
                    free(l3);
                }
            }
            free(l2);
        }
    }

    free(tree->root);
    free(tree);
}

int radix_tree_insert(RadixTree *tree, uint64_t key, void *value);
int radix_tree_insert_offset(RadixTree *tree, uint64_t key, void *value){
    return radix_tree_insert(tree, key, value);
}
int radix_tree_insert_mem_address(RadixTree *tree, uint64_t key, void *value){
    // Store pointer with high bit set to indicate MEMORY pointer, instead of file OFFSET
    value = (void *) ( (uint64_t)(uintptr_t)value | (1ULL << 63) ); // set high bit to indicate pointer
    return radix_tree_insert(tree, key, value);
}

int radix_tree_insert(RadixTree *tree, uint64_t key, void *value){
    if(!tree) return -1;
    if(key >> 48 != 0){
        log_error("radix_tree_insert: key exceeds 48 bits");
        return -1; // only support 48-bit keys
    }

    NodeTable *l1 = tree->root;
    uint8_t indices[6];
    for(int i = 0; i < 6; i++){
        indices[5 - i] = (key >> (i * 8)) & 0xFF;
    }

    // Traverse or create levels, decoding offsets when present (deserialized trees)
    NodeTable **l2_ptr = (NodeTable **)&l1->entries[indices[0]];
    NodeTable *l2 = *l2_ptr;
    if(*l2_ptr){
        l2 = radix_tree_offset_or_pointer_to_nodetable(tree, *l2_ptr);
    }else{
        l2= (NodeTable*)calloc(1, sizeof(NodeTable));
        *l2_ptr = radix_encode_ptr(l2);
    }

    NodeTable **l3_ptr = (NodeTable **)&l2->entries[indices[1]];
    NodeTable *l3 = NULL;
    if(*l3_ptr){
        l3 = radix_tree_offset_or_pointer_to_nodetable(tree, *l3_ptr);
    }else{
        l3= (NodeTable*)calloc(1, sizeof(NodeTable));
        *l3_ptr = radix_encode_ptr(l3);
    }

    NodeTable **l4_ptr = (NodeTable **)&l3->entries[indices[2]];
    NodeTable *l4 = NULL;
    if(*l4_ptr){
        l4 = radix_tree_offset_or_pointer_to_nodetable(tree, *l4_ptr);
    }else{
        l4 = (NodeTable*)calloc(1, sizeof(NodeTable));
        *l4_ptr = radix_encode_ptr(l4);
    }

    NodeTable **l5_ptr = (NodeTable **)&l4->entries[indices[3]];
    NodeTable *l5 = NULL;
    if(*l5_ptr){
        l5 = radix_tree_offset_or_pointer_to_nodetable(tree, *l5_ptr);
    }else{
        l5 = (NodeTable*)calloc(1, sizeof(NodeTable));
        *l5_ptr = radix_encode_ptr(l5);
    }

    NodeTable **l6_ptr = (NodeTable **)&l5->entries[indices[4]];
    NodeTable *l6 = NULL;
    if(*l6_ptr){
        l6 = radix_tree_offset_or_pointer_to_nodetable(tree, *l6_ptr);
    }else{
        l6= (NodeTable*)calloc(1, sizeof(NodeTable));
        *l6_ptr = radix_encode_ptr(l6);
    }
    
    l6->entries[indices[5]] = value;
    return 0;
}

// Decode stored value: high-bit set means direct pointer, otherwise treat as base+offset
static void* radix_tree_decode_value(RadixTree *tree, void *encoded){
    if(!encoded) return NULL;
    uint64_t v = (uint64_t)(uintptr_t)encoded;
    if(v & (1ULL << 63)){
        return (void*)(uintptr_t)(v & ~(1ULL << 63));
    }

    return (void*)((uint8_t*)tree->base + v) ;
}

static NodeTable *radix_tree_offset_or_pointer_to_nodetable(RadixTree *tree, void *offset_or_pointer){
    if(!offset_or_pointer) return NULL;
    uint64_t val = (uint64_t)(uintptr_t)offset_or_pointer;
    if(val & (1ULL << 63)){
        // remove high bit and return
        return (NodeTable *)(uintptr_t)(val &= ~(1ULL << 63));
    }else{
        return (NodeTable *)((uint8_t*)tree->base + val);
    }
}

void* radix_tree_lookup(RadixTree *tree, uint64_t key){
    if(!tree) return NULL;
    if(key >> 48 != 0){
        log_error("radix_tree_lookup: key exceeds 48 bits");
        return NULL; // only support 48-bit keys
    }

    NodeTable *l1 = tree->root;
    uint8_t indices[6];
    for(int i = 0; i < 6; i++){
        indices[5 - i] = (key >> (i * 8)) & 0xFF;
    }

    NodeTable *l2 = radix_tree_offset_or_pointer_to_nodetable(tree, l1->entries[indices[0]]);
    if(!l2) return NULL;

    NodeTable *l3 = radix_tree_offset_or_pointer_to_nodetable(tree, l2->entries[indices[1]]);
    if(!l3) return NULL;

    NodeTable *l4 = radix_tree_offset_or_pointer_to_nodetable(tree, l3->entries[indices[2]]);
    if(!l4) return NULL;

    NodeTable *l5 = radix_tree_offset_or_pointer_to_nodetable(tree, l4->entries[indices[3]]);
    if(!l5) return NULL;

    NodeTable *l6 = radix_tree_offset_or_pointer_to_nodetable(tree, l5->entries[indices[4]]);
    if(!l6) return NULL;

    return l6->entries[indices[5]];
}

int radix_tree_delete(RadixTree *tree, uint64_t key) {
    if(!tree) return -1;
    if(key >> 48 != 0){
        log_error("radix_tree_delete: key exceeds 48 bits");
        return -1; // only support 48-bit keys
    }

    NodeTable *l1 = tree->root;
    uint8_t indices[6];
    for(int i = 0; i < 6; i++){
        indices[5 - i] = (key >> (i * 8)) & 0xFF;
    }

    NodeTable *l2 = radix_tree_offset_or_pointer_to_nodetable(tree, l1->entries[indices[0]]);
    if(!l2) return -1;

    NodeTable *l3 = radix_tree_offset_or_pointer_to_nodetable(tree, l2->entries[indices[1]]);
    if(!l3) return -1;

    NodeTable *l4 = radix_tree_offset_or_pointer_to_nodetable(tree, l3->entries[indices[2]]);
    if(!l4) return -1;

    NodeTable *l5 = radix_tree_offset_or_pointer_to_nodetable(tree, l4->entries[indices[3]]);
    if(!l5) return -1;

    NodeTable *l6 = radix_tree_offset_or_pointer_to_nodetable(tree, l5->entries[indices[4]]);
    if(!l6) return -1;

    l6->entries[indices[5]] = NULL;
    return 0;
}


uint8_t *radix_tree_serialize(RadixTree *tree, size_t *out_size){
    if(!tree || !tree->root) return NULL;

    size_t capacity = sizeof(RadixTree);
    uint8_t *data = (uint8_t *)malloc(capacity);
    if(!data) return NULL;

    size_t offset = 0;
    memcpy(data, tree, sizeof(RadixTree));
    offset += sizeof(RadixTree);

    RadixTree *hdr = (RadixTree *)data;
    hdr->base = 0; // will be set on deserialize

    // helper to grow buffer
    #define ENSURE_CAP(add) do { \
        if(offset + (add) > capacity){ \
            size_t new_cap = capacity * 2; \
            while(new_cap < offset + (add)) new_cap *= 2; \
            uint8_t *tmp = realloc(data, new_cap); \
            if(!tmp) { free(data); return NULL; } \
            data = tmp; \
            hdr = (RadixTree *)data; \
            capacity = new_cap; \
        } \
    } while(0)

    // BFS-style copy using offsets
    NodeTable *root_src = tree->root;
    ENSURE_CAP(sizeof(NodeTable));
    size_t root_off = offset;
    memcpy(data + offset, root_src, sizeof(NodeTable));
    offset += sizeof(NodeTable);
    hdr->root = (NodeTable*)(uintptr_t)root_off; // store offset

    // iterative traversal over fixed-depth radix (6 levels)
    for(int i5 = 0; i5 < (1u << 8); i5++){
        NodeTable *l2_src = radix_tree_offset_or_pointer_to_nodetable(tree, root_src->entries[i5]);
        if(!l2_src) continue;
        ENSURE_CAP(sizeof(NodeTable));
        size_t l2_off = offset;
        memcpy(data + offset, l2_src, sizeof(NodeTable));
        offset += sizeof(NodeTable);
        ((NodeTable*)(data + root_off))->entries[i5] = (void*)(uintptr_t)l2_off;

        for(int i4 = 0; i4 < (1u << 8); i4++){
            NodeTable *l3_src = radix_tree_offset_or_pointer_to_nodetable(tree, l2_src->entries[i4]);
            if(!l3_src) continue;
            ENSURE_CAP(sizeof(NodeTable));
            size_t l3_off = offset;
            memcpy(data + offset, l3_src, sizeof(NodeTable));
            offset += sizeof(NodeTable);
            ((NodeTable*)(data + l2_off))->entries[i4] = (void*)(uintptr_t)l3_off;

            for(int i3 = 0; i3 < (1u << 8); i3++){
                NodeTable *l4_src = radix_tree_offset_or_pointer_to_nodetable(tree, l3_src->entries[i3]);
                if(!l4_src) continue;
                ENSURE_CAP(sizeof(NodeTable));
                size_t l4_off = offset;
                memcpy(data + offset, l4_src, sizeof(NodeTable));
                offset += sizeof(NodeTable);
                ((NodeTable*)(data + l3_off))->entries[i3] = (void*)(uintptr_t)l4_off;

                for(int i2 = 0; i2 < (1u << 8); i2++){
                    NodeTable *l5_src = radix_tree_offset_or_pointer_to_nodetable(tree, l4_src->entries[i2]);
                    if(!l5_src) continue;
                    ENSURE_CAP(sizeof(NodeTable));
                    size_t l5_off = offset;
                    memcpy(data + offset, l5_src, sizeof(NodeTable));
                    offset += sizeof(NodeTable);
                    ((NodeTable*)(data + l4_off))->entries[i2] = (void*)(uintptr_t)l5_off;

                    for(int i1 = 0; i1 < (1u << 8); i1++){
                        NodeTable *l6_src = radix_tree_offset_or_pointer_to_nodetable(tree, l5_src->entries[i1]);
                        if(!l6_src) continue;
                        ENSURE_CAP(sizeof(NodeTable));
                        size_t l6_off = offset;
                        memcpy(data + offset, l6_src, sizeof(NodeTable));
                        offset += sizeof(NodeTable);
                        ((NodeTable*)(data + l5_off))->entries[i1] = (void*)(uintptr_t)l6_off;
                    }
                }
            }
        }
    }

    #undef ENSURE_CAP

    if(out_size) *out_size = offset;
    return data;
}

RadixTree* radix_tree_deserialize(const uint8_t *data, size_t size){
    if(!data || (size && size < sizeof(RadixTree))) return NULL;

    RadixTree *tree = (RadixTree *)data;

    tree->base = (void*)data;
    tree->root = (NodeTable *)((uint8_t*)tree->base + (uintptr_t)tree->root);

    return tree;
}