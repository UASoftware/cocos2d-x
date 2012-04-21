#########################################
#	dwarfs-cocos2d
#########################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

PROJECT_ROOT_PATH := $(LOCAL_PATH)/../..

subdirs :=	$(PROJECT_ROOT_PATH)/lua/proj.android/jni/Android.mk \
			$(PROJECT_ROOT_PATH)/cocos2dx/Android.mk \
			$(PROJECT_ROOT_PATH)/CocosDenshion/android/Android.mk \
			$(LOCAL_PATH)/lua_cocos2dx.mk
			
include $(subdirs)
