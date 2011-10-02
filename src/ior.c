#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

#include "util.h"
#include "ior.h"

static queue_t ior_q;
static array_list_t ior_done;
static pthread_mutex_t ior_q_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ior_q_convar = PTHREAD_COND_INITIALIZER;
static size_t ior_mem_target=0;
static size_t ior_mem_used=0;
static pthread_t ior_thread;

static inline void ior_mem_check()
{
    //return;
    pthread_mutex_lock(&ior_q_mutex);
    
//    printf("Check memory\n");
    if(ior_mem_used > ior_mem_target)
    {
//	printf("Out of memory, using %d bytes, target is %d\n", ior_mem_used, ior_mem_target);
	
	int i=0;
	int count = al_get_count(&ior_done);
	while(i < count)
	{
	    ior_t *io = (ior_t *)al_get(&ior_done, i);
	    
	    if(io->usage_count == 0)
	    {
		ior_mem_used -= io->len;
		al_set(&ior_done, i, al_get(&ior_done, count-1));
		count--;
		al_truncate(&ior_done, count);
//		printf("Drop request %d on file %s, size is %d\n", io, io->fn, io->len);
		free(io->destination);
		free(io);
//		printf("Dropped\n");
		continue;
	    }
	    i++;
	}
//	printf("After purge, we're using %d bytes, target is %d\n", ior_mem_used, ior_mem_target);
	
    }
    pthread_mutex_unlock(&ior_q_mutex);
    
}

static inline int ior_thread_handle(ior_t *p)
{
    
    if(p->len == -1)
    {
	struct stat sb;
	
	if (stat(p->fn, &sb) == -1) {
	    return IOR_STATUS_ERROR;
	}
	p->len = sb.st_size - p->offset;
    }
    if(p->destination == 0)
    {
	p->destination = malloc(p->len);
	if(!p->destination)
	{
	    return IOR_STATUS_ERROR;
	}
	ior_mem_check();
    }
    int fd = open(p->fn, O_RDONLY);
    if(fd == -1)
    {
	return IOR_STATUS_ERROR;
    }

    int status = IOR_STATUS_ERROR;
    if(lseek(fd, p->offset, SEEK_SET) == p->offset)
    {
	size_t done=0;
	while(1)
	{
	    ssize_t dd = read(fd, p->destination+done, p->len);
//	    printf("Read %d bytes of %s\n", dd, p->fn);
	    
	    if(dd == -1)
	    {
		break;
	    }
	    done += dd;
	    if(done == p->len)
	    {
		status = IOR_STATUS_FINISHED;
		break;
	    }
	    if(!dd)
	    {
		break;
	    }
	}
    }
    
    close(fd);
    return status;
    
}

static void *ior_thread_runner(void *arg)
{
    pthread_mutex_lock(&ior_q_mutex);
    while(1)
    {
	if(!q_empty(&ior_q))
	{
	    ior_t *p = (ior_t *)q_get(&ior_q);
	    pthread_mutex_unlock(&ior_q_mutex);
	    int status = ior_thread_handle(p);
	    pthread_mutex_lock(&ior_q_mutex);
	    p->status = status;
	    if(p->status == IOR_STATUS_FINISHED && 
	       p->type == IOR_TYPE_PUBLIC)
		al_push(&ior_done, p);
	}
	else 
	{
	    pthread_cond_wait(&ior_q_convar, &ior_q_mutex);	    
	}
    }
    pthread_mutex_unlock(&ior_q_mutex);
}

void ior_init(size_t mem_target)
{
    ior_mem_target = mem_target;
    q_init(&ior_q); 
    al_init(&ior_done);
    
    int rc = pthread_create(&ior_thread, 0, ior_thread_runner, 0);
    if(rc)
    {
	printf("ERROR; return code from pthread_create() is %d\n", rc);
	exit(1);
    }
}

static inline ior_t *ior_get_cache(
    char *fn, 
    size_t offset, 
    ssize_t len)
{
    int i;
    int count = al_get_count(&ior_done);
//    printf("Search cache for %s %d %d\n", fn, offset, len);
    
    for(i=0; i<count; i++)
    {
	ior_t *p = (ior_t *)al_get(&ior_done, i);
//	printf("Compare %s %d %d\n", p->fn, p->offset, p->len);
	if (
	    offset == p->offset &&
	    (
		(len == (ssize_t)-1 && p->is_whole_file) ||
		(len == p->len)) &&
	    strcmp(fn, p->fn)==0 )
	{
//	    printf("Found match in request %dn", p);
	    return p;
	}
    }
    return 0;
}

ior_t *ior_create(
    char *fn, 
    size_t offset, 
    ssize_t len,
    void *destination )
{
    pthread_mutex_lock(&ior_q_mutex);
    ior_t *res = ior_get_cache(fn, offset, len);
    if(res)
    {
	if(res->usage_count==0)	
	    ior_mem_used -= res->len;
	res->usage_count++;	
    }
    else
    {
	//printf("AAA\n");
	
	res = malloc(sizeof(ior_t));
	res->fn = fn;
	res->offset=offset;
	res->len = len;
	res->status = IOR_STATUS_WORKING;
	res->destination=destination;
	res->type = destination ? IOR_TYPE_PRIVATE : IOR_TYPE_PUBLIC;
	res->usage_count = 1;	
	res->is_whole_file=(len == -1);	

	q_put(&ior_q, res);
	pthread_cond_signal(&ior_q_convar);
    }
    
    pthread_mutex_unlock(&ior_q_mutex);
    return res;
}

int ior_status(ior_t *io)
{
    pthread_mutex_lock(&ior_q_mutex);
    int status = io->status;
    pthread_mutex_unlock(&ior_q_mutex);
    return status;
}

void *ior_get_data(ior_t *io)
{
    return io->destination;
}

void ior_free(ior_t *io)
{
//    printf("a LALALA\n");

    io->usage_count--;
    if(io->type == IOR_TYPE_PRIVATE) 
    {
	
	free(io);    
    }
    else if(io->usage_count == 0)
    {
//	printf("b LALALA\n");
	ior_mem_used += io->len;
    }

//    printf("c LALALA\n");
}


