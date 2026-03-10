#pragma once
#include "tree_format.h"
#include "tree_storage.h"
#include "utils/radix_tree.h"

typedef struct {
    uint8_t *base;
    off64    off;
} NodeRef;

typedef struct {
    uint8_t *base;
    struct TreeFileHeader *hdr;
    RadixTree *idx_hdr;
} TreeView;

/* lifecycle */
TreeView *tree_view_open(TreeStorage *);
void tree_view_close(TreeView *);

/* root */
NodeRef tree_root(TreeView *);
/* index */
RadixTree* tree_view_id_map(TreeView *view) ;

/* navigation */
int     node_is_null(NodeRef);
NodeRef node_first_child(NodeRef);
NodeRef node_next_sibling(NodeRef);
NodeRef node_parent(NodeRef);

/* data access */
const char *node_text(NodeRef);
uint64_t node_id(NodeRef);
uint64_t node_flags(NodeRef);
uint64_t node_layout_height(NodeRef);
uint64_t node_descendents(NodeRef);

NodeResource *node_resource(NodeRef ref) ;