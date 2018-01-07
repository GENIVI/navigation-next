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

message(STATUS "Configuring for nbpal -- ${PLATFORM}...")

# 1. Preparing variables...
set(NBPAL_ROOT_DIR ${PROJECT_SOURCE_DIR}/nbpal/)

message(STATUS "NBPAL DIR:" ${NBPAL_ROOT_DIR})
message(STATUS "CCC ROOT:" ${PROJECT_SOURCE_DIR})
message(STATUS "BRANCH:" ${BRANCH})

# 2. Adding header search pathes

set(TOP_HEADER_PATH "${NBPAL_ROOT_DIR}/include/")
set(HEADER_PATH "${NBPAL_ROOT_DIR}/include/")

file(GLOB INC_FOLDERS
  "${NBPAL_ROOT_DIR}/include/"
  "${NBPAL_ROOT_DIR}/include/${PLATFORM}"
  "${NBPAL_ROOT_DIR}/include/private"
  "${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/include"
  )

foreach(item ${INC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

include_directories(${HEADER_PATH})

# 3. Help functions...

# @todo: should moved into nbgm.
function (get_freetype_info HEADER SOURCES)
  add_definitions("-DFT2_BUILD_LIBRARY")
  set(${HEADER} "${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/include" PARENT_SCOPE)
  set(${SOURCES}
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/autofit/autofit.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftadvanc.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftbbox.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftbitmap.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftcalc.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftcid.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftdbgmem.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftdebug.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftfstype.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftgasp.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftgloadr.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftglyph.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftgxval.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftinit.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftlcdfil.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftmm.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftobjs.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftotval.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftoutln.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftpatent.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftpfr.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftrfork.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftsnames.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftstream.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftstroke.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftsynth.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftsystem.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/fttrigon.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/fttype1.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftutil.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftwinfnt.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/base/ftxf86.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/bdf/bdf.c
    # ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/bzip2/ftbzip2.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/cache/ftcache.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/cff/cff.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/cid/type1cid.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/gzip/ftgzip.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/lzw/ftlzw.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/pcf/pcf.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/pfr/pfr.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/psaux/psaux.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/pshinter/pshinter.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/psnames/psmodule.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/raster/raster.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/sfnt/sfnt.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/smooth/smooth.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/truetype/truetype.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/type1/type1.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/type42/type42.c
    ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/src/winfonts/winfnt.c
    PARENT_SCOPE)
endfunction()

# 4. Add platform specified cmakes.

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM})
  add_subdirectory(${PLATFORM})
else()
  message(FATAL_ERROR "Can't find project for paltform ${PLATFORM}...")
endif ()

# 5. install...
file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

install(
  DIRECTORY "${TOP_HEADER_PATH}/${PLATFORM}"
  DESTINATION "${PACKAGE_DIR}/include/"
  )
