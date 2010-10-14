#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <sys/time.h>
#include <stdarg.h>		

#include "util.h"

#ifdef HAS_WINDOWS_H
#include <windows.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



int randi( int min,
	   int max )
{
   int r = rand();
   r %= (max-min);
   return r+min;
}

static float randf_const = 1.0/RAND_MAX;

float randf()
{
   return (float)rand()*randf_const;
}

float gaussian()
{
   int i;
   float r=0.0f;
   for( i=0; i<12; i++ )
      r+=randf();
   return r-6.0f;
}



/* Linear algebra */


void subtract( float *a,
			   float *b,
			   float *r,
			   int len)
{
	int i;	
	for( i=0; i<len; i++ )
		r[i] = a[i]-b[i];
}

void add( float *a,
		  float *b,
		  float *r,
		  int len)
{
	int i;	
	for( i=0; i<len; i++ )
		r[i] = a[i]+b[i];
}

void add_s( float *a,
			float s,
			float *r,
			int len)
{
	int i;	
	for( i=0; i<len; i++ )
		r[i] = a[i]+s;
}

void multiply_s( float *a,
				 float s,
				 float *r,
				 int len)
{
	int i;	
	for( i=0; i<len; i++ )
		r[i] = a[i]*s;
}

void cross_prod( float *a, 
		 float *b,
		 float *res )
{
   res[0]=a[1]*b[2]-a[2]*b[1];
   res[1]=a[2]*b[0]-a[0]*b[2];
   res[2]=a[0]*b[1]-a[1]*b[0];
}

void copy( float *a,
		   float *r,
		   int len )
{
	int i;
	for( i=0; i<len; i++ )
    {
		r[i] = a[i];
	}
}

void vector( float *res, int len, ... )
{
	int i;
	
	va_list argp;
	va_start(argp, len);

	for( i=0; i<len; i++ )
	{
		res[i] = va_arg( argp, double );
	}
}


float calc_poly( int terms, float *a, float x )
{
	int i;
	float res;
	float x_pow = x;
	
	if( terms == 0 )
		return 0.0f;
	
	res = a[0];
	for( i=1; i<terms; i++ )
	{
		res += x_pow * a[i];
		x_pow *=x;
	}
	
	return res;
}


/* Queue functions */


void q_init( queue *q )
{
	q->start = (void **)malloc( sizeof(void*)*1 );
	q->stop = &q->start[1];
	q->put_pos = q->get_pos = q->start;
}

void q_destroy( queue *q )
{
	free( q->start );
}

/*
static q_print( queue *q )
{
	int i;
	int size = (q->stop-q->start);

	printf( "Storlek: %d\n", size );
	for( i=0; i< size; i++ )
	{
		printf( " %c%c %d: %d\n",
				&q->start[i]==q->get_pos?'g':' ', 
				&q->start[i]==q->put_pos?'p':' ', 
				i,
				q->start[i] );
	}
}

*/
static int q_realloc( queue *q )
{
	void **old_start = q->start;
	void **old_stop = q->stop;
	int diff;
	int new_size;
/*
	printf( "före\n" );
	q_print(q );
*/

	new_size = 2*(q->stop-q->start);
	
	q->start=(void**)realloc( q->start, sizeof(void*)*new_size );
	if( q->start == 0 )
	{
		q->start = old_start;
		return 0;
	}
	
	diff = q->start - old_start;
	q->get_pos += diff;
	q->put_pos += diff;
	q->stop = &q->start[new_size];
	memcpy( old_stop + diff, q->start, sizeof(void*)*(q->get_pos-q->start));
/*
	printf( "Efter:\n" );
	q_print(q );
*/
	return 1;
}

int q_put( queue *q, void *e )
{
	*q->put_pos = e;
	
	if( ++q->put_pos == q->stop )
		q->put_pos = q->start;
	if( q->put_pos == q->get_pos )
		return q_realloc( q );
	return 1;
}

void *q_get( queue *q)
{
	void *e = *q->get_pos;
	if( ++q->get_pos == q->stop )
		q->get_pos = q->start;
	return e;
}

void *q_peek( queue *q )
{
	return *q->get_pos;
}

int q_empty( queue *q )
{
	return q->put_pos == q->get_pos;
}

/* Stack functions */

void stack_init( stack *s )
{
	memset( s, 0, sizeof( stack ) );
}

