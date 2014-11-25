ifeq ($(OS),Windows)
CMD_COPY=xcopy /S
CMD_MKDIR=mkdir
CMD_CHDIR=cd
CMD_EXTRACT=unzip
CMD_RMDIR=rm -rf
else
CMD_COPY=cp -r
CMD_MKDIR=mkdir
CMD_CHDIR=cd
CMD_EXTRACT=unzip
CMD_RMDIR=rm -rf
endif
ARCHIVE_PATH=android
ARCHIVE_FILE=glFlightAndroidBase.zip
FILES_RAW_TEXTURES=texture*.bmp
FILES_RAW_SOUNDS=*.wav
BUILD_DIR_IOS=build/ios
BUILD_DIR_ANDROID=build/android

all: android_workspace ios_workspace
	echo "done"

clean:
	${CMD_RMDIR} build/ios
	${CMD_RMDIR} build/android

workspace: ios_workspace android_workspace

base_ws:

ios_workspace: base_ws
	${CMD_MKDIR} ${BUILD_DIR_IOS}
	${CMD_COPY} Classes ${BUILD_DIR_IOS}
	${CMD_COPY} android ${BUILD_DIR_IOS}
	${CMD_COPY} iPad ${BUILD_DIR_IOS}
	${CMD_COPY} Shaders ${BUILD_DIR_IOS}
	${CMD_COPY} *.png ${BUILD_DIR_IOS}
	${CMD_COPY} *.jpg ${BUILD_DIR_IOS}
	${CMD_COPY} *.bmp ${BUILD_DIR_IOS}
	${CMD_COPY} *.wav ${BUILD_DIR_IOS}
	${CMD_COPY} *.xcodeproj ${BUILD_DIR_IOS}
	${CMD_COPY} *.m ${BUILD_DIR_IOS}
	${CMD_COPY} *.xcodeproj ${BUILD_DIR_IOS}
	${CMD_COPY} *.xib ${BUILD_DIR_IOS}
	${CMD_COPY} *.plist ${BUILD_DIR_IOS}
	${CMD_COPY} *.pch ${BUILD_DIR_IOS}
	${CMD_COPY} *.bundle ${BUILD_DIR_IOS}
	echo "iOS workspace:" ${BUILD_DIR_IOS}
	
android_workspace: base_ws
	PREV_DIR=${PWD}
	PROJ_DIR=${BUILD_DIR_ANDROID}/glFlight/
	PROJ_DIR_RES=${BUILD_DIR_ANDROID}/glFlight/res/raw/
	PROJ_DIR_JNI=${BUILD_DIR_ANDROID}/glFlight/jni/
	PROJ_DIR_GAME=${BUILD_DIR_ANDROID}/glFlight/jni/game/
	${CMD_MKDIR} ${BUILD_DIR_ANDROID}
	${CMD_COPY} ${ARCHIVE_PATH}/${ARCHIVE_FILE} ${BUILD_DIR_ANDROID}
	${CMD_CHDIR} ${BUILD_DIR_ANDROID}; ${CMD_EXTRACT} ${ARCHIVE_FILE}
	${CMD_COPY} android/ ${BUILD_DIR_ANDROID}/glFlight/
	${CMD_COPY} Classes/ ${BUILD_DIR_ANDROID}/glFlight/jni/game/
	${CMD_COPY} *.png ${BUILD_DIR_ANDROID}/glFlight/res/raw/
	${CMD_COPY} *.jpg ${BUILD_DIR_ANDROID}/glFlight/res/raw/
	${CMD_COPY} *.bmp ${BUILD_DIR_ANDROID}/glflight/res/raw/ 
	${CMD_COPY} *.wav ${BUILD_DIR_ANDROID}/glFlight/res/raw/ 
	echo "android workspace: " ${BUILD_DIR_ANDROID}

