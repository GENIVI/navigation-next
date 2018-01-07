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

# 1. Preparing variables...
get_freetype_info(FT_HEADERS FT_SOURCES)

# 2. Adding header search pathes
file(GLOB PRIVATE_HEADER_PATH
  "${NBPAL_ROOT_DIR}/include/private"
  "${FT_HEADERS}"
  "${THIRDPARTY_ROOT_DIR}/sqlite/include"
  "${THIRDPARTY_ROOT_DIR}/glew"
  "${NBPAL_ROOT_DIR}/src/common/taskqueue"
  "${NBPAL_ROOT_DIR}/src/unittests"
  "${NBPAL_ROOT_DIR}/src/win32/*"
  "${NBPAL_ROOT_DIR}/src/winmobile/database"
  "${NBPAL_ROOT_DIR}/src/winmobile/file"
  "${NBPAL_ROOT_DIR}/src/winmobile/pal"
  "${NBPAL_ROOT_DIR}/src/winmobile/timer"
  "${NBPAL_ROOT_DIR}/src/winmobile/util"
  "${NBPAL_ROOT_DIR}/src/winmobile/net"
  )

include_directories(${PRIVATE_HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE BASE_SOURCES
  "${NBPAL_ROOT_DIR}/src/common/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/util/*"
  "${NBPAL_ROOT_DIR}/src/win32/clock/*"
  "${NBPAL_ROOT_DIR}/src/win32/myplace/*"
  "${NBPAL_ROOT_DIR}/src/win32/radio/*"
  "${NBPAL_ROOT_DIR}/src/win32/synchronization/*"
  "${NBPAL_ROOT_DIR}/src/win32/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/win32/testlog/*"
  "${NBPAL_ROOT_DIR}/src/win32/unzip/*"
  "${NBPAL_ROOT_DIR}/src/win32/util/*"
  "${NBPAL_ROOT_DIR}/src/win32/pallog.c"
  "${NBPAL_ROOT_DIR}/src/winmobile/database/*"
  "${NBPAL_ROOT_DIR}/src/winmobile/file/*"
  "${NBPAL_ROOT_DIR}/src/winmobile/pal/*"
  "${NBPAL_ROOT_DIR}/src/winmobile/timer/*"
  "${NBPAL_ROOT_DIR}/src/winmobile/util/palstdlib.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/util/paldebuglog.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/connectionmanager.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/httpconnection.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/httprequest.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/netconnection.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/palnet.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/tcpconnection.cpp"
  "${NBPAL_ROOT_DIR}/src/winmobile/net/wsaconnection.cpp"
  "${THIRDPARTY_ROOT_DIR}/sqlite/Sources/*"
  )

file(GLOB_RECURSE GRAPHICS_SOURCE
  "${NBPAL_ROOT_DIR}/src/win32/font/*"
  "${NBPAL_ROOT_DIR}/src/win32/imageblender/*"
  "${NBPAL_ROOT_DIR}/src/win32/triangulation/*"
  "${THIRDPARTY_ROOT_DIR}/glew/*"
  )

# 4. Set compiling flags
add_definitions("-DFT2_BUILD_LIBRARY")
add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")
add_definitions("-DWIN32")
add_definitions("-D_LIB")
add_definitions("-D_CRT_SECURE_NO_DEPRECATE")
add_definitions("-DWIN32_LEAN_AND_MEAN")
add_definitions("-DPAL_BUILD")

# 5. Generate library.
#@todo: we may need to compile zlib for some platforms.
link_directories(${NBPAL_ROOT_DIR}/src/win32/unzip)

add_library(palbase ${LIB_TYPE} ${BASE_SOURCES} )
add_library(palgraphics ${LIB_TYPE} ${GRAPHICS_SOURCE} ${FT_SOURCES})

if(NOT STATIC_LIB)
  target_link_libraries(palbase ws2_32 wininet Shlwapi zlib_d)
  target_link_libraries(palgraphics GlU32 zlib_d)
endif()

if(UNIT_TEST)
  file(GLOB TEST_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/*.h"
    "${NBPAL_ROOT_DIR}/src/unittests/*.cpp"
    "${NBPAL_ROOT_DIR}/src/unittests/*.c")

   file(GLOB_RECURSE TEST_PLATFORM_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/win32/PlatformUtil.c")

  include_directories("${NBPAL_ROOT_DIR}/src/unittests/")

  set(APP_TYPE MACOSX_BUNDLE)
  # add cunit..
  add_executable(nbpal_test  ${APP_TYPE} ${TEST_SOURCES} ${TEST_PLATFORM_SOURCES})

  target_link_libraries(nbpal_test palbase palgraphics cunit)

  set_target_properties(nbpal_test PROPERTIES SUBSYSTEM "CONSOLE")
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