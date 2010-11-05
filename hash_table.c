#include <stdlib.h>
#include <stdio.h>

#include "hash_table.h"

/**
   Minimum size for hash tables
*/
#define HASH_MIN_SIZE 7

/* Hash table functions */

void oom_handler(void *pos)
{
    printf("Out of memory\n");
    exit(1);
}

void hash_init2( hash_table_t *h,
				int (*hash_func)(void *key),
				 int (*compare_func)(void *key1, void *key2),
				 size_t capacity)
{
	int i;
	size_t sz = 32;
	while( sz < (capacity*4/3) )
		sz*=2;
	/*
	  Make sure the size is a Mersenne number. Should hopfully be a
	  reasonably good size with regard to avoiding patterns of collisions.
	*/
	sz--;
	

	h->arr = malloc( sizeof(hash_struct_t)*sz );
	if( !h->arr )
	{
		oom_handler( h );
		return;
	}
	
	h->size = sz;
	for( i=0; i< sz; i++ )
		h->arr[i].key = 0;
	h->count=0;
	h->hash_func = hash_func;
	h->compare_func = compare_func;
	h->cache=-1;
}

void hash_init( hash_table_t *h,
				int (*hash_func)(void *key),
				int (*compare_func)(void *key1, void *key2) )
{
	h->arr = 0;
	h->size = 0;
	h->count=0;
	h->hash_func = hash_func;
	h->compare_func = compare_func;
	h->cache=-1;
}


void hash_destroy( hash_table_t *h )
{
	free( h->arr );
}

/**
   Search for the specified hash key in the table
   \return index in the table, or to the first free index if the key is not in the table
*/
static int hash_search( hash_table_t *h,
						void *key )
{
	int hv;
	int pos;

	if( h->cache>=0 && h->arr[h->cache].key)
	{
		if( h->compare_func( h->arr[h->cache].key, key ) )
		{
			return h->cache;
		}
	}

	hv = h->hash_func( key );
	pos = (hv & 0x7fffffff) % h->size;
	while(1)
	{
		if( (h->arr[pos].key == 0 ) ||
			( h->compare_func( h->arr[pos].key, key ) ) )
		{
			h->cache = pos;
			return pos;
		}
		pos++;
		pos %= h->size;
	}
}

/**
   Reallocate the hash array. This is quite expensive, as every single entry has to be rehashed and moved.
*/
static int hash_realloc( hash_table_t *h,
						 int sz )
{

	/* Avoid reallocating when using pathetically small tables */
	if( ( sz < h->size ) && (h->size < HASH_MIN_SIZE))
		return 1;
	sz = maxi( sz, HASH_MIN_SIZE );
	
	hash_struct_t *old_arr = h->arr;
	int old_size = h->size;
	
	int i;

	h->cache = -1;
	h->arr = malloc( sizeof( hash_struct_t) * sz );
	if( h->arr == 0 )
	{
		h->arr = old_arr;
		oom_handler( h );
		return 0;
	}

	memset( h->arr,
			0,
			sizeof( hash_struct_t) * sz );
	h->size = sz;

	for( i=0; i<old_size; i++ )
	{
		if( old_arr[i].key != 0 )
		{
			int pos = hash_search( h, old_arr[i].key );
			h->arr[pos].key = old_arr[i].key;
			h->arr[pos].data = old_arr[i].data;
		}
	}
	free( old_arr );

	return 1;
}


int hash_put( hash_table_t *h,
			  const void *key,
			  const void *data )
{
	int pos;
	
	if( (float)(h->count+1)/h->size > 0.75f )
	{
		if( !hash_realloc( h, (h->size+1) * 2 -1 ) )
		{
			return 0;
		}
	}

	pos = hash_search( h, (void *)key );

	if( h->arr[pos].key == 0 )
	{
		h->count++;
	}

	h->arr[pos].key = (void *)key;
	h->arr[pos].data = (void *)data;
	return 1;
}

void *hash_get( hash_table_t *h,
				const void *key )
{
	if( !h->count )
		return 0;
	
	int pos = hash_search( h, (void *)key );	
	if( h->arr[pos].key == 0 )
	{
		return 0;
	}
	else
	{
		void *res =h->arr[pos].data;
		return res;
	}
}

void *hash_get_key( hash_table_t *h,
					const void *key )
{	
	if( !h->count )
		return 0;
	
	int pos = hash_search( h, (void *)key );
	if( h->arr[pos].key == 0 )
		return 0;
	else
		return h->arr[pos].key;
}

int hash_get_count( hash_table_t *h)
{
	return h->count;
}

void hash_remove( hash_table_t *h,
				  const void *key,
				  void **old_key,
				  void **old_val )
{
	if( !h->count )
	{

		if( old_key != 0 )
			*old_key = 0;
		if( old_val != 0 )
			*old_val = 0;
		return;
	}

	int pos = hash_search( h, (void *)key );
	int next_pos;

	if( h->arr[pos].key == 0 )
	{

		if( old_key != 0 )
			*old_key = 0;
		if( old_val != 0 )
			*old_val = 0;
		return;
	}

	h->count--;

	if( old_key != 0 )
		*old_key = h->arr[pos].key;
	if( old_val != 0 )
		*old_val = h->arr[pos].data;

	h->arr[pos].key = 0;

	next_pos = pos+1;
	next_pos %= h->size;

	while( h->arr[next_pos].key != 0 )
	{

		int hv = h->hash_func( h->arr[next_pos].key );
		int ideal_pos = ( hv  & 0x7fffffff) % h->size;
		int dist_old = (next_pos - ideal_pos + h->size)%h->size;
		int dist_new = (pos - ideal_pos + h->size)%h->size;
		if ( dist_new < dist_old )
		{
			h->arr[pos].key = h->arr[next_pos].key;
			h->arr[pos].data = h->arr[next_pos].data;
			h->arr[next_pos].key = 0;
			pos = next_pos;
		}
		next_pos++;

		next_pos %= h->size;

	}

	if( (float)(h->count+1)/h->size < 0.2f && h->count < 63 )
	{
		hash_realloc( h, (h->size+1) / 2 -1 );
	}

	return;
}

