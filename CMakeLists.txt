cmake_minimum_required(VERSION 3.17)
project(twainful)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreFoundation -w")

# check for freeimage
find_package(FreeImage)
if(FreeImage_FOUND)
    set(STATUS_FREEIMG "OK")
else(FreeImage_FOUND)
    set(STATUS_FREEIMG "not found")
endif(FreeImage_FOUND)

include_directories(include)
include_directories(${FreeImage_INCLUDE_DIRS})

add_executable(twainful
    src/Common.h
    src/CommonTwain.cpp
    src/CommonTwain.h
    src/DsmInterface.cpp
    src/DsmInterface.h
    src/main.cpp
    src/main.h
    src/TiffWriter.cpp
    src/TiffWriter.h
    src/TwainApp.cpp
    src/TwainApp.h
    src/TwainString.cpp
    src/TwainString.h)

target_link_libraries(twainful ${FreeImage_LIBRARIES})