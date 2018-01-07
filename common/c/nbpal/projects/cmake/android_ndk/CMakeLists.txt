##############################################################################
##  @file        CMakeLists.txt
##  @defgroup    Makefiles
##
##  Copyright (C) 2014 TeleCommunication Systems, Inc.
##
##  The information contained herein is confidential, proprietary to
##  TeleCommunication Systems, Inc., and considered a trade secret as defined
##  in section 499C of the penal code of the State of California. Use of this
##  information by anyone other than authorized employees of TeleCommunication
##  Systems is granted only under a written non-disclosure agreement, expressly
##  prescribing the scope and manner of such use.
##############################################################################

file(GLOB SRC_FOLDERS
  "${NBPAL_ROOT_DIR}/src/common/*"
  "${NBPAL_ROOT_DIR}/src/${PLATFORM}/*"
  )

set(HEADER_PATH
  ${NBPAL_ROOT_DIR}/include
  ${NBPAL_ROOT_DIR}/include/protected
  ${NBPAL_ROOT_DIR}/include/android_ndk
  ${NBPAL_ROOT_DIR}/include/private
  ${NBPAL_ROOT_DIR}/include/posix
  ${NBPAL_ROOT_DIR}/src/android_ndk
  ${NBPAL_ROOT_DIR}/src/android_ndk/net
  ${NBPAL_ROOT_DIR}/src/android_ndk/pal
  ${NBPAL_ROOT_DIR}/src/android_ndk/taskqueue
  ${NBPAL_ROOT_DIR}/src/android_ndk/util
  ${NBPAL_ROOT_DIR}/src/android_ndk/imageblender
  ${NBPAL_ROOT_DIR}/src/android_ndk/jni
  ${NBPAL_ROOT_DIR}/src/android_ndk/synchronization
  ${NBPAL_ROOT_DIR}/src/common
  ${NBPAL_ROOT_DIR}/src/common/database
  ${NBPAL_ROOT_DIR}/src/common/finishfunction
  ${NBPAL_ROOT_DIR}/src/common/taskqueue
  ${NBPAL_ROOT_DIR}/src/posix/taskqueue
  ${NBPAL_ROOT_DIR}/src/posix/util
  ${NBPAL_ROOT_DIR}/src/common/unzip
  ${NBPAL_ROOT_DIR}/src/common/posix
  ${NBPAL_ROOT_DIR}/src/posix/net
  ${NBPAL_ROOT_DIR}/src/posix/taskqueue
  ${NBPAL_ROOT_DIR}/src/posix/timer
  ${THIRDPARTY_ROOT_DIR}/sqlite/include
  ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/include
  ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/include/freetype
  ${THIRDPARTY_ROOT_DIR}/lp150b15
  ${THIRDPARTY_ROOT_DIR}/libjpeg/include
  ${THIRDPARTY_ROOT_DIR}/libjpeg/include/android
  ${THIRDPARTY_ROOT_DIR}/libjpeg_turbo/include
  ${THIRDPARTY_ROOT_DIR}/speex/public
  ${THIRDPARTY_ROOT_DIR}/speex/android_ndk
  ${THIRDPARTY_ROOT_DIR}/speex/speex-1.2rc1/include
  ${THIRDPARTY_ROOT_DIR}/speex/speex-1.2rc1/include/speex
  ${THIRDPARTY_ROOT_DIR}/speex/speex-1.2rc1/libspeex
  ${THIRDPARTY_ROOT_DIR}/speex/libogg-1.3.0/include/ogg
  ${THIRDPARTY_ROOT_DIR}/speex/libogg-1.3.0/include
  ${THIRDPARTY_ROOT_DIR}/glues)

if (ANDROID_STL STREQUAL "stlport_shared")
  set(USE_STLPORT 1)
  list(APPEND HEADER_PATH "${NBPAL_ROOT_DIR}/include/boost")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDK_CXX_FLAGS}")
endif ()

