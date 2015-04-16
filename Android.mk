LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := HRMTest

LOCAL_SRC_FILES += \
	i2c-ctrl.cpp \
	i2c-ctrl.h \
	heartrate.cpp \
	pixart_8001_1000.h \
	led_ctrl.cpp \
	led_ctrl.h \

include $(BUILD_EXECUTABLE)
