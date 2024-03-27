cmake_minimum_required(VERSION 3.13)
project(glFlight)
set(CMAKE_CXX_STANDARD 14)

FILE(GLOB GAME_SRC ../Classes/*.c)
FILE(GLOB FRAMEWORK_SUPPORT ogl-samples-master/framework/*.cpp)
set(SOURCE_FILES ogl-glflight/stubs.cpp ogl-glflight/gameWinStubs.c ogl-glflight/glFlightMain.cpp ogl-glflight/gameFramework.cpp ogl-glflight/gameAudioOpenAL.cpp ogl-glflight/gameJoystick.cpp ${GAME_SRC})

include_directories(ogl-glflight)
include_directories(../Classes)

add_definitions(/FI"gameWinTypes.h")

add_definitions(-fpermissive)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DGAME_PLATFORM_OGL=1)
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DGLEW_STATIC)
add_definitions(-D_NOT_POSIX)

set(OGL_DIR ogl-samples-master)

add_subdirectory(${OGL_DIR})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(PLATFORM "win64")
set(EGL_DIRECTORY egl-1.4)
set(ES_DIRECTORY gles-2.0)
set(GLFW_DIRECTORY glfw-3.1.1)
set(FREEIMAGE_DIRECTORY FreeImage-3.15.4)
set(OPENAL_DIRECTORY "C:\\Program Files (x86)\\OpenAL 1.1 SDK")

link_directories(${OGL_DIR}/external/${EGL_DIRECTORY}/lib/${PLATFORM})
include_directories(${OGL_DIR}/external/${EGL_DIRECTORY})

link_directories(${OGL_DIR}/external/${ES_DIRECTORY}/lib/${PLATFORM})
include_directories(${OGL_DIR}/external/${ES_DIRECTORY})

link_directories(${OGL_DIR}/external/${FREEIMAGE_DIRECTORY}/${PLATFORM})

link_directories(${OGL_DIR}/external/${GLFW_DIRECTORY}/src)

include_directories(${OGL_DIR}/framework)
link_directories(${OGL_DIR}/external/framework)

include_directories(${OPENAL_DIRECTORY}/include)
link_directories(${OPENAL_DIRECTORY}/libs/${PLATFORM})

include_directories(${OGL_DIR}/external)
include_directories(${OGL_DIR}/external/glfw-3.1.1/include)
include_directories(${OGL_DIR}/external/glfw-3.1.1/deps)
include_directories(${OGL_DIR}/external/glu-0.1.0.0/glu)

add_executable(glFlightMain ${SOURCE_FILES})

target_link_libraries(glFlightMain framework libEGL FreeImage glfw3 OpenAL32)
target_link_libraries(glFlightMain wsock32 ws2_32 winmm)
target_link_libraries(glFlightMain opengl32)

#add_dependencies(glFlightMain ${OGL_DIR})

install(TARGETS glFlightMain DESTINATION glFlight.build)

file(GLOB resourcesBmp ../*.bmp)
file(GLOB resourcesWav ../*.wav)

foreach(resFile ${resourcesBmp})
    add_custom_command(TARGET glFlightMain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${resFile} resources)
endforeach(resFile)

foreach(resFile ${resourcesWav})
    add_custom_command(TARGET glFlightMain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${resFile} resources)
endforeach(resFile)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/joy_config.txt
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(ZIP_COMMAND tar -czf out.zip)

#add_custom_command(TARGET glFlightMain POST_BUILD
#    COMMAND echo  > out.txt)


