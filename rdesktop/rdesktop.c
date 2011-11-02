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

char g_title[64] = "";
char *g_username;
char *password;
char g_hostname[16];
char g_keymapname[PATH_MAX] = "";
unsigned int g_keylayout = 0x409; /* Defaults to US keyboard layout */
int g_keyboard_type = 0x4; /* Defaults to US keyboard layout */
int g_keyboard_subtype = 0x0; /* Defaults to US keyboard layout */
int g_keyboard_functionkeys = 0xc; /* Defaults to US keyboard layout */
int g_sizeopt = 0; /* If non-zero, a special size has been
                    requested. If 1, the geometry will be fetched
                    from _NET_WORKAREA. If negative, absolute value
                    specifies the percent of the whole screen. */
int g_width = 800;
int g_height = 600;
int g_xpos = 0;
int g_ypos = 0;
int g_pos = 0; /* 0 position unspecified,
                1 specified,
                2 xpos neg,
                4 ypos neg  */
extern int g_tcp_port_rdp;
int g_server_depth = -1;
int g_win_button_size = 0; /* If zero, disable single app mode */
RD_BOOL g_bitmap_compression = True;
RD_BOOL g_sendmotion = True;
RD_BOOL g_bitmap_cache = True;
RD_BOOL g_bitmap_cache_persist_enable = False;
RD_BOOL g_bitmap_cache_precache = True;
RD_BOOL g_encryption = True;
RD_BOOL g_packet_encryption = True;
RD_BOOL g_desktop_save = True; /* desktop save order */
RD_BOOL g_polygon_ellipse_orders = True; /* polygon / ellipse orders */
RD_BOOL g_fullscreen = False;
RD_BOOL g_grab_keyboard = True;
RD_BOOL g_hide_decorations = False;
RD_BOOL g_use_rdp5 = True;
RD_BOOL g_rdpclip = True;
RD_BOOL g_console_session = False;
RD_BOOL g_numlock_sync = False;
RD_BOOL g_lspci_enabled = False;
RD_BOOL g_owncolmap = False;
RD_BOOL g_ownbackstore = True; /* We can't rely on external BackingStore */
RD_BOOL g_seamless_rdp = False;
RD_BOOL g_user_quit = False;
uint32 g_embed_wnd;
uint32 g_rdp5_performanceflags = RDP5_NO_WALLPAPER | RDP5_NO_FULLWINDOWDRAG
| RDP5_NO_MENUANIMATIONS;
/* Session Directory redirection */
RD_BOOL g_redirect = False;
char g_redirect_server[64];
char g_redirect_domain[16];
char g_redirect_password[64];
char *g_redirect_username;
char g_redirect_cookie[128];
uint32 g_redirect_flags = 0;

uint32 g_reconnect_logonid = 0;
char g_reconnect_random[16];
RD_BOOL g_has_reconnect_random = False;
uint8 g_client_random[SEC_RANDOM_SIZE];
RD_BOOL g_pending_resize = False;

RD_BOOL deactivated;
uint32 ext_disc_reason = 0;

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

void parse_server_and_port(char *server) {
	printf("parse_server_and_port\n");
	char *p;
#ifdef IPv6
	int addr_colons;
#endif
    
#ifdef IPv6
	p = server;
	addr_colons = 0;
	while (*p)
        if (*p++ == ':')
            addr_colons++;
	if (addr_colons >= 2)
	{
		/* numeric IPv6 style address format - [1:2:3::4]:port */
		p = strchr(server, ']');
		if (*server == '[' && p != NULL)
		{
			if (*(p + 1) == ':' && *(p + 2) != '\0')
                g_tcp_port_rdp = strtol(p + 2, NULL, 10);
			/* remove the port number and brackets from the address */
			*p = '\0';
			strncpy(server, server + 1, strlen(server));
		}
	}
	else
	{
		/* dns name or IPv4 style address format - server.example.com:port or 1.2.3.4:port */
		p = strchr(server, ':');
		if (p != NULL)
		{
			g_tcp_port_rdp = strtol(p + 1, NULL, 10);
			*p = 0;
		}
	}
#else /* no IPv6 support */
	p = strchr(server, ':');
	if (p != NULL)
	{
		g_tcp_port_rdp = strtol(p + 1, NULL, 10);
		*p = 0;
	}
#endif /* IPv6 */
    
}

