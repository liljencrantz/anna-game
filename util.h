/* 
Generic utilities library
*/

/* Structures for datatypes. They should all be treated as opaque
 * datastructures. */

#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <string.h>

typedef struct
{
	void **start, **stop, **put_pos, **get_pos;
}
queue;


typedef struct
{
	void **arr;
	int pos, size;
}
stack;


typedef struct
{
	void **arr;
	int count, size;
	int (*compare)(void *e1, void *e2);
}
priority_queue;

typedef struct
{
  void *(*fn)(void *data, size_t);
  void *data;
}
allocfn_t;

/* Linear algebra functions (in R3) */
static inline float minf( float a,
	      float b )
{
	return a<b?a:b;
}


static inline float maxf( float a,
	      float b )
{
	return a>b?a:b;
}

static inline int mini( int a,
	  int b )
{
	return a<b?a:b;
}


static inline int maxi( int a, 
	  int b )
{
	return a>b?a:b;
}

static inline float dot_prod( float *a,
		float *b,
		int len)
{
	int i;
	float res=0;
	for( i=0; i<len; i++ )
		res += a[i]*b[i];
	return res;
}

void cross_prod( float *a, float *b, float *res );

static inline void normalize( float *a, float *res, int len )
{
    int i;
    float inv_len = 1.0f/sqrtf(dot_prod( a, a, len ) );
    for( i=0; i<len; i++ )
    {
	res[i] = inv_len*a[i];
    }
}

static inline void rotate_z(float *v, float a)
{
    float f1 = cos(a);
    float f2 = sin(a);
    float tmp = v[0]*f1 - v[1]*f2;
    v[1] = v[0]*f2 + v[1]*f1;
    v[0] = tmp;
}

static inline void rotate_y(float *v, float a)
{
    float f1 = cos(a);
    float f2 = sin(a);
    float tmp = v[0]*f1 + v[2]*f2;
    v[2] = -v[0]*f2 + v[2]*f1;
    v[0] = tmp;
}

static inline void rotate_x(float *v, float a)
{
    float f1 = cos(a);
    float f2 = sin(a);
    float tmp = v[1]*f1 - v[2]*f2;
    v[2] = v[1]*f2 + v[2]*f1;
    v[1] = tmp;
}

void subtract( float *a, float *b, float *res, int len );
void add( float *a, float *b, float *res, int len );
void copy( float *a, float *res, int len );
void multiply_s( float *a, float b, float *r, int len );
void add_s( float *a, float b, float *r, int len );
/* puts the elements a,b and c into the vector res */
void vector( float *res, int len, ... );

/* Calculate a polynominal */
float calc_poly( int terms, float *a, float x );

/* 
 * All the datastuctures below autoresize. The queue, stack and
 * priorityqueue are all impemented using an array and are guaranteed
 * to never be less than 50% full. 
*/

/* Queue functions */
void q_init( queue *q );
void q_destroy( queue *q );
int q_put( queue *q, void *e );
void *q_get( queue *q);
void *q_peek( queue *q);
int q_empty( queue *q );

/* Stack functions */
void stack_init( stack *s );
void stack_destroy( stack *s );
int stack_push( stack *s, void *e );
void *stack_pop( stack *s );
void *stack_peek( stack *s );
int stack_empty();
int stack_get_count( stack *s );

/* Priority queue */
void pq_init( priority_queue *q,
			  int (*compare)(void *e1, void *e2) );
int pq_put( priority_queue *q,
			 void *e );
void *pq_get( priority_queue *q );
void *pq_peek( priority_queue *q );
int pq_empty( priority_queue *q );
int pq_get_count( priority_queue *q );
void pq_destroy(  priority_queue *q );

/* Returns the number of microseconds since January the 1:st, 1970 */
double get_time();

void set_current_thread_name(char *name);

/*
  An allocfn wrapper for calloc. This is here to make it reasonably
  simple to just use libc memory allocation for anna objects that
  might either want that or be allocated in Lua.
 */
extern allocfn_t allocfn_calloc;

#endif
