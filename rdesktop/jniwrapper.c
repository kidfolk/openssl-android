#include "org_kidfolk_androidRDP_AndroidRDPActivity.h"
#include "org_kidfolk_androidRDP_RemoteView.h"
#include "rdesktop.h"

extern int g_width;
extern int g_height;

extern char *g_username;
extern char *password;
extern int g_server_depth;

extern RD_BOOL deactivated;
extern uint32 ext_disc_reason;

extern RD_BOOL g_packet_encryption;
extern RD_BOOL g_encryption;



JavaVM *cached_jvm;
JNIEnv *cached_env;
jobject *cached_obj;

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    setResolution
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_setResolution
(JNIEnv *env, jobject obj, jint width, jint height)
{
    g_width = width;
    g_height = height;
}

JNIEXPORT jstring JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_getenv(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_INFO, "JNIMsg", "getenv");
    return (*env)->NewStringUTF(env, getenv("EXTERNAL_STORAGE"));
}

/*
 * Class:     org_kidfolk_androidRDP_RdesktopNative
 * Method:    setUsername
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_setUsername(JNIEnv *env, jobject obj, jstring username)
{
    const char *nativeString = (*env)->GetStringUTFChars(env,username,NULL);
    g_username = (char *) xmalloc(strlen(nativeString) + 1);
    STRNCPY(g_username,nativeString,strlen(nativeString)+1);
}

/*
 * Class:     org_kidfolk_androidRDP_RdesktopNative
 * Method:    setPassword
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_setPassword(JNIEnv *env, jobject obj, jstring jpassword)
{
    const char *str = (*env)->GetStringUTFChars(env,jpassword,NULL);
    password = (char *) xmalloc(strlen(str) + 1);
    STRNCPY(password,str,strlen(str)+1);
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    setServerDepth
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_setServerDepth
(JNIEnv * env, jobject obj, jint depth)
{
    g_server_depth = depth;
}

JNIEXPORT jint JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1connect(JNIEnv *env, jobject obj, jstring jserver, jint flags, jstring domain, jstring jpassword, jstring shell, jstring directory, jboolean g_redirect)
{
    cached_env = env;
    cached_obj = obj;
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdp_1connect");
    int result = 1;
    const char *nativeServer = (*env)->GetStringUTFChars(env,jserver,NULL);
    char *server = (char *) xmalloc(strlen(nativeServer)+1);
    STRNCPY(server,nativeServer,strlen(nativeServer)+1);
    parse_server_and_port(server);
    result = rdp_connect(server,flags,domain,password,shell,directory,0);
    if (!g_packet_encryption)
        g_encryption = False;
    return result;
    
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    rdpdr_init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdpdr_1init(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdpdr_1init");
    rdpdr_init();
    
}

/*
 * Class:     org_kidfolk_androidRDP_RdesktopNative
 * Method:    rdp_main_loop
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1main_1loop(JNIEnv *env, jobject obj)
{
    rdp_main_loop(&deactivated, &ext_disc_reason);
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    rdp_disconnect
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1disconnect(JNIEnv *env, jobject obj)
{
    
    rdp_disconnect();
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    rdp_reset_state
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1reset_1state
(JNIEnv * env, jobject obj)
{
    rdp_reset_state();
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    cache_save_state
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_cache_1save_1state
(JNIEnv * env, jobject obj)
{
    cache_save_state();
    
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    rdp_send_client_window_status
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1send_1client_1window_1status
(JNIEnv *env, jobject obj, jint status)
{
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1send_1client_1window_1status");
    rdp_send_client_window_status(status);
}

/*
 * Class:     org_kidfolk_androidRDP_AndroidRDPActivity
 * Method:    rdp_send_input
 * Signature: (ISSSS)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1send_1input
(JNIEnv *env, jobject obj, jint time, jshort message_type, jshort device_flags, jshort param1, jshort param2)
{
    rdp_send_input(time,message_type,device_flags,param1,param2);
}

void android_draw_bitmap(int x,int y,int cx,int cy,int width,int height,uint8 *data)
{
    jclass cls = (*cached_env)->GetObjectClass(cached_env, cached_obj);
    jmethodID mid = (*cached_env)->GetMethodID(cached_env,cls,"renderBitmap","(IIIIII[B)V");
    if (mid == NULL) {
        return; /* method not found */
    }
    jbyteArray arr = (*cached_env)->NewByteArray(cached_env,width*height*2);
    (*cached_env)->SetByteArrayRegion(cached_env,arr,0,width*height*2,(jbyte*)data);
    //__android_log_print(ANDROID_LOG_INFO,"JNIMsg","android_draw_bitmap(x=%d,y=%d,cx=%d,cy=%d,width=%d,height=%d,data.length=%d)",x, y, cx, cy, width, height,length);
    (*cached_env)->CallVoidMethod(cached_env, cached_obj, mid,x,y,cx,cy,width,height,arr);
    (*cached_env)->DeleteLocalRef(cached_env,cls);
    (*cached_env)->DeleteLocalRef(cached_env,arr);
}

void android_draw_line(int startx,int starty,int endx,int endy,uint32 color,uint8 width,uint8 style)
{
    jclass cls = (*cached_env)->GetObjectClass(cached_env, cached_obj);
    jmethodID mid = (*cached_env)->GetMethodID(cached_env,cls,"renderLine","(IIIIIIB)V");
    if (mid == NULL) {
        return; /* method not found */
    }
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","android_draw_line(startx=%d,starty=%d,endx=%d,endy=%d,color=%d,width=%d,style=%d)",startx, starty, endx, endy, color,width,style);
    (*cached_env)->CallVoidMethod(cached_env, cached_obj,mid, startx,starty,endx,endy,color,width,style);
    (*cached_env)->DeleteLocalRef(cached_env,cls);
}

void android_draw_rect(int x,int y,int cx,int cy,uint32 color)
{
    jclass cls = (*cached_env)->GetObjectClass(cached_env, cached_obj);
    jmethodID mid = (*cached_env)->GetMethodID(cached_env,cls,"renderRect","(IIIII)V");
    if (mid == NULL) {
        return; /* method not found */
    }
    
    (*cached_env)->CallVoidMethod(cached_env, cached_obj, mid,x,y,cx,cy,color);
    (*cached_env)->DeleteLocalRef(cached_env,cls);
}

void android_set_pixel(int x, int y, int color)
{
	jclass cls = (*cached_env)->GetObjectClass(cached_env, cached_obj);
	jmethodID mid = (*cached_env)->GetMethodID(cached_env,cls,"setPixel","(III)V");
    if (mid == NULL) {
        return; /* method not found */
    }
	(*cached_env)->CallVoidMethod(cached_env, cached_obj,mid, x, y, color);
	(*cached_env)->DeleteLocalRef(cached_env, cls);
}

/*
 * Class:     org_kidfolk_androidRDP_RemoteView
 * Method:    native_handle_mouse_button
 * Signature: (JIIII)V
 */
JNIEXPORT void JNICALL Java_org_kidfolk_androidRDP_RemoteView_native_1handle_1mouse_1button
(JNIEnv *env, jobject obj, jint button, jint x, jint y, jint isdown)
{
    ui_mouse_button(button,x,y,isdown);
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    
    cached_jvm = jvm;
    if ((*jvm)->GetEnv(jvm, (void **)&cached_env, JNI_VERSION_1_2)) {
        return JNI_ERR; /* JNI version not supported */
    }
    //TODO register_native_function
    //register_native_function(cached_env);
    return (*cached_env)->GetVersion(cached_env);

}

