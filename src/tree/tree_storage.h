#ifndef TREE_STORAGE_H
#define TREE_STORAGE_H

#include <stddef.h>
#include <stdint.h>

/**
 * tree_storage.h - mmap/IO 
 * 
 * responsibility: how data enters and exits memory (mmap/munmap, file I/O)
 * rules: ignorance to Node/Tree，no offset operations
 */

typedef struct TreeStorage_s TreeStorage;

// Open/close storage
TreeStorage* tree_storage_open(const char *path, int readonly);
void tree_storage_close(TreeStorage *storage);

uint64_t tree_storage_get_last_saved_lsn(TreeStorage *storage);

// Get base pointer and size
void* tree_storage_get_base(TreeStorage *storage);
void* tree_storage_get_index_base(TreeStorage *storage) ;
size_t tree_storage_get_size(TreeStorage *storage);

#endif // TREE_STORAGE_H
