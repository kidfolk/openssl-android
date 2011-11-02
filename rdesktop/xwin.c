/* -*- c-basic-offset: 8 -*-
rdesktop: A Remote Desktop Protocol client.
User interface services - X Window System
Copyright (C) Matthew Chapman <matthewc.unsw.edu.au> 1999-2008
Copyright 2007-2008 Pierre Ossman <ossman@cendio.se> for Cendio AB
Copyright 2002-2011 Peter Astrand <astrand@cendio.se> for Cendio AB

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <strings.h>
#include "rdesktop.h"
#include "bsops.h"

extern int g_sizeopt;
extern int g_width;
extern int g_height;
extern int g_xpos;
extern int g_ypos;
extern int g_pos;
extern RD_BOOL g_sendmotion;
extern RD_BOOL g_fullscreen;
extern RD_BOOL g_grab_keyboard;
extern RD_BOOL g_hide_decorations;
extern RD_BOOL g_pending_resize;
extern char g_title[];
/* Color depth of the RDP session.
 As of RDP 5.1, it may be 8, 15, 16 or 24. */
extern int g_server_depth;
extern int g_win_button_size;

static int g_x_socket;

static unsigned long g_seamless_focused = 0;
static RD_BOOL g_seamless_started = False;	/* Server end is up and running */
static RD_BOOL g_seamless_active = False;	/* We are currently in seamless mode */
static RD_BOOL g_seamless_hidden = False;	/* Desktop is hidden on server */
static RD_BOOL g_seamless_broken_restack = False;	/* WM does not properly restack */
extern RD_BOOL g_seamless_rdp;

extern uint32 g_embed_wnd;
RD_BOOL g_enable_compose = False;
RD_BOOL g_Unobscured;		/* used for screenblt */

/* Color depth of the X11 visual of our window (e.g. 24 for True Color R8G8B visual).
 This may be 32 for R8G8B8 visuals, and then the rest of the bits are undefined
 as far as we're concerned. */
static int g_depth;
/* Bits-per-Pixel of the pixmaps we'll be using to draw on our window.
 This may be larger than g_depth, in which case some of the bits would
 be kept solely for alignment (e.g. 32bpp pixmaps on a 24bpp visual). */
static int g_bpp;

/* Maps logical (xmodmap -pp) pointing device buttons (0-based) back
 to physical (1-based) indices. */
static unsigned char g_pointer_log_to_phys_map[32];
static RD_HCURSOR g_null_cursor = NULL;

static RD_BOOL g_focused;
static RD_BOOL g_mouse_in_wnd;
/* Indicates that:
 1) visual has 15, 16 or 24 depth and the same color channel masks
 as its RDP equivalent (implies X server is LE),
 2) host is LE
 This will trigger an optimization whose real value is questionable.
 */
static RD_BOOL g_compatible_arch;
/* Indicates whether RDP's bitmaps and our XImages have the same
 binary format. If so, we can avoid an expensive translation.
 Note that this can be true when g_compatible_arch is false,
 e.g.:
 
 RDP(LE) <-> host(BE) <-> X-Server(LE)
 
 ('host' is the machine running rdesktop; the host simply memcpy's
 so its endianess doesn't matter)
 */
static RD_BOOL g_no_translate_image = False;

/* endianness */
static RD_BOOL g_host_be;
static RD_BOOL g_xserver_be;
static int g_red_shift_r, g_blue_shift_r, g_green_shift_r;
static int g_red_shift_l, g_blue_shift_l, g_green_shift_l;

/* software backing store */
extern RD_BOOL g_ownbackstore;

/* Moving in single app mode */
static RD_BOOL g_moving_wnd;
static int g_move_x_offset = 0;
static int g_move_y_offset = 0;
static RD_BOOL g_using_full_workarea = False;

/* colour maps */
extern RD_BOOL g_owncolmap;
static uint32 *g_colmap = NULL;

struct bitmap
{
	uint8 * data;
	uint32 width;
	uint32 height;
};

void
ui_resize_window()
{
}

/* Returns 0 after user quit, 1 otherwise */
int
ui_select(int rdp_socket)
{
}

void
ui_move_pointer(int x, int y)
{
}

RD_HBITMAP
ui_create_bitmap(int width, int height, uint8 * data)
{
	struct bitmap * b;
	int size;
    
	size = width * height * ((g_server_depth + 7) / 8);
	b = (struct bitmap *) xmalloc(sizeof(struct bitmap));
	b->data = (uint8 *) xmalloc(size);
	memcpy(b->data, data, size);
	b->width = width;
	b->height = height;
    
	return (RD_HBITMAP)b;
}

