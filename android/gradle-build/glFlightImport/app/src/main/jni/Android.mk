LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := glFlight
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -lEGL

##LOCAL_CFLAGS += -IC:\Users\justin\android\android-ndk-r9d\platforms\android-19\arch-arm\usr\include

LOCAL_SRC_FILES += game/world.c game/worldElem.c game/gameUtils.c game/gameBounding.c game/gameCamera.c game/gameShip.c game/quaternions.c game/mesh.c game/gameAudio.c game/actions.c game/textures.c game/gameGraphics.c game/glFlight.c game/gameNetwork.c game/gameAI.c game/gamePlay.c game/world_file.c game/gameInterface.c game/gameInput.c game/gameSettings.c game/collision.c game/gameDialogs.c game/gameMotionInterp.c game/mapgenerated.c game/framebuffer.c
LOCAL_CFLAGS = -g -DEXPERIMENTAL=1
LOCAL_CFLAGS += -w -fpermissive
LOCAL_CPPFLAGS = -w -fpermissive -Wno-narrowing -Wno-reserved-user-defined-literal -Wno-discarded-qualifiers -g

LOCAL_SRC_FILES += glFlightJNI.cpp stubs.cpp

include $(BUILD_SHARED_LIBRARY)