void stack_destroy( stack *s )
{
	free( s->arr );
}


int stack_push( stack *s, void *e )
{
	if( s->size <= s->pos )
	{
		void **old_arr = s->arr;
		s->size = maxi( 1, s->size*2 );
		s->arr = realloc( s->arr, sizeof(void*)*s->size );
		if( s->arr == 0 )
		{
			s->arr = old_arr;
			return 0;
		}
	}
	
	s->arr[s->pos++]=e;
	//printf( "Lägg till %d till stacken på plats %d\n", e, s->pos );
	return 1;
}

void *stack_pop( stack *s )
{
	void *e = s->arr[--s->pos];
	if( s->pos*2 < s->size )
	{
		void ** old_arr = s->arr;
		int old_size = s->size;
		s->size = s->size/2;
		s->arr = realloc( s->arr, sizeof(void*)*s->size );
		if( s->arr == 0 )
		{
			s->arr = old_arr;
			s->size = old_size;
		}
	}
	return e;
}

void *stack_peek( stack *s )
{
	return s->arr[s->pos-1];
}

int stack_empty( stack *s )
{
	return s->pos == 0;
}

int stack_get_count( stack *s )
{
	return s->pos;
}

/* Hash table functions */


static void hash_create_arr( hash_table *h, int s )
{
	int i;
	
	h->arr = malloc( sizeof(hash_struct)*s );
	h->size = s;
	for( i=0; i< s; i++ )
		h->arr[i].key = 0;
	
}


void hash_init( hash_table *h, 
				int (*hash_func)(void *key),
				int (*compare_func)(void *key1, void *key2) )
{
	hash_create_arr( h, 7 );
	h->count=0;
	h->hash_func = hash_func;
	h->compare_func = compare_func;
}

void hash_destroy( hash_table *h )
{
	free( h->arr );
}


static int hash_search( hash_table *h,
						void *key )
{
	int pos = abs(h->hash_func( key )) % h->size;
	while(1)
	{
		if( (h->arr[pos].key == 0 ) || 
			( h->compare_func( h->arr[pos].key, key ) ) )
		{
			return pos;
		}
		pos++;
		pos %= h->size;
	}
}