//void uiDrawImage(int x, int y, int cx, int cy, int width, int height, const uint8* data);

void
ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data)
{
	android_draw_bitmap(x, y, cx, cy, width, height, data);
}

void
ui_destroy_bitmap(RD_HBITMAP bmp)
{
	struct bitmap * b;
    
	b = (struct bitmap *) bmp;
	if (b != 0)
	{
		xfree(b->data);
	}
	xfree(b);
}

RD_HGLYPH
ui_create_glyph(int width, int height, uint8 * data)
{
	int i;
	int j;
	char * glyph_data;
	struct bitmap * the_glyph;
    
	glyph_data = (char *) xmalloc(width * height);
	memset(glyph_data, 0, width * height);
	the_glyph = (struct bitmap *) xmalloc(sizeof(struct bitmap));
	memset(the_glyph, 0, sizeof(struct bitmap));
	the_glyph->width = width;
	the_glyph->height = height;
	the_glyph->data = glyph_data;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			if (bs_is_pixel_on(data, j, i, width, 1))
			{
				bs_set_pixel_on(glyph_data, j, i, width, 8, 255);
			}
		}
	}
	return (RD_HGLYPH)the_glyph;
}

void
ui_destroy_glyph(RD_HGLYPH glyph)
{
	struct bitmap * the_glyph;
    
	the_glyph = glyph;
	if (the_glyph != 0)
	{
		xfree(the_glyph->data);
	}
	xfree(the_glyph);
}

RD_HCURSOR
ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
                 uint8 * andmask, uint8 * xormask, int bpp)
{
	return (RD_HCURSOR) NULL;
}

void
ui_set_cursor(RD_HCURSOR cursor)
{
}

void
ui_destroy_cursor(RD_HCURSOR cursor)
{
}

void
ui_set_null_cursor(void)
{
	ui_set_cursor(g_null_cursor);
}

RD_HCOLOURMAP
ui_create_colourmap(COLOURMAP * colours)
{
}

void
ui_destroy_colourmap(RD_HCOLOURMAP map)
{
}

void
ui_set_colourmap(RD_HCOLOURMAP map)
{
}

void
ui_set_clip(int x, int y, int cx, int cy)
{
}

void
ui_reset_clip(void)
{
}

void
ui_bell(void)
{
}

void
ui_destblt(uint8 opcode,
           /* dest */ int x, int y, int cx, int cy)
{
}

void
ui_patblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
}

void
ui_screenblt(uint8 opcode,
             /* dest */ int x, int y, int cx, int cy,
             /* src */ int srcx, int srcy)
{
}

void
ui_memblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ RD_HBITMAP src, int srcx, int srcy)
{
	struct bitmap* b;
    
	b = (struct bitmap*)src;
    
	ui_paint_bitmap(x, y, cx, cy, b->width, b->height, b->data);
}

