#ifndef IOR_H
#define IOR_H

#define IOR_TYPE_PUBLIC 0
#define IOR_TYPE_PRIVATE 1

#define IOR_STATUS_WORKING 0
#define IOR_STATUS_FINISHED 1
#define IOR_STATUS_ERROR 1

/**
   This struct should be treated as an opaque data structure - it's fields may change without notice. 
 */
typedef struct
{
    char *fn;
    size_t offset;
    ssize_t len;
    void *destination;
    int type;
    int status;
    time_t last_usage;
    int usage_count;    
    int is_whole_file;
}
    ior_t;

/**
   Initialize the library.

   mem_target is the target amount of cache, i.e. reads that are in
   memory, without users.
 */
void ior_init(size_t mem_target);
/**
   Create a new io request and load it's content asynchronously
 */
ior_t *ior_create(
    char *fn, 
    size_t offset,
    ssize_t len,
    void *destination);
int ior_status(ior_t *io);
void *ior_get_data(ior_t *io);
void ior_free(ior_t *io);

#endif
