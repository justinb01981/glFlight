################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/game/actions.c \
../jni/game/gameAI.c \
../jni/game/gameAudio.c \
../jni/game/gameBounding.c \
../jni/game/gameCamera.c \
../jni/game/gameDialogs.c \
../jni/game/gameGraphics.c \
../jni/game/gameInput.c \
../jni/game/gameInterface.c \
../jni/game/gameNetwork.c \
../jni/game/gamePlay.c \
../jni/game/gameSettings.c \
../jni/game/gameShip.c \
../jni/game/gameUtils.c \
../jni/game/glFlight.c \
../jni/game/mesh.c \
../jni/game/quaternions.c \
../jni/game/textures.c \
../jni/game/world.c \
../jni/game/worldElem.c \
../jni/game/world_file.c 

OBJS += \
./jni/game/actions.o \
./jni/game/gameAI.o \
./jni/game/gameAudio.o \
./jni/game/gameBounding.o \
./jni/game/gameCamera.o \
./jni/game/gameDialogs.o \
./jni/game/gameGraphics.o \
./jni/game/gameInput.o \
./jni/game/gameInterface.o \
./jni/game/gameNetwork.o \
./jni/game/gamePlay.o \
./jni/game/gameSettings.o \
./jni/game/gameShip.o \
./jni/game/gameUtils.o \
./jni/game/glFlight.o \
./jni/game/mesh.o \
./jni/game/quaternions.o \
./jni/game/textures.o \
./jni/game/world.o \
./jni/game/worldElem.o \
./jni/game/world_file.o 

C_DEPS += \
./jni/game/actions.d \
./jni/game/gameAI.d \
./jni/game/gameAudio.d \
./jni/game/gameBounding.d \
./jni/game/gameCamera.d \
./jni/game/gameDialogs.d \
./jni/game/gameGraphics.d \
./jni/game/gameInput.d \
./jni/game/gameInterface.d \
./jni/game/gameNetwork.d \
./jni/game/gamePlay.d \
./jni/game/gameSettings.d \
./jni/game/gameShip.d \
./jni/game/gameUtils.d \
./jni/game/glFlight.d \
./jni/game/mesh.d \
./jni/game/quaternions.d \
./jni/game/textures.d \
./jni/game/world.d \
./jni/game/worldElem.d \
./jni/game/world_file.d 


# Each subdirectory must supply rules for building sources it contributes
jni/game/%.o: ../jni/game/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


