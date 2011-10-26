#include <stdarg.h>		/* va_list va_start va_end */
#include <unistd.h>		/* read close getuid getgid getpid getppid gethostname */
#include <fcntl.h>		/* open */
#include <pwd.h>		/* getpwuid */
#include <termios.h>		/* tcgetattr tcsetattr */
#include <sys/stat.h>		/* stat */
#include <sys/time.h>		/* gettimeofday */
#include <sys/times.h>		/* times */
#include <ctype.h>		/* toupper */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "rdesktop.h"
#include "ssl.h"
#include "org_kidfolk_androidRDP_AndroidRDPActivity.h"
#include <android/log.h>

char *g_username;
//char password[64];
char *password;
char g_hostname[16];

RD_BOOL g_bitmap_cache = True;
RD_BOOL g_bitmap_cache_persist_enable = False;
int g_server_depth = -1;
RD_BOOL g_bitmap_cache_precache = True;
RD_BOOL g_encryption = True;
RD_BOOL g_use_rdp5 = True;
uint32 g_rdp5_performanceflags = RDP5_NO_WALLPAPER | RDP5_NO_FULLWINDOWDRAG
| RDP5_NO_MENUANIMATIONS;
RD_BOOL g_has_reconnect_random = False;
uint32 g_reconnect_logonid = 0;
char g_reconnect_random[16];

/* Session Directory redirection */
RD_BOOL g_redirect = False;
char g_redirect_server[64];
char g_redirect_domain[16];
char g_redirect_password[64];
char *g_redirect_username;
char g_redirect_cookie[128];
uint32 g_redirect_flags = 0;

uint8 g_client_random[SEC_RANDOM_SIZE];

int g_width = 800;
int g_height = 600;

RD_BOOL g_bitmap_compression = True;
RD_BOOL g_desktop_save = True; /* desktop save order */
RD_BOOL g_polygon_ellipse_orders = True; /* polygon / ellipse orders */
RD_BOOL g_pending_resize = False;

unsigned int g_keylayout = 0x409; /* Defaults to US keyboard layout */
int g_keyboard_type = 0x4; /* Defaults to US keyboard layout */
int g_keyboard_subtype = 0x0; /* Defaults to US keyboard layout */
int g_keyboard_functionkeys = 0xc; /* Defaults to US keyboard layout */

RD_BOOL g_console_session = False;

RD_BOOL deactivated;
uint32 ext_disc_reason = 0;

JNIEnv *genv;
jobject *gobj;

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
//    g_username = (char *) xmalloc(strlen(username) + 1);
//    STRNCPY(g_username, username, sizeof(g_username));
    const char *nativeString = (*env)->GetStringUTFChars(env,username,NULL);
    g_username = (char *) xmalloc(strlen(nativeString) + 1);
    STRNCPY(g_username,nativeString,strlen(nativeString)+1);
    //__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "setUsername:%s",g_username);
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
    //__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "setPassword:%s",str);
}