void
ui_triblt(uint8 opcode,
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

//void uiDrawLine(int x1, int y1, int x2, int y2, int color, int width, int style);

void
ui_line(uint8 opcode,
        /* dest */ int startx, int starty, int endx, int endy,
        /* pen */ PEN * pen)
{
	int r, g, b;
	uint32 color;
    
	SPLIT_COLOUR16(pen->colour, r, g, b);
    
	MAKE_COLOUR32(color, r, g, b);
    
	color |= 0xFF000000;
    
	android_draw_line(startx, starty, endx, endy, color, pen->width, pen->style);
}

//void uiFillRect(int x, int y, int cx, int cy, int color);

void
ui_rect(
        /* dest */ int x, int y, int cx, int cy,
        /* brush */ int colour)
{
	int r, g, b;
	uint32 color;
    
	SPLIT_COLOUR16(colour, r, g, b);
    
	MAKE_COLOUR32(color, r, g, b);
    
	color |= 0xFF000000;
    
	android_draw_rect(x, y, cx, cy, color);
}

void
ui_polygon(uint8 opcode,
           /* mode */ uint8 fillmode,
           /* dest */ RD_POINT * point, int npoints,
           /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
}

void
ui_polyline(uint8 opcode,
            /* dest */ RD_POINT * points, int npoints,
            /* pen */ PEN * pen)
{
}

void
ui_ellipse(uint8 opcode,
           /* mode */ uint8 fillmode,
           /* dest */ int x, int y, int cx, int cy,
           /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
}

/* warning, this function only draws on wnd or backstore, not both */
void
ui_draw_glyph(int mixmode,
              /* dest */ int x, int y, int cx, int cy,
              /* src */ RD_HGLYPH glyph, int srcx, int srcy,
              int bgcolour, int fgcolour)
{
}

//////////////////////////////////////////////////////////////////////////

static void
draw_glyph(int x, int y, void * glyph, int fgcolor)
{
	struct bitmap * b;
    
	//LOGI("---------------- draw_glyph");
    
	b = glyph;
	bs_draw_glyph(x, y, b->data, b->width, b->height, fgcolor);
}

#define DO_GLYPH(ttext,idx) \
{ \
glyph = cache_get_font(font, ttext[idx]); \
if (!(flags & TEXT2_IMPLICIT_X)) \
{ \
xyoffset = ttext[++idx]; \
if (xyoffset & 0x80) \
{ \
if (flags & TEXT2_VERTICAL) \
{ \
y += ttext[idx + 1] | (ttext[idx + 2] << 8); \
} \
else \
{ \
x += ttext[idx + 1] | (ttext[idx + 2] << 8); \
} \
idx += 2; \
} \
else \
{ \
if (flags & TEXT2_VERTICAL) \
{ \
y += xyoffset; \
} \
else \
{ \
x += xyoffset; \
} \
} \
} \
if (glyph != NULL) \
{ \
draw_glyph(x + glyph->offset, y + glyph->baseline, glyph->pixmap, \
fgcolour); \
if (flags & TEXT2_IMPLICIT_X) \
{ \
x += glyph->width; \
} \
} \
}

void
ui_draw_text(uint8 font, uint8 flags, uint8 opcode, int mixmode, int x, int y,
             int clipx, int clipy, int clipcx, int clipcy,
             int boxx, int boxy, int boxcx, int boxcy, BRUSH * brush,
             int bgcolour, int fgcolour, uint8 * text, uint8 length)
{
	FONTGLYPH *glyph;
	int i, j, xyoffset, x1, y1;
	DATABLOB *entry;
    
	/* Sometimes, the boxcx value is something really large, like
     32691. This makes XCopyArea fail with Xvnc. The code below
     is a quick fix. */
	if (boxx + boxcx > g_width)
	{
		boxcx = g_width - boxx;
	}
    
	if (boxcx > 1)
	{
		ui_rect(boxx, boxy, boxcx, boxcy, bgcolour);
	}
	else if (mixmode == MIX_OPAQUE)
	{
		ui_rect(clipx, clipy, clipcx, clipcy, bgcolour);
	}
    
	//SET_FOREGROUND(fgcolour);
	//SET_BACKGROUND(bgcolour);
	//XSetFillStyle(g_display, g_gc, FillStippled);
    
	/* Paint text, character by character */
	for (i = 0; i < length;)
	{
		switch (text[i])
		{
			case 0xff:
				/* At least two bytes needs to follow */
				if (i + 3 > length)
				{
					warning("Skipping short 0xff command:");
					for (j = 0; j < length; j++)
						fprintf(stderr, "%02x ", text[j]);
					fprintf(stderr, "\n");
					i = length = 0;
					break;
				}
				cache_put_text(text[i + 1], text, text[i + 2]);
				i += 3;
				length -= i;
				/* this will move pointer from start to first character after FF command */
				text = &(text[i]);
				i = 0;
				break;
                
			case 0xfe:
				/* At least one byte needs to follow */
				if (i + 2 > length)
				{
					warning("Skipping short 0xfe command:");
					for (j = 0; j < length; j++)
						fprintf(stderr, "%02x ", text[j]);
					fprintf(stderr, "\n");
					i = length = 0;
					break;
				}
				entry = cache_get_text(text[i + 1]);
				if (entry->data != NULL)
				{
					if ((((uint8 *) (entry->data))[1] == 0)
					    && (!(flags & TEXT2_IMPLICIT_X)) && (i + 2 < length))
					{
						if (flags & TEXT2_VERTICAL)
							y += text[i + 2];
						else
							x += text[i + 2];
					}
					for (j = 0; j < entry->size; j++)
						DO_GLYPH(((uint8 *) (entry->data)), j);
				}
				if (i + 2 < length)
					i += 3;
				else
					i += 2;
				length -= i;
				/* this will move pointer from start to first character after FE command */
				text = &(text[i]);
				i = 0;
				break;
                
			default:
				DO_GLYPH(text, i);
				i++;
				break;
		}
	}
}

void
ui_desktop_save(uint32 offset, int x, int y, int cx, int cy)
{
}

void
ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy)
{
}

