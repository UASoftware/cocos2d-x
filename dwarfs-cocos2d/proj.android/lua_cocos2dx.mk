LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := lua_cocos2dx_static
LOCAL_MODULE_FILENAME := liblua-cocos2dx
LOCAL_ARM_MODE := arm

ifneq ($(APP_OPTIM),debug)
	LOCAL_CFLAGS := -O3
endif #($(APP_OPTIM),debug)



PROJECT_ROOT_PATH := ../..
LUA_ROOT_PATH := $(PROJECT_ROOT_PATH)/lua

LOCAL_SRC_FILES :=	$(LUA_ROOT_PATH)/cocos2dx_support/CCLuaEngine.cpp \
					$(LUA_ROOT_PATH)/cocos2dx_support/Cocos2dxLuaLoader.cpp \
					$(LUA_ROOT_PATH)/cocos2dx_support/LuaCocos2d.cpp \
					$(LUA_ROOT_PATH)/cocos2dx_support/tolua_fix.c \
					$(LUA_ROOT_PATH)/CocosDenshion_support/LuaSimpleAudioEngine.cpp

LOCAL_C_INCLUDES := $(LUA_ROOT_PATH)/lua \
					$(LUA_ROOT_PATH)/tolua \
					$(LUA_ROOT_PATH)/CocosDenshion_support \
					$(PROJECT_ROOT_PATH)/cocos2dx \
                    $(PROJECT_ROOT_PATH)/cocos2dx/platform \
                    $(PROJECT_ROOT_PATH)/cocos2dx/include \
                    $(PROJECT_ROOT_PATH)/CocosDenshion/include

LOCAL_SHARED_LIBRARIES := cocos2dx_shared lua_shared cocosdenshion_shared
LOCAL_LDLIBS := -llog        
    
ifneq ($(APP_OPTIM),debug)
	LOCAL_CFLAGS += -O3 -DNDEBUG
else
	LOCAL_CFLAGS += -DDEBUG -DCOCOS2D_DEBUG=1
endif #($(APP_OPTIM),debug)
    
include $(BUILD_STATIC_LIBRARY)
