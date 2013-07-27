NO_COLOR=\033[0m
GREEN_COLOR=\033[32;01m
BLUE_COLOR=\033[34;01m
PB_COLOR=\033[33m


SDK_PATH:=$(shell pwd)/../../../../ARDroneLib
PC_TARGET=yes
USE_LINUX=yes

ifdef MYKONOS
   include $(ARDRONE_CUSTOM_CONFIG)
   include $(ARDRONE_BUILD_CONFIG)
else
   include $(SDK_PATH)/Soft/Build/custom.makefile
   include $(SDK_PATH)/Soft/Build/config.makefile
endif

ifeq "$(RELEASE_BUILD)" "yes"
   ARDRONE_TARGET_DIR=$(shell pwd)/../../Build/Release
else
   ARDRONE_TARGET_DIR=$(shell pwd)/../../Build/Debug
endif

TARGET=ardrone_navigation

SRC_DIR:=$(shell pwd)/../Sources


COMMON_MOB_SRC_PATH = common
UTILS_SRC_PATH 		= utils
VIDEO_SRC_PATH 		= video
UI_SRC_PATH 		= UI
BT_SRC_PATH 		= BT
REF_SEND_SRC_PATH 	= ref_send
IHM_SRC_PATH 		= ihm
NDC_SRC_PATH 		= navdata_client
VC_SRC_PATH		= vicon_client

# Define application source files

GENERIC_BINARIES_SOURCE_DIR:=$(SRC_DIR)

GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
	$(COMMON_MOB_SRC_PATH)/mobile_config.c		\
	$(COMMON_MOB_SRC_PATH)/mobile_wifi.c		\
	$(UI_SRC_PATH)/ui.c				\
	$(UI_SRC_PATH)/gamepad.c			\
	$(UI_SRC_PATH)/wiimote.c			\
	$(UI_SRC_PATH)/ardrone_ini.c    		\
	$(UTILS_SRC_PATH)/utils.c			\
	$(UTILS_SRC_PATH)/remote_console.c		\
	$(NDC_SRC_PATH)/navdata_client.c		\
	$(NDC_SRC_PATH)/navdata_tablepilotage.c		\
	$(NDC_SRC_PATH)/navdata_ihm.c			\
	$(IHM_SRC_PATH)/view_drone_attitude.c		\
	$(IHM_SRC_PATH)/ihm_vision.c    		\
	$(IHM_SRC_PATH)/ihm_stages_o_gtk.c    		\
	$(IHM_SRC_PATH)/ihm.c \
	$(IHM_SRC_PATH)/ihm_config.c

ifeq ($(PC_USE_POLARIS),yes)
	GENERIC_CFLAGS+=-DPC_USE_POLARIS
	GENERIC_BINARIES_COMMON_SOURCE_FILES+=		\
	$(NDC_SRC_PATH)/navdata_polaris.c		
endif

ifeq ($(USE_ARDRONE_VICON),yes)
GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
	$(VC_SRC_PATH)/vicon_client.c			\
	$(VC_SRC_PATH)/vicon_ihm.c
endif

ifeq ($(USE_RAW_CAPTURE),yes)	
GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
    $(NDC_SRC_PATH)/raw_capture.c
endif

ifeq ($(USE_ARDRONE_VICON),yes)
   GENERIC_CFLAGS+=-DUSE_ARDRONE_VICON
endif

GENERIC_INCLUDES+=					\
	$(SRC_DIR) 					\
	$(LIB_DIR) 					\
	$(SDK_PATH)/Soft/Common 			\
	$(SDK_PATH)/Soft/Lib 				\
	/usr/include/libxml2


ifeq ($(PC_USE_POLARIS),yes)
GENERIC_INCLUDES+=						\
	$(SRC_PATH)/Soft/Lib  \
	$(SRC_PATH)/Soft/Lib/libPolaris
endif

ifeq ($(USE_ARDRONE_VICON),yes)
GENERIC_INCLUDES+=						\
	$(SRC_PATH)/Soft/Lib  \
	$(SRC_PATH)/Soft/Lib/libViconDataStreamSDK
endif

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
GENERIC_INCLUDES+=						\
	$(SRC_PATH)/Soft/Lib	\
	$(SRC_PATH)/Soft/Lib/libCalibration
endif

ifeq ($(USE_ARDRONE_VISION),yes)
GENERIC_INCLUDES+=						\
	$(SRC_PATH)/Soft/Lib	\
	$(SRC_PATH)/Soft/Lib/Vision
endif

GENERIC_TARGET_BINARIES_PREFIX=

GENERIC_TARGET_BINARIES_DIR=$(ARDRONE_TARGET_DIR)

GENERIC_BINARIES_SOURCE_ENTRYPOINTS+=			\
	$(COMMON_MOB_SRC_PATH)/mobile_main.c

GENERIC_INCLUDES:=$(addprefix -I,$(GENERIC_INCLUDES))

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
GENERIC_CFLAGS+=-DPC_USE_CALIBRATION
endif

ifeq ($(USE_ARDRONE_VISION),yes)
GENERIC_CFLAGS+=-DPC_USE_VISION
endif

GENERIC_LIB_PATHS=-L$(GENERIC_TARGET_BINARIES_DIR)
GENERIC_CFLAGS+=-DLIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE
GENERIC_LIBS=-liw -lpc_ardrone -lgthread-2.0 -lgtk-x11-2.0 -lrt -lxml2 -ludev -lswscale

ifeq ($(USE_ARDRONE_VISION),yes)
GENERIC_LIBS+=-lpc_vision
endif

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
GENERIC_LIBS+=-lcalibration
endif

ifeq ($(PC_USE_POLARIS),yes)
GENERIC_LIBS+=-lpc_polaris
endif

ifeq ($(USE_ARDRONE_VICON),yes)
GENERIC_LIB_PATHS=-L$(SRC_PATH)/Soft/Lib/libViconDataStreamSDK
GENERIC_LIBS+=-lViconDataStreamSDK -lpc_vicon
endif

ifeq ($(PC_USE_TABLE_PILOTAGE),yes)
GENERIC_LIBS+=-lpc_test_benchs
endif

ifeq ($(WIIMOTE_SUPPORT),yes)
#  GENERIC_LIBS+=-lpc_cwiid
# Stephane : lib cwiid is now a Ubuntu package
GENERIC_LIBS+=-lcwiid
endif

ifeq ($(EXTERNAL_CONTROL),yes)
GENERIC_LIBS+=-lpc_ardrone_control
endif

SDK_FLAGS+="USE_APP=yes"
SDK_FLAGS+="APP_ID=ardrone_navigation"

GENERIC_CFLAGS+=-DRECORD_VIDEO

ifeq ($(FFMPEG_RECORDING_SUPPORT),yes)
GENERIC_LIBS+=-lswscale -lavformat -lavcodec -lavutil -lm 
GENERIC_CFLAGS+=-DUSE_FFMPEG_RECORDER
endif

export GENERIC_CFLAGS
export GENERIC_LIBS
export GENERIC_LIB_PATHS
export GENERIC_INCLUDES
export GENERIC_BINARIES_SOURCE_DIR
export GENERIC_BINARIES_COMMON_SOURCE_FILES
export GENERIC_TARGET_BINARIES_PREFIX
export GENERIC_TARGET_BINARIES_DIR
export GENERIC_BINARIES_SOURCE_ENTRYPOINTS

# Bug fix ...
export GENERIC_LIBRARY_SOURCE_DIR=$(GENERIC_BINARIES_SOURCE_DIR)

.PHONY: $(TARGET) build_libs

all: print_title build_libs $(TARGET)

print_title:
	@if [ '$(MAKECMDGOALS)' != 'clean' ]; then echo "$(BLUE_COLOR)-- Building Ardrone Navigation -- $(NO_COLOR)"; fi

$(TARGET): 
	@if [ '$(MAKECMDGOALS)' != 'clean' ]; then echo "$(GREEN_COLOR)Linking navigation $(NO_COLOR)"; fi
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)
	mv $(GENERIC_TARGET_BINARIES_DIR)/$(COMMON_MOB_SRC_PATH)/mobile_main $(TARGET)
	mv $(TARGET) $(ARDRONE_TARGET_DIR)/
	@if [ ! -e $(ARDRONE_TARGET_DIR)/ardrone.xml ]; then cp $(SRC_DIR)/xml_files/ardrone_default.xml $(ARDRONE_TARGET_DIR)/ardrone.xml; fi
ifeq ($(USE_ARDRONE_VICON),yes)
	cp $(SRC_PATH)/Soft/Lib/libViconDataStreamSDK/libViconDataStreamSDK.so $(ARDRONE_TARGET_DIR)/
	cp $(SRC_PATH)/Soft/Lib/libViconDataStreamSDK/configure_vicon.sh $(ARDRONE_TARGET_DIR)/
endif

$(MAKECMDGOALS): build_libs
	@if [ '$(MAKECMDGOALS)' != 'clean' ]; then echo "$(GREEN_COLOR)Compiling navigation sources$(NO_COLOR)"; fi
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)

build_libs: 
ifeq ($(WIIMOTE_SUPPORT),yes)
	$(info Compiling Navigation with Wiimote)
endif
	@$(MAKE) -C $(SDK_PATH)/Soft/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)

makefileverbose:
	@echo "Dump compilation flags:"
	@echo "GENERIC_CFLAGS : $(GENERIC_CFLAGS)"
	@echo "GENERIC_LIBS : $(GENERIC_LIBS)"
	@echo "GENERIC_LIB_PATHS : $(GENERIC_LIB_PATHS)"
	@echo "GENERIC_INCLUDES : $(GENERIC_INCLUDES)"
	@echo "GENERIC_BINARIES_SOURCE_DIR : $(GENERIC_BINARIES_SOURCE_DIR)"
	@echo "GENERIC_BINARIES_COMMON_SOURCE_FILES : $(GENERIC_BINARIES_COMMON_SOURCE_FILES)"
	@echo "GENERIC_TARGET_BINARIES_PREFIX : $(GENERIC_TARGET_BINARIES_PREFIX)"
	@echo "GENERIC_TARGET_BINARIES_DIR : $(GENERIC_TARGET_BINARIES_DIR)"
	@echo "GENERIC_BINARIES_SOURCE_ENTRYPOINTS : $(GENERIC_BINARIES_SOURCE_ENTRYPOINTS)"
	@echo "TMP_SDK_FLAGS: $(TMP_SDK_FLAGS)"
	@echo "SDK_FLAGS: $(SDK_FLAGS	)"
	@echo "Make cmd goals: $(MAKECMDGOALS)"

