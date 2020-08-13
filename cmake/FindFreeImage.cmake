# Find FreeImage includes and library
#
# This module defines
#  FreeImage_INCLUDE_DIRS
#  FreeImage_LIBRARIES, the libraries to link against to use FreeImage.
#  FreeImage_LIBRARY_DIRS, the location of the libraries
#  FreeImage_FOUND, If false, do not try to use FreeImage
#
# Copyright © 2007, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (FreeImage_LIBRARIES AND FreeImage_INCLUDE_DIRS)
    set(FreeImage_FIND_QUIETLY TRUE) # Already in cache, be silent
else (FreeImage_LIBRARIES AND FreeImage_INCLUDE_DIRS)
    message(STATUS "Looking for FreeImage")
endif (FreeImage_LIBRARIES AND FreeImage_INCLUDE_DIRS)

set(FreeImage_INCLUDE_SEARCH_DIRS
    ${FreeImage_LIBRARY_SEARCH_DIRS}
    ${CMAKE_LIBRARY_PATH}
    /usr/include
    /usr/local/include
    /opt/include
    /opt/freeimage/include
    )

set(FreeImage_LIBRARY_SEARCH_DIRS
    ${FreeImage_LIBRARY_SEARCH_DIRS}
    ${CMAKE_LIBRARY_PATH}
    /usr/lib
    /usr/local/lib
    /opt/lib
    /opt/freeimage/lib
    )

find_path(FreeImage_INCLUDE_DIRS FreeImage.h ${FreeImage_INCLUDE_SEARCH_DIRS})
find_library(FreeImage_LIBRARIES freeimage PATHS ${FreeImage_LIBRARY_SEARCH_DIRS})

#Do some preparation
separate_arguments(FreeImage_INCLUDE_DIRS)
separate_arguments(FreeImage_LIBRARIES)

mark_as_advanced(FreeImage_INCLUDE_DIRS FreeImage_LIBRARIES FreeImage_LIBRARY_DIRS)

if (FreeImage_INCLUDE_DIRS AND FreeImage_LIBRARIES)
    set(FreeImage_FOUND TRUE)
endif (FreeImage_INCLUDE_DIRS AND FreeImage_LIBRARIES)

if (FreeImage_FOUND)
    if (NOT FreeImage_FIND_QUIETLY)
        MESSAGE(STATUS "  libraries : ${FreeImage_LIBRARIES} from ${FreeImage_LIBRARY_DIRS}")
        MESSAGE(STATUS "  includes  : ${FreeImage_INCLUDE_DIRS}")
    endif (NOT FreeImage_FIND_QUIETLY)
else (FreeImage_FOUND)
    if (FreeImage_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find FreeImage")
    endif (FreeImage_FIND_REQUIRED)
endif (FreeImage_FOUND)