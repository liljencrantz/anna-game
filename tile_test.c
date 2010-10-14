

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "tile.h"

#define TWIDDLE(a,b,c) (((a) ^ (((b)>>5) | ((b)<<27)) ^ (((c)>>7) | ((c)<<25))))

int main()
{
    printf("Size of a single tile: %.2f kB\n", 1.0*sizeof(tile_t)/1024);
    
/*
    int i,j;
    for(i=0; i<10; i++)
    {
	printf("Tile with %d levels has %d nodes and %d heightmap elements.\n", i, TILE_NODE_COUNT(i), TILE_HM_COUNT(i));	
    }
    for(j=0; j<9; j++)
    {
	printf("Node level %d begins at offset %d and heightmap elements begin at offset %d.\n", j, TILE_NODE_OFFSET(j), TILE_HM_OFFSET(j));
    }
    
*/
    tile_t *t = calloc(1, sizeof(tile_t));
    nid_t nid = 
	{
	    5,49
	}
    ;
    int i, j, k;

    for(i=0; i<TILE_SUBTILE_COUNT; i++)
    {
	t->subtile[i] = calloc(1, sizeof(tile_t));
    }
    
    for(i=0;i<TILE_LEVELS; i++)
    {
	int node_side_len = 1<<i;
	int hm_side_len = 2<<i;
	for(j=0; j<node_side_len; j++)
	{
	    for(k=0; k<node_side_len; k++)
	    {
		int idx = TILE_NODE_OFFSET(i) + j + node_side_len*k;
		int twid= TWIDDLE(i,j,k);
		printf(
		    "Twiddling %d %d %d, with index %d has value %d\n",
		    i, j, k, idx, twid);
		t->node[idx].distortion = twid;
		
	    }
	}
	for(j=0; j<hm_side_len; j++)
	{
	    for(k=0; k<hm_side_len; k++)
	    {
		t->hm[TILE_HM_OFFSET(i) + j + hm_side_len*k].height = TWIDDLE(i,j,k);
	    }
	}
    }
    
    for(i=0;i<TILE_LEVELS; i++)
    {
	int node_side_len = 1<<i;
	int hm_side_len = 2<<i;

	for(j=0; j<node_side_len; j++)
	{
	    for(k=0; k<node_side_len; k++)
	    {
		nid_t nid;
		NID_SET(nid, i, j, k);
		
		t_node_t *n = tile_nid_lookup(t, nid);
		int val = TWIDDLE(i,j,k);
		int idx = n - t->node;
		
		if(n->distortion != val)
		{
		    printf("Error at %d, %d, %d, index %d: %d != %d\n", i,j,k, idx,n->distortion, val);
		    exit(1);
		}
		
		assert(n->distortion == TWIDDLE(i,j,k));
	    }
	}
	
	for(j=0; j<hm_side_len; j++)
	{
	    for(k=0; k<hm_side_len; k++)
	    {
		hid_t hid;
		HID_SET(hid, i, j, k);
		
		heightmap_element_t *h = tile_hid_lookup(t, hid);
		
		assert(h->height == TWIDDLE(i,j,k));
	    }
	}
	
    }
    
    for(i=0;i<TILE_LEVELS*2; i++)
    {
	int node_side_len = 1<<i;
	int hm_side_len = 2<<i;
	
	for(j=0; j<node_side_len; j++)
	{
	    for(k=0; k<node_side_len; k++)
	    {
		nid_t nid;
		NID_SET(nid, i, j, k);
		
		t_node_t *n = tile_nid_lookup(t, nid);
		int val = TWIDDLE(k,j,i);

		n->distortion = val;
	    }
	}
	
	for(j=0; j<hm_side_len; j++)
	{
	    for(k=0; k<hm_side_len; k++)
	    {
		hid_t hid;
		HID_SET(hid, i, j, k);
		
		heightmap_element_t *h = tile_hid_lookup(t, hid);
		int val = TWIDDLE(k,j,i);
		h->height = val;
	    }
	}
	
    }


    for(i=0;i<TILE_LEVELS*2; i++)
    {
	int node_side_len = 1<<i;
	int hm_side_len = 2<<i;
	
	for(j=0; j<node_side_len; j++)
	{
	    for(k=0; k<node_side_len; k++)
	    {
		nid_t nid;
		NID_SET(nid, i, j, k);
		
		t_node_t *n = tile_nid_lookup(t, nid);
		int val = TWIDDLE(k,j,i);

		int idx = n - t->node;
		
		if(n->distortion != val)
		{
		    printf("Error at %d, %d, %d, index %d: %d != %d\n", i,j,k, idx,n->distortion, val);
		    exit(1);
		}
		
	    }
	}
	
	for(j=0; j<hm_side_len; j++)
	{
	    for(k=0; k<hm_side_len; k++)
	    {
		hid_t hid;
		HID_SET(hid, i, j, k);
		
		heightmap_element_t *h = tile_hid_lookup(t, hid);
		int val = TWIDDLE(k,j,i);
		
		assert(h->height == val);
	    }
	}
	
    }
    
    printf("Done.\n");
    
    return 0;
    
}
