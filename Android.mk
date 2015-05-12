LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := HRMTest

LOCAL_SRC_FILES += \
	i2c-ctrl.cpp \
	heartrate.cpp \
	led_ctrl.cpp \

LOCAL_LDLIBS += $(LOCAL_PATH)/libpaw8001motion.so


include $(BUILD_EXECUTABLE)
