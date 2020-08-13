cmake_minimum_required(VERSION 3.17)
project(twainful)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreFoundation -w")

include_directories(lib)

add_executable(twainful
    src/Common.h
    src/CommonTwain.cpp
    src/CommonTwain.h
    src/DsmInterface.cpp
    src/DsmInterface.h
    src/main.cpp
    src/TwainApp.cpp
    src/TwainApp.h
    src/TwainString.cpp
    src/TwainString.h src/main.h src/TiffWriter.cpp src/TiffWriter.h)