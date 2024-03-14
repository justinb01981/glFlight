LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := glFlight
LOCAL_LDLIBS := -lEGL -lGLESv1_CM -llog
LOCAL_SRC_FILES := glFlightJNI.cpp stubs.cpp

GAME_SRC := ../../../../../../../Classes
LOCAL_C_INCLUDES := $(LOCAL_PATH)/${GAME_SRC}
DEFINEFLAGS := -DEXPERIMENTAL=1 -DDEBUG=1

LOCAL_CFLAGS := ${LOCAL_CFLAGS} -Wno-narrowing -Wno-reserved-user-defined-literal -Wno-ignored-qualifiers -Wno-writable-strings -Wno-implicit-const-int-float-conversion -Wno-incompatible-pointer-types-discards-qualifiers ${DEFINEFLAGS}

LOCAL_SRC_FILES += \
    ${GAME_SRC}/world.c \
    ${GAME_SRC}/worldElem.c \
    ${GAME_SRC}/gameUtils.c \
    ${GAME_SRC}/gameBounding.c \
    ${GAME_SRC}/gameCamera.c \
    ${GAME_SRC}/gameShip.c \
    ${GAME_SRC}/quaternions.c \
    ${GAME_SRC}/mesh.c \
    ${GAME_SRC}/gameAudio.c \
    ${GAME_SRC}/actions.c \
    ${GAME_SRC}/textures.c \
    ${GAME_SRC}/gameGraphics.c \
    ${GAME_SRC}/glFlight.c \
    ${GAME_SRC}/gameNetwork.c \
    ${GAME_SRC}/gameAI.c \
    ${GAME_SRC}/gamePlay.c \
    ${GAME_SRC}/world_file.c \
    ${GAME_SRC}/gameInterface.c \
    ${GAME_SRC}/gameInput.c \
    ${GAME_SRC}/gameSettings.c \
    ${GAME_SRC}/collision.c \
    ${GAME_SRC}/gameDialogs.c \
    ${GAME_SRC}/gameMotionInterp.c \
    ${GAME_SRC}/mapgenerated.c



include $(BUILD_SHARED_LIBRARY)
