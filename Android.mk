LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := i2c-ctrl

LOCAL_SRC_FILES += \
	i2c-ctrl.cpp \
	i2c-ctrl.h \
	test_main.cpp \

include $(BUILD_EXECUTABLE)
