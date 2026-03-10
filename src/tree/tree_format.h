#pragma once
#include <stdint.h>

#define TREE_MAGIC 0x5445524546494C45ULL  // "TREEFILE"
#define TREE_VERSION 1

typedef uint64_t off64;
#define OFF_NULL ((off64)0)

typedef struct TreeFileHeader {
    uint64_t magic;
    uint64_t version;
    off64    root_off;
    uint64_t checkpoint_lsn;
    uint64_t max_node_id;
    uint64_t node_count;
    off64   id_map_off;
} TreeFileHeader;

typedef struct NodeDisk {
    uint64_t node_id;

    off64 parent;
    off64 first_child;
    off64 next_sibling;

    uint64_t layout_height;
    uint64_t descendents;

    uint16_t type;
    uint64_t flags;
    uint32_t text_len;
    uint32_t ext_len;
    char text[];  // Flexible array member (C99)
} NodeDisk;

typedef enum {
    EXTLV_TYPE_NONE     = 0,
    EXTLV_TYPE_RESOURCE = 1,
} ExtTLVType;

typedef struct ExtTLV {
    uint16_t type;
    uint16_t flags;
    uint32_t length;
    char data[];
} ExtTLV;

typedef struct NodeResource{
    uint32_t length;
    char text[];
} NodeResource;