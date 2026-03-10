#include "tree_storage.h"
#include "tree_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../utils/logging.h"

static inline void *
mmap_slice(int fd, off_t off, size_t file_size,
           size_t *mapped_len, void **map_base)
{
    size_t page = sysconf(_SC_PAGESIZE);
    off_t map_off = off & ~(page - 1);
    size_t delta = off - map_off;
    size_t len = file_size - map_off;

    void *base = mmap(NULL, len,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE, fd, map_off);

    if (base == MAP_FAILED) return NULL;

    *map_base   = base;
    *mapped_len = len;
    return (uint8_t *)base + delta;
}

struct TreeStorage_s {
    int fd;
    char *path;
    void *base;
    void *index_mmap_base;// mmap_base is page size aligned 
    size_t index_mapped_len;// mmap_base is page size aligned 
    void *index_base;
    size_t file_size;
    uint64_t last_saved_lsn;
    int readonly;
};

uint64_t tree_storage_get_last_saved_lsn(TreeStorage *storage) {
    return storage ? storage->last_saved_lsn : 0;
}

TreeStorage* tree_storage_open(const char *path, int readonly) {
    TreeStorage *storage = calloc(1, sizeof(struct TreeStorage_s));
    if (!storage) return NULL;
    storage->path = strdup(path);

    int flags = readonly ? O_RDONLY : O_RDWR;
    storage->fd = open(storage->path, flags);
    if (storage->fd < 0) {
        free(storage->path);
        free(storage);
        return NULL;
    }

    struct stat st;
    if (fstat(storage->fd, &st) < 0) {
        close(storage->fd);
        free(storage->path);
        free(storage);
        return NULL;
    }

    storage->file_size = st.st_size;
    storage->readonly = readonly;

    int prot = readonly ? PROT_READ : (PROT_READ | PROT_WRITE);
    storage->base = mmap(NULL, storage->file_size, prot, MAP_SHARED, storage->fd, 0);
    if (storage->base == MAP_FAILED) {
        close(storage->fd);
        free(storage->path);
        free(storage);
        return NULL;
    }

    // read header, get checkpoint_lsn
    if (storage->file_size >= sizeof(struct TreeFileHeader)) {
        struct TreeFileHeader *hdr = (struct TreeFileHeader *)storage->base;
        if (hdr->magic == TREE_MAGIC) {
            storage->last_saved_lsn = hdr->checkpoint_lsn;
            log_debug("tree_storage_open: loaded checkpoint_lsn=%lu from %s", 
                     storage->last_saved_lsn, path);
            
            storage->index_base = mmap_slice(storage->fd, hdr->id_map_off, storage->file_size,
                &storage->index_mapped_len, &storage->index_mmap_base);

            if (storage->index_base == MAP_FAILED) {
                log_error("tree_storage_open: Failed to mmap index from %s", path);
                storage->index_base = NULL;
            }
        } else {
            log_warn("tree_storage_open: invalid magic in %s", path);
            storage->last_saved_lsn = 0;
        }

    } else {
        log_warn("tree_storage_open: file too small in %s", path);
        storage->last_saved_lsn = 0;
    }


    return storage;
}

void tree_storage_close(TreeStorage *storage) {
    if (!storage) return;
    if (storage->base && storage->base != MAP_FAILED) {
        munmap(storage->base, storage->file_size);
    }
    if (storage->index_base && storage->index_base != MAP_FAILED) {
        munmap(storage->index_mmap_base, storage->index_mapped_len);
    }
    if (storage->fd >= 0) {
        close(storage->fd);
    }
    free(storage->path);
    free(storage);
}

void* tree_storage_get_base(TreeStorage *storage) {
    return storage ? storage->base : NULL;
}
void* tree_storage_get_index_base(TreeStorage *storage) {
    return storage ? storage->index_base : NULL;
}

size_t tree_storage_get_size(TreeStorage *storage) {
    return storage ? storage->file_size : 0;
}
