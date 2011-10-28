#include <android/log.h>
typedef unsigned char uint8;
void ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height,
                     uint8 *data) {
    android_bitmap_creater(x,y,cx,cy,width,height,data);
}