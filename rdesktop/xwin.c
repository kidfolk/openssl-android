#include <android/log.h>
#include "rdesktop.h"
#include "bsops.h"

/* Color depth of the RDP session.
 As of RDP 5.1, it may be 8, 15, 16 or 24. */
extern int g_server_depth;

static int g_red_shift_r, g_blue_shift_r, g_green_shift_r;
static int g_red_shift_l, g_blue_shift_l, g_green_shift_l;

struct bitmap
{
    uint8 *data;
    uint32 width;
    uint32 height;
};

RD_HBITMAP ui_create_bitmap(int width,int height,uint8 *data)
{
    struct bitmap *b;
    int size;
    
    size = width * height * ((g_server_depth+7)/8);
    b = (struct bitmap *)xmalloc(sizeof(struct bitmap));
    b->data = (uint8 *)xmalloc(size);
    memcpy(b->data,data,size);
    b->width = width;
    b->height = height;
    
    return (RD_HBITMAP)b;
}

void ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height,
                     uint8 *data) {
    android_draw_bitmap(x,y,cx,cy,width,height,data);
}

void ui_destroy_bitmap(RD_HBITMAP bmp)
{
    struct bitmap *b;
    
    b = (struct bitmap *)bmp;
    
    if(b!=0){
        xfree(b->data);
    }
    xfree(b);
}

RD_HGLYPH ui_create_glyph(int width, int height, uint8 * data) {
    char *glyph_data;
    struct bitmap *glyph;
    
    glyph_data = (char *)xmalloc(width*height);
    memset(glyph_data,0,width*height);
    glyph = (struct bitmap *)xmalloc(sizeof(struct bitmap));
    memset(glyph,0,sizeof(struct bitmap));
    glyph->width = width;
    glyph->height = height;
    glyph->data = glyph_data;
    
    return (RD_HGLYPH) glyph;
}

void ui_destroy_glyph(RD_HGLYPH glyph)
{
	struct bitmap * the_glyph;
    
	the_glyph = glyph;
	if (the_glyph != 0)
	{
		xfree(the_glyph->data);
	}
	xfree(the_glyph);
}

void ui_line(uint8 opcode,
/* dest */int startx, int starty, int endx, int endy,
/* pen */PEN * pen) {
	int r,g,b;
    uint32 color;
    
    SPLIT_COLOUR16(pen->colour,r,g,b);
    MAKE_COLOUR32(color,r,g,b);
    
    color |= 0xFF000000;
    
    android_draw_line(startx,starty,endx,endy,color,pen->width,pen->style);
    
}

RD_HCURSOR ui_create_cursor(unsigned int x, unsigned int y, int width,
                 int height, uint8 * andmask, uint8 * xormask, int bpp) {
    return (RD_HCURSOR) NULL;
}

void ui_set_cursor(RD_HCURSOR cursor) {
	
}

void ui_memblt(uint8 opcode,
/* dest */int x, int y, int cx, int cy,
/* src */RD_HBITMAP src, int srcx, int srcy) 
{
    
    struct bitmap *b;
    
    b = (struct bitmap*)src;
    
    ui_paint_bitmap(x,y,cx,cy,b->width,b->height,b->data);
	
}

void ui_patblt(uint8 opcode,
/* dest */int x, int y, int cx, int cy,
/* brush */BRUSH * brush, int bgcolour, int fgcolour) 
{
    
}

void ui_triblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ RD_HBITMAP src, int srcx, int srcy,
          /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
	/* This is potentially difficult to do in general. Until someone
     comes up with a more efficient way of doing it I am using cases. */
    
	switch (opcode)
	{
		case 0x69:	/* PDSxxn */
			ui_memblt(ROP2_XOR, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_NXOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;
            
		case 0xb8:	/* PSDPxax */
			ui_patblt(ROP2_XOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			ui_memblt(ROP2_AND, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_XOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;
            
		case 0xc0:	/* PSa */
			ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_AND, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;
            
		default:
			unimpl("triblt 0x%x\n", opcode);
			ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
	}
}


void ui_rect(
/* dest */int x, int y, int cx, int cy,
/* brush */int colour) {
	int r,g,b;
    uint32 color;
    
    SPLIT_COLOUR16(color,r,g,b);
    
    MAKE_COLOUR32(color,r,g,b);
    
    color |= 0xFF000000;
    
    android_draw_rect(x,y,cx,cy,color);
}


















