/* -*- c-basic-offset: 8 -*-
 rdesktop: A Remote Desktop Protocol client.
 Generics backingstore operations
 Copyright (C) Jay Sorg 2005-2006
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define SPLIT_COLOUR15(c, r, g, b) \
{ \
r = ((c >> 7) & 0xf8) | ((c >> 12) & 0x7); \
g = ((c >> 2) & 0xf8) | ((c >>  8) & 0x7); \
b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); \
}

#define SPLIT_COLOUR16(c, r, g, b) \
{ \
r = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7); \
g = ((c >> 3) & 0xfc) | ((c >>  9) & 0x3); \
b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); \
}

#define MAKE_COLOUR15(c, r, g, b) \
{ \
c = ( \
(((r & 0xff) >> 3) << 10) | \
(((g & 0xff) >> 3) <<  5) | \
(((b & 0xff) >> 3) <<  0) \
); \
}

#define MAKE_COLOUR32(c, r, g, b) \
{ \
c = ( \
((r & 0xff) << 16) | \
((g & 0xff) <<  8) | \
((b & 0xff) <<  0) \
); \
}

int bs_get_pixel(int x, int y);
void bs_set_pixel(int x, int y, int pixel, int rop, int use_clip);
int bs_do_rop(int rop, int src, int dst);
void bs_init(int width, int height, int bpp);
void bs_exit(void);
void bs_set_clip(int x, int y, int cx, int cy);
void bs_reset_clip(void);
void bs_set_pixel_on(char * data, int x, int y, int width, int bpp,
                     int pixel);
int bs_is_pixel_on(char * data, int x, int y, int width, int bpp);
void bs_copy_mem(char * d, char * s, int n);
void bs_copy_memb(char * d, char * s, int n);
int bs_warp_coords(int * x, int * y, int * cx, int * cy,
                   int * srcx, int * srcy);
void bs_rect(int x, int y, int cx, int cy, int colour, int rop);
void bs_screenblt(int opcode, int x, int y, int cx, int cy,
                  int srcx, int srcy);
void bs_memblt(int opcode, int x, int y, int cx, int cy,
               void * srcdata, int srcwidth, int srcheight,
               int srcx, int srcy);
void bs_copy_box(char * dst, int x, int y, int cx, int cy, int line_size);
void bs_draw_glyph(int x, int y, char * glyph_data, int glyph_width,
                   int glyph_height, int fgcolour);
void bs_line(int opcode, int startx, int starty, int endx, int endy,
             int pen_width, int pen_style, int pen_colour);
void bs_patblt(int opcode, int x, int y, int cx, int cy,
               int brush_style, char * brush_pattern,
               int brush_x_org, int brush_y_org,
               int bgcolour, int fgcolour);