int hash_contains( hash_table_t *h,
				   const void *key )
{
	if( !h->count )
		return 0;
	
	int pos = hash_search( h, (void *)key );
	return h->arr[pos].key != 0;
}

/**
   Push hash value into array_list_t
*/
/*
static void hash_put_data( void *key,
						   void *data,
						   void *al )
{
	al_push( (array_list_t *)al,
			 data );
}


void hash_get_data( hash_table_t *h,
					array_list_t *arr )
{
	hash_foreach2( h, &hash_put_data, arr );
}

static void hash_put_key( void *key, void *data, void *al )
{
	al_push( (array_list_t *)al, key );
}


void hash_get_keys( hash_table_t *h,
					array_list_t *arr )
{
	hash_foreach2( h, &hash_put_key, arr );
}
*/
void hash_foreach( hash_table_t *h,
				   void (*func)( void *, void *) )
{
	int i;
	for( i=0; i<h->size; i++ )
	{
		if( h->arr[i].key != 0 )
		{
			func( h->arr[i].key, h->arr[i].data );
		}
	}
}

void hash_foreach2( hash_table_t *h,
					void (*func)( void *, void *, void * ),
					void *aux )
{
	int i;
	for( i=0; i<h->size; i++ )
	{
		if( h->arr[i].key != 0 )
		{
			func( h->arr[i].key, h->arr[i].data, aux );
		}
	}
}


/**
   Helper function for hash_wcs_func
*/
static unsigned int rotl1( unsigned int in )
{
	return (in<<1|in>>31);
}

/**
   Helper function for hash_wcs_func
*/
static unsigned int rotl5( unsigned int in )
{
	return (in<<5|in>>27);
}

/**
   Helper function for hash_wcs_func
*/
static unsigned int rotl30( unsigned int in )
{
	return (in<<30|in>>2);
}

/**
   The number of words of input used in each lap by the sha-like
   string hashing algorithm. 
*/
#define WORD_COUNT 16

int hash_wcs_func( void *data )
{
	const wchar_t *in = (const wchar_t *)data;
	unsigned int a,b,c,d,e;
	int t;
	unsigned int k0=0x5a827999u;	
	unsigned int k1 =0x6ed9eba1u;
	
	unsigned int w[2*WORD_COUNT];	
	
	/*
	  Same constants used by sha1
	*/
	a=0x67452301u;
	b=0xefcdab89u;
	c=0x98badcfeu;
	d=0x10325476u;
	e=0xc3d2e1f0u;
	
	if( data == 0 )
		return 0;
	
	while( *in )
	{
		int i;

		/*
		  Read WORD_COUNT words of data into w
		*/
		for( i=0; i<WORD_COUNT; i++ )
		{
			if( !*in)
			{
				/*
				  We have reached EOF, fill in the rest with zeroes
				*/
				for( ;i<WORD_COUNT; i++ )
					w[i]=0;
			}
			else
				w[i]=*in++;
			
		}
		
		/*
		  And fill up the rest by rotating the previous content
		*/
		for( i=WORD_COUNT; i<(2*WORD_COUNT); i++ )
		{
			w[i]=rotl1(w[i-1]^w[i-(WORD_COUNT/2)]^w[i-(WORD_COUNT/2-1)]^w[i-WORD_COUNT]);
		}

		/*
		  Only 2*WORD_COUNT laps, not 80 like in sha1. Only two types
		  of laps, not 4 like in sha1
		*/
		for( t=0; t<WORD_COUNT; t++ )
		{
			unsigned int temp;
			temp = (rotl5(a)+(b^c^d)+e+w[t]+k0);
			e=d;
			d=c;
			c=rotl30(b);
			b=a;
			a=temp;
		}
		for( t=WORD_COUNT; t<(2*WORD_COUNT); t++ )
		{
			unsigned int temp;
			temp = (rotl5(a)+((b&c)|(b&d)|(c&d))+e+w[t]+k1);
			e=d;
			d=c;
			c=rotl30(b);
			b=a;
			a=temp;
		}
	}

	/*
	  Implode from 160 to 32 bit hash and return
	*/
	return a^b^c^d^e;
}

int hash_wcs_cmp( void *a, void *b )
{
	return wcscmp((wchar_t *)a,(wchar_t *)b) == 0;
}

int hash_str_cmp( void *a, void *b )
{
	return strcmp((char *)a,(char *)b) == 0;
}

int hash_str_func( void *data )
{
	int res = 0x67452301u;
	const char *str = data;	

	while( *str )
		res = (18499*rotl5(res)) ^ *str++;
	
	return res;
}

int hash_ptr_func( void *data )
{
	return (int)(long) data;
}

/**
   Hash comparison function suitable for direct pointer comparison
*/
int hash_ptr_cmp( void *a,
                  void *b )
{
	return a == b;
}

