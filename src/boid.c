#include <stdlib.h>
#include <stdio.h>
#include "boid.h"
#include "util.h"

#define BOID_MAX_DISTANCE 3.0
#define BOID_MIN_DISTANCE 3.0

static inline int boid_dst_sq(boid_set_t *b, int i, int j)
{
    if(i==j)
	return 0;
    
    float *p1 = b->data[i].pos;
    float *p2 = b->data[j].pos;
    float diff[]=
	{
	    p1[0]-p2[0],
	    p1[1]-p2[1],
	    p1[2]-p2[2]
	}
    ;
    return (diff[0]*diff[0]+diff[1]*diff[1]+diff[2]*diff[2]);    
}


void boid_calc(boid_set_t *b, int idx, float *v1, float *v2, float *v3)
{
    int i;
    int friends=0;
	
    for(i=0;i<b->count; i++)
    {
	float dst_sq = boid_dst_sq(b, idx, i);
	if(dst_sq < (BOID_MAX_DISTANCE*BOID_MAX_DISTANCE))
	{
	    friends++;
	    
	    v1[0] += (b->data[i].pos[0] - b->data[idx].pos[0]);
	    v1[1] += (b->data[i].pos[1] - b->data[idx].pos[1]);
	    v1[2] += (b->data[i].pos[2] - b->data[idx].pos[2]);	    
	    
	    if(dst_sq < (BOID_MIN_DISTANCE * BOID_MIN_DISTANCE))
	    {
		float woot = 0.1*minf(4,(BOID_MIN_DISTANCE * BOID_MIN_DISTANCE) - dst_sq);
		v2[0] -= (b->data[i].pos[0] - b->data[idx].pos[0])*woot;
		v2[1] -= (b->data[i].pos[1] - b->data[idx].pos[1])*woot;
		v2[2] -= (b->data[i].pos[2] - b->data[idx].pos[2])*woot;
	    }
	    
	    v3[0] += b->data[i].vel[0];
	    v3[1] += b->data[i].vel[1];
	    v3[2] += b->data[i].vel[2];
	    
	}
    }
    
    if(friends == 0)
	return;

    v1[0] /= (friends);
    v1[1] /= (friends);
    v1[2] /= (friends);    

    v1[0] += (b->target[0] - b->data[idx].pos[0])/10;
    v1[1] += (b->target[1] - b->data[idx].pos[1])/10;
    v1[2] += (b->target[2] - b->data[idx].pos[2])/10;	    

    v3[0] /= (friends*18);
    v3[1] /= (friends*18);
    v3[2] /= (friends*18);    
}




void boid_step(boid_set_t *b, float dt)
{
    int i,j, from, to;
    
    for(i=0;i<b->count; i++)
    {
	
	float v1[3]={0,0,0};
	float v2[3]={0,0,0};
	float v3[3]={0,0,0};
	float vn[3];
	boid_calc(b, i, v1, v2, v3);
	add(v1,v2,vn,3);
	add(vn,v3,vn,3);
	if((vn[0]==0) && (vn[1]==0) && (vn[2]==0))
	    vn[1]=1;
	
	normalize(vn,vn,3);
	multiply_s(vn, b->speed, vn, 3);
	vn[2]*= 0.1;
	
	for(j=0; j<3;j++)
	{	    
	    b->data[i].vel[j] = 0.99*b->data[i].vel[j] +0.01*(vn[j]);
	    b->data[i].pos[j] += b->data[i].vel[j]*dt;
	}
    }

}

boid_set_t *boid_set_init(int count, float x, float y)
{
    boid_set_t *b = calloc(1,sizeof(boid_set_t)+sizeof(boid_t)*count);
    b->count=count;
    int i;
    for(i=0; i<count;i++)
    {
	b->data[i].pos[0] = x + (i%10);
	b->data[i].pos[1] = y + (i/10);
	b->data[i].pos[2] = 3;
    }
    b->speed = 20;
    
    return b;
    
}

