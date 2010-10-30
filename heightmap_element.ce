#include "heightmap_element.h"
#include "scene.h"


int hid_get_nid_at_level(hid_t hid, nid_t *nid, int level)
{
    int hid_x = HID_GET_X_POS(hid);
    int hid_y = HID_GET_Y_POS(hid);
    int hid_level = HID_GET_LEVEL(hid);
    int w_diff = hid_level-level+1;
    int nid_x_width=1;
    int nid_x = hid_x >> w_diff;
    int nid_y_width=1;
    int nid_y = hid_y >> w_diff;
    if((nid_x<<w_diff == hid_x) && nid_x != 0)
    {
	nid_x--;
	nid_x_width=2;
    }
    if((nid_y<<w_diff == hid_y) && nid_y != 0)
    {
	nid_y--;
	nid_y_width=2;
    }
    int i,j;
    int count = 0;
    
    for(i=0; i<nid_x_width; i++)
    {
	for(j=0; j<nid_y_width; j++)
	{
	    NID_SET(nid[count], level, nid_x+i, nid_y+j);
	    count++;
	}
	
    }
/*
    printf(
	"Get nids at level %d for hid %d %d %d. Found %d matches:", 
	level, HID_GET_LEVEL(hid), HID_GET_X_POS(hid), HID_GET_Y_POS(hid), count);
    for(i=0; i<count;i++)
    {
	printf(" (%d %d %d)", NID_GET_LEVEL(nid[i]), NID_GET_X_POS(nid[i]), NID_GET_Y_POS(nid[i]));
	
    }
    printf("\n");
*/  
    return count;
    
}


