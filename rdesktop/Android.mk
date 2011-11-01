LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := rdesktop.c \
                   bitmap.c \
                   pstcache.c \
                   cache.c \
                   tcp.c \
                   iso.c \
                   mcs.c \
                   ssl.c \
                   licence.c \
                   orders.c \
                   mppc.c \
                   rdp5.c \
                   rdp.c \
                   secure.c \
                   channels.c \
                   cliprdr.c \
                   printercache.c \
                   rdpdr.c \
                   parallel.c \
                   printer.c \
                   disk.c \
                   serial.c \
                   xwin.c \
                   jniwrapper.c \
                   
                   
                   

#LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -llog
                   
LOCAL_SHARED_LIBRARIES := \
			libssl \
			libcrypto \
			
LOCAL_C_INCLUDES := \
	$(NDK_PROJECT_PATH) \
	$(NDK_PROJECT_PATH)/include
	
LOCAL_MODULE := rdesktop

LOCAL_CFLAGS := -DMONOLITH

          
include $(BUILD_SHARED_LIBRARY)
