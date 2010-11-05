#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <sys/time.h>
#include <stdarg.h>		
#include <sys/prctl.h>


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

void set_current_thread_name(char *name)
{
    /*
      Thread name must never be longer than 16 bytes. Them's the rules...
    */
    if(strlen(name) > 15)
    {
	name[15] = 0;
    }
    
    prctl(PR_SET_NAME, name,0,0,0);
}

void *allocfn_calloc_fn(void *data, size_t sz) {
  return calloc(1,sz);
}

allocfn_t allocfn_calloc = {
 allocfn_calloc_fn,
 0
};