static int hash_realloc( hash_table *h, int sz )
{
	hash_struct *old_arr = h->arr;
	int old_size = h->size;
	
	int i;

	h->arr = malloc( sizeof( hash_struct) * sz );
	if( h->arr == 0 )
	{
		h->arr = old_arr;
		return 0;
	}
	
	memset( h->arr, 0, sizeof( hash_struct) * sz );
	h->size = sz;
	
//	printf( "Ny storlek %d\n", sz );

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


int hash_put( hash_table *h, void *key, void *data )
{
	int pos;

//	printf( "Stoppar in nyckel %d, data %d\n", key, data );
	
	
	if( (float)(h->count+1)/h->size > 0.75f )
	{
		if( !hash_realloc( h, (h->size+1) * 2 -1 ) )
		{
			return 0;
		}
	}

	pos = hash_search( h, key );

/*
	printf( "Stoppar element %d (#%d)->%d på plats %d\n", 
			key, 
			h->hash_func( key )%h->size,
			data,
			pos );
*/
	if( h->arr[pos].key == 0 )
	{		
		h->count++;
	}
	
	h->arr[pos].key = key;
	h->arr[pos].data = data;
	return 1;
}

void *hash_get( hash_table *h, void *key )
{
	int pos = hash_search( h, key );
	if( h->arr[pos].key == 0 )
		return 0;
	else
		return h->arr[pos].data;
}

int hash_get_count( hash_table *h)
{
	return h->count;
}

void *hash_remove( hash_table *h, void *key )
{
	int pos = hash_search( h, key );
	int next_pos;
	
	void *result;
	if( h->arr[pos].key == 0 )
	{
/*		printf( "Element %d fanns inte på %d\n", 
				key, 
				h->hash_func( key ) % h->size );
*/	
		return 0;
	}
/*
	printf( "Tar bort element %d (#%d) från plats %d\n", 
			h->arr[pos].key,
			h->hash_func( key ) % h->size,
			pos );
*/
	h->count--;
	
	result = h->arr[pos].data;

	h->arr[pos].key = 0;
	
	next_pos = pos+1;
	next_pos %= h->size;
	
	while( h->arr[next_pos].key != 0 )
	{
		int ideal_pos = h->hash_func( h->arr[next_pos].key ) % h->size;
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

	if( (float)(h->count+1)/h->size < 0.35f )
	{
		hash_realloc( h, (h->size+1) / 2 -1 );
	}

	return result;
}

int hash_contains( hash_table *h, void *key )
{
	int pos = hash_search( h, key );
	return h->arr[pos].key != 0;
}

void hash_get_elements( hash_table *h, void ** arr )
{
	int i,pos=0;
	for( i=0; i<h->size; i++ )
	{
		if( h->arr[i].key != 0 )
		{
			arr[pos++]=h->arr[i].data;
		}
	}
//	printf( "Hittade %d av %d element\n", pos, h->count );
	
}


void pq_init( priority_queue *q,
			  int (*compare)(void *e1, void *e2) )
{
	q->arr=0;
	q->size=0;
	q->count=0;
	q->compare = compare;
}
/*
  static void pq_print( priority_queue *q )
  {
  int i;
  printf( "Size: %d\n", q->count );
  for( i=0; i<q->count; i++ )
  {
  printf( " %d\n", q->arr[i] );
  }
  
  }
*/

void pq_check( priority_queue *q, int i )
{
	int l,r;
	if( q->count <= i )
		return;
	
	l=i*2+1;
	r=i*2+2;
	
//	printf( "Kontrollerar nod %d mot barnen %d och %d\n", i, l, r );
	
	if( (q->count > l) && (q->compare(q->arr[i], q->arr[l]) < 0) )
	{
		printf( "FEL: Plats %d mindre än %d\n", i, l );
	}
	if( (q->count > r) && (q->compare(q->arr[i], q->arr[r]) < 0) )
	{
		printf( "FEL: Plats %d mindre än %d\n", i, r );
	}
	pq_check( q, l );
	pq_check( q, r );
}


int pq_put( priority_queue *q,
			 void *e )
{
	int i;
	
/*	printf( "Lägger till %d\n", e );

	printf( "före\n" );
	pq_print( q );	
*/
	if( q->size == q->count )
	{
		void **old_arr = q->arr;
		int old_size = q->size;
		q->size = maxi( 4, 2*q->size );
		q->arr = (void **)realloc( q->arr, sizeof(void*)*q->size );
		if( q->arr == 0 )
		{
			q->arr = old_arr;
			q->size = old_size;
			return 0;
		}
	}
	
	i = q->count;
	while( (i>0) && (q->compare( q->arr[(i-1)/2], e )<0 ) )
	{
		q->arr[i] = q->arr[(i-1)/2];
		i = (i-1)/2;
	}
	q->arr[i]=e;

	q->count++;

/*	printf( "efter\n" );
	pq_print( q );
*/	
/*
	pq_check(q, 0 );
*/

	return 1;
	
}

static void pq_heapify( priority_queue *q, int i )
{
	int l, r, largest;
	l = 2*(i)+1;
	r = 2*(i)+2;
	if( (l < q->count) && (q->compare(q->arr[l],q->arr[i])>0) )
	{
		largest = l;
	}
	else
	{
		largest = i;
	}
	if( (r < q->count) && (q->compare( q->arr[r],q->arr[largest])>0) )
	{
		largest = r;
	}
/*
	printf( "Största av %d %d %d är %d\n", 
			q->arr[i],
			q->arr[l],
			q->arr[r],
			q->arr[largest] );
*/
	if( largest != i )
	{
		void *tmp = q->arr[largest];
		q->arr[largest]=q->arr[i];
		q->arr[i]=tmp;
		pq_heapify( q, largest );
	}
}

void *pq_get( priority_queue *q )
{
	void *result = q->arr[0];
	q->arr[0] = q->arr[--q->count];
	pq_heapify( q, 0 );

/*	pq_check(q, 0 );	*/
//	printf( "efter get\n" );
//	pq_print( q );

	return result;
}

void *pq_peek( priority_queue *q )
{
	return q->arr[0];
}

int pq_empty( priority_queue *q )
{
	return q->count == 0;
}

int pq_get_count( priority_queue *q )
{
	return q->count;
}

void pq_destroy(  priority_queue *q )
{
	free( q->arr );
}

double get_time()
{
    struct timeval time_struct;
    gettimeofday( &time_struct, 0 );
    return (double)time_struct.tv_sec+((double)time_struct.tv_usec)/1000000.0;
}