JNIEXPORT jint JNICALL Java_org_kidfolk_androidRDP_AndroidRDPActivity_rdp_1connect(JNIEnv *env, jobject obj, jstring jserver, jint flags, jstring domain, jstring jpassword, jstring shell, jstring directory, jboolean g_redirect)
{
    genv = env;
    gobj = obj;
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdp_1connect");
    int result = 1;
    const char *nativeServer = (*env)->GetStringUTFChars(env,jserver,NULL);
    char *server = (char *) xmalloc(strlen(nativeServer)+1);
    STRNCPY(server,nativeServer,strlen(nativeServer)+1);
    //__android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdp_1connect server:%s",server);
    //__android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdp_1connect username:%s",g_username);
    //__android_log_print(ANDROID_LOG_INFO,"JNIMsg","rdp_1connect password:%s",password);
    result = rdp_connect(server,flags,domain,password,shell,directory,g_redirect);
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

void android_bitmap_creater(int x,int y,int cx,int cy,int width,int height,uint8 *data)
{
    jclass cls = (*genv)->GetObjectClass(genv, gobj);
    jmethodID mid = (*genv)->GetMethodID(genv,cls,"getBitmapBytesFormNative","(IIII[B)V");
    if (mid == NULL) {
        return; /* method not found */
    }
    jint size = strlen(data);
    jbyteArray bArray = (*genv)->NewByteArray(genv,size);
    memcpy(bArray,data,size);
    //jstring s = (*genv)->NewString(genv,data, size);
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","android_bitmap_creater(x=%d,y=%d,cx=%d,cy=%d,width=%d,height=%d,data=%d)",x, y, cx, cy, width, height,strlen(data));
    (*genv)->CallVoidMethod(genv, gobj, mid,x,y,width,height,bArray);
    __android_log_print(ANDROID_LOG_INFO,"JNIMsg","android_bitmap_creater(x=%d,y=%d,cx=%d,cy=%d,width=%d,height=%d,data=%p)",x, y, cx, cy, width, height,data);

}

/* report an unimplemented protocol feature */
void unimpl(char *format, ...)
{
	va_list ap;
    
	fprintf(stderr, "NOT IMPLEMENTED: ");
    
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

/* report a warning */
void warning(char *format, ...)
{
	va_list ap;
    
	fprintf(stderr, "WARNING: ");
    
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

/* report an error */
void error(char *format, ...)
{
	va_list ap;
    
	fprintf(stderr, "ERROR: ");
    
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

/* realloc; exit if out of memory */
void *
xrealloc(void *oldmem, size_t size) {
	void *mem;
    
	if (size == 0)
		size = 1;
	mem = realloc(oldmem, size);
	if (mem == NULL)
	{
		error("xrealloc %ld\n", size);
		exit(EX_UNAVAILABLE);
	}
	return mem;
}

/* free */
void xfree(void *mem) {
	free(mem);
}

void save_licence(unsigned char *data, int length) {
	char *home, *path, *tmppath;
	int fd;
    
	home = getenv("EXTERNAL_STORAGE");
	if (home == NULL
        )
		return;
    
	path = (char *) xmalloc(
                            strlen(home) + strlen(g_hostname) + sizeof("/.rdesktop/licence."));
    
	sprintf(path, "%s/.rdesktop", home);
	if ((mkdir(path, 0700) == -1) && errno != EEXIST)
	{
		perror(path);
		return;
	}
    
	/* write licence to licence.hostname.new, then atomically rename to licence.hostname */
    
	sprintf(path, "%s/.rdesktop/licence.%s", home, g_hostname);
	tmppath = (char *) xmalloc(strlen(path) + sizeof(".new"));
	strcpy(tmppath, path);
	strcat(tmppath, ".new");
    
	fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1) {
		perror(tmppath);
		return;
	}
    
	if (write(fd, data, length) != length) {
		perror(tmppath);
		unlink(tmppath);
	} else if (rename(tmppath, path) == -1) {
		perror(path);
		unlink(tmppath);
	}
    
	close(fd);
	xfree(tmppath);
	xfree(path);
}

int load_licence(unsigned char **data) {
	char *home, *path;
	struct stat st;
	int fd, length;
    
	home = getenv("EXTERNAL_STORAGE");
	if (home == NULL
        )
		return -1;
    
	path = (char *) xmalloc(
                            strlen(home) + strlen(g_hostname) + sizeof("/.rdesktop/licence."));
	sprintf(path, "%s/.rdesktop/licence.%s", home, g_hostname);
    
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -1;
    
	if (fstat(fd, &st))
		return -1;
    
	*data = (uint8 *) xmalloc(st.st_size);
	length = read(fd, *data, st.st_size);
	close(fd);
	xfree(path);
	return length;
}

/* Create the bitmap cache directory */
RD_BOOL rd_pstcache_mkdir(void) {
	char *home;
	char bmpcache_dir[256];
    
	//home = getenv("HOME");
    //get android sdcard root directory
    home = getenv("EXTERNAL_STORAGE");
    
	if (home == NULL)
		return False;
    
	sprintf(bmpcache_dir, "%s/%s", home, ".rdesktop");
    
	if ((mkdir(bmpcache_dir, S_IRWXU) == -1) && errno != EEXIST)
	{
		perror(bmpcache_dir);
		return False;
	}
    
	sprintf(bmpcache_dir, "%s/%s", home, ".rdesktop/cache");
    
	if ((mkdir(bmpcache_dir, S_IRWXU) == -1) && errno != EEXIST)
	{
		perror(bmpcache_dir);
		return False;
	}
    
	return True;
}

/* open a file in the .rdesktop directory */
int rd_open_file(char *filename) {
	char *home;
	char fn[256];
	int fd;
    
	home = getenv("EXTERNAL_STORAGE");
	if (home == NULL
        )
		return -1;sprintf(fn, "%s/.rdesktop/%s", home, filename);
	fd = open(fn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1)
		perror(fn);
	return fd;
}

/* do a write lock on a file */
RD_BOOL rd_lock_file(int fd, int start, int len) {
	struct flock lock;
    
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = start;
	lock.l_len = len;
	if (fcntl(fd, F_SETLK, &lock) == -1)
		return False;
	return True;
}

/* close file */
void rd_close_file(int fd) {
	close(fd);
}

/* read from file*/
int rd_read_file(int fd, void *ptr, int len) {
	return read(fd, ptr, len);
}

/* write to file */
int rd_write_file(int fd, void *ptr, int len) {
	return write(fd, ptr, len);
}

/* move file pointer */
int rd_lseek_file(int fd, int offset) {
	return lseek(fd, offset, SEEK_SET);
}

/* malloc; exit if out of memory */
void *
xmalloc(int size) {
	void *mem = malloc(size);
	if (mem == NULL)
	{
		error("xmalloc %d\n", size);
		exit(EX_UNAVAILABLE);
	}
	return mem;
}

/* Generate a 32-byte random for the secure transport code. */
void generate_random(uint8 * random) {
	struct stat st;
	struct tms tmsbuf;
	SSL_MD5 md5;
	uint32 *r;
	int fd, n;
    
	/* If we have a kernel random device, try that first */
	if (((fd = open("/dev/urandom", O_RDONLY)) != -1)
        || ((fd = open("/dev/random", O_RDONLY)) != -1)) {
		n = read(fd, random, 32);
		close(fd);
		if (n == 32)
			return;
	}
    
#ifdef EGD_SOCKET
	/* As a second preference use an EGD */
	if (generate_random_egd(random))
		return;
#endif
    
	/* Otherwise use whatever entropy we can gather - ideas welcome. */
	r = (uint32 *) random;
	r[0] = (getpid()) | (getppid() << 16);
	r[1] = (getuid()) | (getgid() << 16);
	r[2] = times(&tmsbuf); /* system uptime (clocks) */
	gettimeofday((struct timeval *) &r[3], NULL); /* sec and usec */
	stat("/tmp", &st);
	r[5] = st.st_atime;
	r[6] = st.st_mtime;
	r[7] = st.st_ctime;
    
	/* Hash both halves with MD5 to obscure possible patterns */
	ssl_md5_init(&md5);
	ssl_md5_update(&md5, random, 16);
	ssl_md5_final(&md5, random);
	ssl_md5_update(&md5, random + 16, 16);
	ssl_md5_final(&md5, random + 16);
}

/*
 input: src is the string we look in for needle.
 Needle may be escaped by a backslash, in
 that case we ignore that particular needle.
 return value: returns next src pointer, for
 succesive executions, like in a while loop
 if retval is 0, then there are no more args.
 pitfalls:
 src is modified. 0x00 chars are inserted to
 terminate strings.
 return val, points on the next val chr after ins
 0x00
 
 example usage:
 while( (pos = next_arg( optarg, ',')) ){
 printf("%s\n",optarg);
 optarg=pos;
 }
 
 */
char *
next_arg(char *src, char needle) {
	char *nextval;
	char *p;
	char *mvp = 0;
    
	/* EOS */
	if (*src == (char) 0x00)
		return 0;
    
	p = src;
	/*  skip escaped needles */
	while ((nextval = strchr(p, needle))) {
		mvp = nextval - 1;
		/* found backslashed needle */
		if (*mvp == '\\' && (mvp > src)) {
			/* move string one to the left */
			while (*(mvp + 1) != (char) 0x00) {
				*mvp = *(mvp + 1);
				mvp++;
			}
			*mvp = (char) 0x00;
			p = nextval;
		} else {
			p = nextval + 1;
			break;
		}
        
	}
    
	/* more args available */
	if (nextval) {
		*nextval = (char) 0x00;
		return ++nextval;
	}
    
	/* no more args after this, jump to EOS */
	nextval = src + strlen(src);
	return nextval;
}

void toupper_str(char *p) {
	while (*p) {
		if ((*p >= 'a') && (*p <= 'z'))
			*p = toupper((int) *p);
		p++;
	}
}

/* not all clibs got ltoa */
#define LTOA_BUFSIZE (sizeof(long) * 8 + 1)

char *
l_to_a(long N, int base) {
	static char ret[LTOA_BUFSIZE];
    
	char *head = ret, buf[LTOA_BUFSIZE], *tail = buf + sizeof(buf);
    
	register int divrem;
    
	if (base < 36 || 2 > base)
		base = 10;
    
	if (N < 0) {
		*head++ = '-';
		N = -N;
	}
    
	tail = buf + sizeof(buf);
	*--tail = 0;
    
	do {
		divrem = N % base;
		*--tail = (divrem <= 9) ? divrem + '0' : divrem + 'a' - 10;
		N /= base;
	} while (N);
    
	strcpy(head, tail);
	return ret;
}