configure_file("${NBPAL_ROOT_DIR}/include/android_ndk/smartpointer.h.in"
  "${NBPAL_ROOT_DIR}/include/android_ndk/smartpointer.h"
  )

include_directories(${HEADER_PATH})

# 3. Adding source files...
message(STATUS "Platform: ${PLATFORM}")
file(GLOB_RECURSE BASE_SOURCES
  "${NBPAL_ROOT_DIR}/src/android_ndk/jni/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/net/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/pal/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/radio/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/util/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/synchronization/*"
  "${NBPAL_ROOT_DIR}/src/common/database/*"
  "${NBPAL_ROOT_DIR}/src/common/finishfunction/*"
  "${NBPAL_ROOT_DIR}/src/common/posix/*"
  "${NBPAL_ROOT_DIR}/src/common/unzip/*"
  "${NBPAL_ROOT_DIR}/src/common/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/posix/clock/*"
  "${NBPAL_ROOT_DIR}/src/posix/file/*"
  "${NBPAL_ROOT_DIR}/src/posix/net/*"
  "${NBPAL_ROOT_DIR}/src/posix/synchronization/*"
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/posix/timer/*"
  "${NBPAL_ROOT_DIR}/src/posix/pal/*"
  "${NBPAL_ROOT_DIR}/src/util/*"
  "${THIRDPARTY_ROOT_DIR}/sqlite/Sources/*"
  )

list(REMOVE_ITEM BASE_SOURCES
  "${NBPAL_ROOT_DIR}/src/android_ndk/pal/wm_global.cpp"
  "${NBPAL_ROOT_DIR}/src/posix/net/palnet.cpp"
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/taskmanagermultithreaded.cpp"
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/threadposix.cpp"
  "${NBPAL_ROOT_DIR}/src/android_ndk/radio/com_navbuilder_pal_android_ndk_RadioInterface.cpp"
  )

file(GLOB_RECURSE GRAPHICS_SOURCES
  "${NBPAL_ROOT_DIR}/src/common/gl/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/font/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/gl/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/imageblender/*"
  "${NBPAL_ROOT_DIR}/src/android_ndk/triangulation/*"
  "${THIRDPARTY_ROOT_DIR}/lp150b15/*"
  "${THIRDPARTY_ROOT_DIR}/libjpeg/src/*"
  "${THIRDPARTY_ROOT_DIR}/libjpeg_turbo/src/*"
  #"${THIRDPARTY_ROOT_DIR}/speex/libogg-1.3.0/src/*"
  #"${THIRDPARTY_ROOT_DIR}/speex/private/*"
  #"${THIRDPARTY_ROOT_DIR}/speex/speex-1.2rc1/libspeex/*.c"
  "${THIRDPARTY_ROOT_DIR}/glues/*"
  )

list(REMOVE_ITEM GRAPHICS_SOURCES
  "${THIRDPARTY_ROOT_DIR}/lp150b15/pngtest.c"
  )

# Third party dependencies.
include_directories(
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/"
  "${THIRDPARTY_ROOT_DIR}/sqlite/include"
  "${THIRDPARTY_ROOT_DIR}/glues"
  )

get_freetype_info(FT_HEADERS FT_SOURCES)

# 5. Generate library.
add_definitions("-DANDROID_NDK")
add_library(palbase ${LIB_TYPE} ${BASE_SOURCES})
add_library(palgraphics ${LIB_TYPE} ${GRAPHICS_SOURCES} ${FT_SOURCES})

if(NOT STATIC_LIB)
  target_link_libraries(palbase z log)
  target_link_libraries(palgraphics z GLESv2 EGL android GLESv1_CM)
endif()

if(UNIT_TEST)
  message(STATUS "Not implemented...")
endif(UNIT_TEST)

# 6. install...
install(
  TARGETS palbase
  DESTINATION "${PACKAGE_DIR}/lib"
  )

install(
  TARGETS palgraphics
  DESTINATION "${PACKAGE_DIR}/lib"
  )