static int handle_disconnect_reason(RD_BOOL deactivated, uint16 reason) {
	printf("handle_disconnect_reason\n");
	char *text;
	int retval;
    
	switch (reason) {
        case exDiscReasonNoInfo:
            text = "No information available";
            if (deactivated)
                retval = EX_OK;
            else
                retval = EXRD_UNKNOWN;
            break;
            
        case exDiscReasonAPIInitiatedDisconnect:
        case exDiscReasonWindows7Disconnect:
            text = "Server initiated disconnect";
            retval = EXRD_API_DISCONNECT;
            break;
            
        case exDiscReasonAPIInitiatedLogoff:
            text = "Server initiated logoff";
            retval = EXRD_API_LOGOFF;
            break;
            
        case exDiscReasonServerIdleTimeout:
            text = "Server idle timeout reached";
            retval = EXRD_IDLE_TIMEOUT;
            break;
            
        case exDiscReasonServerLogonTimeout:
            text = "Server logon timeout reached";
            retval = EXRD_LOGON_TIMEOUT;
            break;
            
        case exDiscReasonReplacedByOtherConnection:
            text = "The session was replaced";
            retval = EXRD_REPLACED;
            break;
            
        case exDiscReasonOutOfMemory:
            text = "The server is out of memory";
            retval = EXRD_OUT_OF_MEM;
            break;
            
        case exDiscReasonServerDeniedConnection:
            text = "The server denied the connection";
            retval = EXRD_DENIED;
            break;
            
        case exDiscReasonServerDeniedConnectionFips:
            text = "The server denied the connection for security reason";
            retval = EXRD_DENIED_FIPS;
            break;
            
        case exDiscReasonLicenseInternal:
            text = "Internal licensing error";
            retval = EXRD_LIC_INTERNAL;
            break;
            
        case exDiscReasonLicenseNoLicenseServer:
            text = "No license server available";
            retval = EXRD_LIC_NOSERVER;
            break;
            
        case exDiscReasonLicenseNoLicense:
            text = "No valid license available";
            retval = EXRD_LIC_NOLICENSE;
            break;
            
        case exDiscReasonLicenseErrClientMsg:
            text = "Invalid licensing message";
            retval = EXRD_LIC_MSG;
            break;
            
        case exDiscReasonLicenseHwidDoesntMatchLicense:
            text = "Hardware id doesn't match software license";
            retval = EXRD_LIC_HWID;
            break;
            
        case exDiscReasonLicenseErrClientLicense:
            text = "Client license error";
            retval = EXRD_LIC_CLIENT;
            break;
            
        case exDiscReasonLicenseCantFinishProtocol:
            text = "Network error during licensing protocol";
            retval = EXRD_LIC_NET;
            break;
            
        case exDiscReasonLicenseClientEndedProtocol:
            text = "Licensing protocol was not completed";
            retval = EXRD_LIC_PROTO;
            break;
            
        case exDiscReasonLicenseErrClientEncryption:
            text = "Incorrect client license enryption";
            retval = EXRD_LIC_ENC;
            break;
            
        case exDiscReasonLicenseCantUpgradeLicense:
            text = "Can't upgrade license";
            retval = EXRD_LIC_UPGRADE;
            break;
            
        case exDiscReasonLicenseNoRemoteConnections:
            text = "The server is not licensed to accept remote connections";
            retval = EXRD_LIC_NOREMOTE;
            break;
            
        default:
            if (reason > 0x1000 && reason < 0x7fff) {
                text = "Internal protocol error";
            } else {
                text = "Unknown reason";
            }
            retval = EXRD_UNKNOWN;
	}
	if (reason != exDiscReasonNoInfo
        )
		fprintf(stderr, "disconnect: %s.\n", text);
    
	return retval;
}