/* these do nothing here but are used in uiports */
void
ui_begin_update(void)
{
}

void
ui_end_update(void)
{
}


void
ui_seamless_begin(RD_BOOL hidden)
{
	if (!g_seamless_rdp)
		return;
    
	if (g_seamless_started)
		return;
    
	g_seamless_started = True;
	g_seamless_hidden = hidden;
    
	if (!hidden)
		ui_seamless_toggle();
}


void
ui_seamless_end()
{
	g_seamless_started = False;
	g_seamless_active = False;
	g_seamless_hidden = False;
}


void
ui_seamless_hide_desktop()
{
	if (!g_seamless_rdp)
		return;
    
	if (!g_seamless_started)
		return;
    
	if (g_seamless_active)
		ui_seamless_toggle();
    
	g_seamless_hidden = True;
}


void
ui_seamless_unhide_desktop()
{
	if (!g_seamless_rdp)
		return;
    
	if (!g_seamless_started)
		return;
    
	g_seamless_hidden = False;
    
	ui_seamless_toggle();
}


void
ui_seamless_toggle()
{
	if (!g_seamless_rdp)
		return;
    
	if (!g_seamless_started)
		return;
    
	if (g_seamless_hidden)
		return;
    
	if (g_seamless_active)
	{
		/* Deactivate */
	}
	else
	{
		/* Activate */
		//seamless_send_sync();
	}
    
	g_seamless_active = !g_seamless_active;
}


void
ui_seamless_create_window(unsigned long id, unsigned long group, unsigned long parent,
                          unsigned long flags)
{
}


void
ui_seamless_destroy_window(unsigned long id, unsigned long flags)
{
}


void
ui_seamless_destroy_group(unsigned long id, unsigned long flags)
{
}


void
ui_seamless_seticon(unsigned long id, const char *format, int width, int height, int chunk,
                    const char *data, int chunk_len)
{
}


void
ui_seamless_delicon(unsigned long id, const char *format, int width, int height)
{
}


void
ui_seamless_move_window(unsigned long id, int x, int y, int width, int height, unsigned long flags)
{
}


void
ui_seamless_restack_window(unsigned long id, unsigned long behind, unsigned long flags)
{
}


void
ui_seamless_settitle(unsigned long id, const char *title, unsigned long flags)
{
}


void
ui_seamless_setstate(unsigned long id, unsigned int state, unsigned long flags)
{
}


void
ui_seamless_syncbegin(unsigned long flags)
{
}


void
ui_seamless_ack(unsigned int serial)
{
}

unsigned int
read_keyboard_state()
{
	return 0;
}

uint16
ui_get_numlock_state(unsigned int state)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
void
ui_mouse_move(int x, int y)
{
	rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, (uint16) x, (uint16) y);
}


/*****************************************************************************/
void
ui_mouse_button(int button, int x, int y, int down)
{
	uint16 flags;
    
	flags = 0;
	if (down)
	{
		flags |= MOUSE_FLAG_DOWN;
	}
	switch (button)
	{
        case 1:
            flags |= MOUSE_FLAG_BUTTON1;
            break;
        case 2:
            flags |= MOUSE_FLAG_BUTTON2;
            break;
        case 3:
            flags |= MOUSE_FLAG_BUTTON3;
            break;
        case 4:
            flags |= MOUSE_FLAG_BUTTON4;
            break;
        case 5:
            flags |= MOUSE_FLAG_BUTTON5;
            break;
	}
	rdp_send_input(0, RDP_INPUT_MOUSE, flags, (uint16) x, (uint16) y);
}


/*****************************************************************************/
void
ui_key_down(int key, int ext)

{
	rdp_send_input(0, RDP_INPUT_SCANCODE, (uint16) (RDP_KEYPRESS | ext),
                   (uint16) key, 0);
}

/*****************************************************************************/
void
ui_key_up(int key, int ext)
{
	rdp_send_input(0, RDP_INPUT_SCANCODE, (uint16) (RDP_KEYRELEASE | ext),
                   (uint16) key, 0);
}
