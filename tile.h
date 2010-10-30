#ifndef TILE_H
#define TILE_H

#include "node.h"
#include "heightmap_element.h"
  
/**
   Number of node levels in a tile
*/
#define TILE_LEVELS 4
#define TILE_SUBTILE_COUNT_PER_SIDE (1<<TILE_LEVELS)
#define TILE_SUBTILE_NODE_PER_TILE(sublevel) (1<<(sublevel))
#define TILE_SUBTILE_HM_PER_TILE(sublevel) (2<<(sublevel))
/**
   = sum(n=1..level){ 2^(2n+1) }
 */
#define TILE_NODE_COUNT(level) (0x55555555 & ((1<<(2*(level+1)))-1))
#define TILE_HM_COUNT(level) (TILE_NODE_COUNT(level+1)-1)

#define TILE_NODE_OFFSET(level) (TILE_NODE_COUNT(level-1))
#define TILE_HM_OFFSET(level) (TILE_HM_COUNT(level-1))
#define TILE_SUBTILE_COUNT (1<<(2*TILE_LEVELS))

struct tile
{
    t_node_t node[TILE_NODE_COUNT(TILE_LEVELS)];
    heightmap_element_t hm[TILE_HM_COUNT(TILE_LEVELS)];
    struct tile *subtile[TILE_SUBTILE_COUNT];    
};

typedef struct tile tile_t;

void tile_init(tile_t *t);
void tile_add_subtile(tile_t *t, int subidx, tile_t *s);
t_node_t *tile_nid_lookup(tile_t *tile, nid_t nid);
heightmap_element_t *tile_hid_lookup(tile_t *tile, hid_t nid);

#endif
