#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <inttypes.h>

#include "tree_view.h"
#include "tree_storage.h"
#include "utils/logging.h"
#include "utils/radix_tree.h"

// ===== Lifecycle =====

TreeView *tree_view_open(TreeStorage *storage) {
    if (!storage) return NULL;
    void *mapped = tree_storage_get_base(storage);
    if (!mapped) return NULL;
    
    TreeView *view = malloc(sizeof(TreeView));
    view->base = (uint8_t *)mapped;
    view->hdr = (struct TreeFileHeader *)mapped;

    view->idx_hdr = (RadixTree *)(tree_storage_get_index_base(storage));
    
    return view;
}

void tree_view_close(TreeView *view) {
    if (view) free(view);
}

// ===== Root access =====

NodeRef tree_root(TreeView *view) {
    if (!view || !view->hdr) {
        return (NodeRef){ NULL, OFF_NULL };
    }
    
    off64 root_off = view->hdr->root_off;
    if (root_off == OFF_NULL) {
        return (NodeRef){ NULL, OFF_NULL };
    }
    
    return (NodeRef){ view->base, root_off };
}

RadixTree* tree_view_id_map(TreeView *view) {
    if (!view || !view->idx_hdr) {
        return NULL;
    }
    return radix_tree_deserialize((const uint8_t *)view->idx_hdr, 0);
}

// ===== Navigation helpers =====

// Get NodeDisk pointer from NodeRef offset
static struct NodeDisk* get_node_disk(NodeRef ref) {
    if (!ref.base || ref.off == OFF_NULL) {
        return NULL;
    }
    return (struct NodeDisk *)(ref.base + ref.off);
}

int node_is_null(NodeRef ref) {
    return ref.base == NULL || ref.off == OFF_NULL;
}

NodeRef node_first_child(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd || nd->first_child == OFF_NULL) {
        return (NodeRef){ NULL, OFF_NULL };
    }
    return (NodeRef){ ref.base, nd->first_child };
}

NodeRef node_next_sibling(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd || nd->next_sibling == OFF_NULL) {
        return (NodeRef){ NULL, OFF_NULL };
    }
    return (NodeRef){ ref.base, nd->next_sibling };
}

NodeRef node_parent(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd || nd->parent == OFF_NULL) {
        return (NodeRef){ NULL, OFF_NULL };
    }
    return (NodeRef){ ref.base, nd->parent };
}

// ===== Data access =====

const char *node_text(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd) return "";
    return nd->text;
}

uint64_t node_id(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd) return 0;
    return nd->node_id;
}

uint64_t node_flags(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd) return 0;
    return nd->flags;
}

uint64_t node_layout_height(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd) return 0;
    return nd->layout_height;
}

uint64_t node_descendents(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (!nd) return 0;
    return nd->descendents;
}

/**
 * caller is responsible for freeing the returned NodeResource
 */
NodeResource *node_resource(NodeRef ref) {
    struct NodeDisk *nd = get_node_disk(ref);
    if (nd->ext_len != 0) {
        // Locate the start of the ext_len section
        uint8_t *ext_base = (uint8_t *)nd + sizeof(struct NodeDisk) + nd->text_len;
        // Iterate through the TLVs to find the layout TLV
        uint8_t *ptr = ext_base;
        uint8_t *end = ext_base + nd->ext_len;
        while (ptr + sizeof(ExtTLV) <= end) {
            ExtTLV *tlv = (ExtTLV *)ptr;
            if( tlv->type == EXTLV_TYPE_RESOURCE){
                NodeResource *resource = calloc(1, tlv->length);
                memcpy(resource, tlv->data, tlv->length);
                if(tlv->length != sizeof(NodeResource) + resource->length){
                    log_warn("overlay_materialize: Resource TLV length mismatch for node id=%" PRIu64, node_id(ref));
                }
                return resource;
            }
            ptr += sizeof(ExtTLV) + tlv->length;
        }
    }
    return NULL;
}