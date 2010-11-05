#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "util.h"

/*
 * The hashtable is implemented using
 * a single hashfunction and element storage directly in the array. It
 * is guaranteed to never be more than 75% full or less than 35% full.
 */
typedef struct
{
	void *key, *data;
}
hash_struct_t;

/**
   Data structure for the hash table implementaion. A hash table allows for
   retrieval and removal of any element in O(1), so long as a proper
   hash function is supplied.

   The hash table is implemented using a single hash function and
   element storage directly in the array. When a collision occurs, the
   hashtable iterates until a zero element is found. When the table is
   75% full, it will automatically reallocate itself. This
   reallocation takes O(n) time. The table is guaranteed to never be
   more than 75% full or less than 30% full (Unless the table is
   nearly empty). Its size is always a Mersenne number.

*/

typedef struct hash_table
{
	/** The array containing the data */
	hash_struct_t *arr;
	/** A simple one item cache. This should always point to the index of the last item to be used */
	int cache;	
	/** Number of elements */
	int count;
	/** Length of array */
	int size;
	/** Hash function */
	int (*hash_func)( void *key );
	/** Comparison function */
	int (*compare_func)( void *key1, void *key2 );
}
	hash_table_t;


/**
   Initialize a hash table. The hash function must never return the value 0.
*/
void hash_init( hash_table_t *h,
				int (*hash_func)( void *key),
				int (*compare_func)( void *key1, void *key2 ) );

/**
   Initialize a hash table. The hash function must never return the value 0.
*/
void hash_init2( hash_table_t *h,
				int (*hash_func)( void *key ),
				 int (*compare_func)( void *key1, void *key2 ),
				 size_t capacity);

/**
   Destroy the hash table and free associated memory.
*/
void hash_destroy( hash_table_t *h );
/**
   Set the key/value pair for the hashtable. 
*/
int hash_put( hash_table_t *h, 
			  const void *key,
			  const void *data );
/**
   Returns the data with the associated key, or 0 if no such key is in the hashtable
*/
void *hash_get( hash_table_t *h,
				const void *key );
/**
   Returns the hash tables version of the specified key
*/
void *hash_get_key( hash_table_t *h, 
					const void *key );

/**
   Returns the number of key/data pairs in the table.
*/
int hash_get_count( hash_table_t *h);
/**
   Remove the specified key from the hash table if it exists. Do nothing if it does not exist.

   \param h The hashtable
   \param key The key
   \param old_key If not 0, a pointer to the old key will be stored at the specified address
   \param old_data If not 0, a pointer to the data will be stored at the specified address
*/
void hash_remove( hash_table_t *h, 
				  const void *key, 
				  void **old_key,
				  void **old_data );

/**
   Checks whether the specified key is in the hash table
*/
int hash_contains( hash_table_t *h, 
				   const void *key );

/**
   Appends all keys in the table to the specified list
*/
//void hash_get_keys( hash_table_t *h,
//					array_list_t *arr );

/**
   Appends all data elements in the table to the specified list
*/
//void hash_get_data( hash_table_t *h,
//					array_list_t *arr );

/**
   Call the function func for each key/data pair in the table
*/
void hash_foreach( hash_table_t *h, 
				   void (*func)( void *, void * ) );

/**
   Same as hash_foreach, but the function func takes an additional
   argument, which is provided by the caller in the variable aux 
*/
void hash_foreach2( hash_table_t *h, void (*func)( void *, 
												   void *, 
												   void *), 
					void *aux );

/**
   Hash function suitable for character strings. 
*/
int hash_str_func( void *data );
/**
   Hash comparison function suitable for character strings
*/
int hash_str_cmp( void *a,
				  void *b );

/**
   Hash function suitable for wide character strings. Uses a version
   of the sha cryptographic function which is simplified in order to
   returns a 32-bit number.
*/
int hash_wcs_func( void *data );

/**
   Hash comparison function suitable for wide character strings
*/
int hash_wcs_cmp( void *a, 
				  void *b );

/**
   Hash function suitable for direct pointer comparison
*/
int hash_ptr_func( void *data );


/**
   Hash comparison function suitable for direct pointer comparison
*/
int hash_ptr_cmp( void *a,
                  void *b );



#endif
