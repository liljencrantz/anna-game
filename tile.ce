#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "tile.h"

#define my_assert(cond) if(!(cond)){int *x=0;*x=0;}

    

heightmap_element_t tile_default_he = 
{
    0,
    {
	0.0, 0.0, 1.0
    }
}
    ;

void tile_init(tile_t *t)
{
    memset(t, 0, sizeof(tile_t));    
}

t_node_t *tile_nid_lookup(tile_t *tile, nid_t nid)
{
    if(NID_GET_LEVEL(nid) < TILE_LEVELS)
    {
	int idx = TILE_NODE_OFFSET(NID_GET_LEVEL(nid)) + NID_GET_IDX(nid);
/*
	printf(
	    "Direct tile nid lookup. %d, %d, index %d\n\n",
	    NID_GET_LEVEL(nid), NID_GET_IDX(nid), idx);
*/	
	return &tile->node[idx];
	
    }
    else
    {
	nid_t nid2;	

	unsigned int x_pos = NID_GET_X_POS(nid);
	unsigned int y_pos = NID_GET_Y_POS(nid);
	
	NID_SET_LEVEL(nid2,NID_GET_LEVEL(nid)-TILE_LEVELS);

	unsigned int len = TILE_SUBTILE_NODE_PER_TILE(NID_GET_LEVEL(nid2));
	
	unsigned int subtile_x_pos = x_pos % len;
	unsigned int subtile_x_idx = x_pos / len;
	
	unsigned int subtile_y_pos = y_pos % len;
	unsigned int subtile_y_idx = y_pos / len;

	NID_SET_POS(nid2, subtile_x_pos, subtile_y_pos);
	
	unsigned int subtile_idx = subtile_x_idx + subtile_y_idx*TILE_SUBTILE_COUNT_PER_SIDE;
/*	
	printf(
	    "Subtile lookup. %d, %d => %d, (%d,%d) => %d, %d, (%d,%d) => %d, %d, %d\n",
	    NID_GET_LEVEL(nid), NID_GET_IDX(nid),
	    NID_GET_LEVEL(nid), x_pos, y_pos,
	    subtile_idx, NID_GET_LEVEL(nid2), subtile_x_pos, subtile_y_pos,
	    subtile_idx, NID_GET_LEVEL(nid2), NID_GET_IDX(nid2));
*/	
	if(!tile->subtile[subtile_idx])
	{
/*
	    printf(
		"Null tile lookup. %d, %d\n",
		NID_GET_LEVEL(nid), NID_GET_IDX(nid));
*/
	    return 0;
	}
	
	return tile_nid_lookup(
	    tile->subtile[subtile_idx],
	    nid2);
    }
}


heightmap_element_t *tile_hid_lookup(tile_t *tile, hid_t hid)
{
    if(HID_GET_LEVEL(hid) < TILE_LEVELS)
    {
/*
	printf(
	    "Direct tile hid lookup. %d, %d\n\n",
	    HID_GET_LEVEL(hid), HID_GET_IDX(hid));
*/	
	return &tile->hm[TILE_HM_OFFSET(HID_GET_LEVEL(hid)) + HID_GET_IDX(hid)];
    }
    else
    {
	hid_t hid2;

	unsigned int x_pos = HID_GET_X_POS(hid);
	unsigned int y_pos = HID_GET_Y_POS(hid);
	
	HID_SET_LEVEL(hid2,HID_GET_LEVEL(hid)-TILE_LEVELS);
	
	unsigned int len = TILE_SUBTILE_HM_PER_TILE(HID_GET_LEVEL(hid2));

	unsigned int subtile_x_pos = x_pos % len;
	unsigned int subtile_x_idx = x_pos / len;
	
	unsigned int subtile_y_pos = y_pos % len;
	unsigned int subtile_y_idx = y_pos / len;

	HID_SET_POS(hid2, subtile_x_pos, subtile_y_pos);
	
	unsigned int subtile_idx = subtile_x_idx + subtile_y_idx*TILE_SUBTILE_COUNT_PER_SIDE;
//	printf("x %d, y %d => %d, %d\n", x_pos, y_pos, subtile_x_idx, subtile_y_idx);
/*
	if(
	    (subtile_x_pos == 0 && subtile_x_idx==TILE_SUBTILE_COUNT_PER_SIDE) ||
	    (subtile_y_pos == 0 && subtile_y_idx==TILE_SUBTILE_COUNT_PER_SIDE))
	{
	    return &tile_default_he;	    
	}
*/	
	
	my_assert(subtile_x_idx < TILE_SUBTILE_COUNT_PER_SIDE);
	my_assert(subtile_y_idx < TILE_SUBTILE_COUNT_PER_SIDE);

/*	
	printf(
	    "Subtile lookup. %d, %d => %d, (%d,%d) => %d, %d, (%d,%d) => %d, %d, %d\n",
	    HID_GET_LEVEL(hid), HID_GET_IDX(hid),
	    HID_GET_LEVEL(hid), x_pos, y_pos,
	    subtile_idx, HID_GET_LEVEL(hid2), subtile_x_pos, subtile_y_pos,
	    subtile_idx, HID_GET_LEVEL(hid2), HID_GET_IDX(hid2));
*/	
	if(!tile->subtile[subtile_idx])
	{
/*	    printf(
		"Null hm lookup. %d, %d\n",
		HID_GET_LEVEL(hid), HID_GET_IDX(hid));
*/
	    return 0;
	    return &tile_default_he;
	}
//	printf("subtile %d\n", subtile_idx);
	return tile_hid_lookup(
	    tile->subtile[subtile_idx],
	    hid2);
    }
}

