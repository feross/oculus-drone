LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Do not use thumb because of VLIB assembly parts
LOCAL_CFLAGS := -Wall -marm
LOCAL_CFLAGS += -I$(SDK_PATH)
LOCAL_CFLAGS += -I$(SDK_PATH)/Soft/Common
LOCAL_CFLAGS += -I$(SDK_PATH)/Soft/Lib
LOCAL_CFLAGS += -I$(SDK_PATH)/VLIB
LOCAL_CFLAGS += -I$(SDK_PATH)/VLIB/Platform/arm9
LOCAL_CFLAGS += -I$(SDK_PATH)/VP_SDK
LOCAL_CFLAGS += -I$(SDK_PATH)/VP_SDK/VP_Com
LOCAL_CFLAGS += -I$(SDK_PATH)/VP_SDK/VP_Os
LOCAL_CFLAGS += -I$(SDK_PATH)/VP_SDK/VP_Os/linux
LOCAL_CFLAGS += -I$(SDK_PATH)/VP_SDK/VP_Com/linux
LOCAL_CFLAGS += -D__USE_GNU -DNO_ARDRONE_MAINLOOP -DUSE_ANDROID -D__linux__ -DTARGET_CPU_ARM=1 -DTARGET_CPU_X86=0 -DUSE_WIFI
LOCAL_CFLAGS += -DANDROID_NDK -DDISABLE_IMPORTGL

LOCAL_MODULE := ardrone

LOCAL_SRC_FILES := \
    app.c \
	android.c \
	default.c \
   video_stage.c \
   opengl_stage.c \
   navdata.c \
   video.c \
   control_ack.c 

LOCAL_STATIC_LIBRARIES := pc_ardrone vlib sdk

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)

