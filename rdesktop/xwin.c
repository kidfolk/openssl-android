#include <android/log.h>
typedef unsigned char uint8;
void ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height,
                     uint8 *data) {
    //AndroidBitmapInfo info;
    //void* pixels;
    android_bitmap_creater(x,y,cx,cy,width,height,data);
	__android_log_print(ANDROID_LOG_INFO,"JNIMsg","ui_paint_bitmap(l=%d,t=%d,r=%d,b=%d,w=%d,h=%d,Bpp=%d,cmp=%d)",x, y, cx, cy, width, height);
}