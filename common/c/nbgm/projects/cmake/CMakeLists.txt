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
set(NBGM_ROOT_DIR ${PROJECT_SOURCE_DIR}/nbgm)

message(STATUS "")
message(STATUS "Configuring for nbgm:")
message(STATUS "\t\tNBGM DIR:" ${NBGM_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes
get_parent(PROJECT_SOURCE_DIR DEPOT_ROOT 3)

file(GLOB PLATFORM_HEADER "${NBGM_ROOT_DIR}/nbgmmain/include/${PLATFORM}/*.h")
set(TOP_HEADER_PATH ${NBGM_ROOT_DIR}/nbgmmain/include)
set(HEADER_PATH
  ${TOP_HEADER_PATH}
  ${TOP_HEADER_PATH}/protected
  ${TOP_HEADER_PATH}/private
  ${TOP_HEADER_PATH}/${PLATFORM}
  ${NBGM_ROOT_DIR}/nbre/include
  ${NBGM_ROOT_DIR}/nbre/include/private
  ${NBGM_ROOT_DIR}/nbre/include/debug
  ${THIRDPARTY_ROOT_DIR}/libjpeg_turbo/include
  ${THIRDPARTY_ROOT_DIR}/lp150b15
  ${THIRDPARTY_ROOT_DIR}/zlib
  ${THIRDPARTY_ROOT_DIR}/libjpeg/include
  ${THIRDPARTY_ROOT_DIR}/libjpeg/include/${PLATFORM}
  ${THIRDPARTY_ROOT_DIR}/freetype-2.4.4/include)

if (PLATFORM STREQUAL "win32")
  list(APPEND HEADER_PATH
    ${NBGM_ROOT_DIR}/rendersystem/d3d9/include
    ${NBGM_ROOT_DIR}/rendersystem/d3d9/include/private
    ${NBGM_ROOT_DIR}/rendersystem/gl/include
    ${NBGM_ROOT_DIR}/rendersystem/gl/include/private
    ${THIRDPARTY_ROOT_DIR}/glew-1.7.0/include
    )
else ()
  list(APPEND HEADER_PATH
    ${NBGM_ROOT_DIR}/rendersystem/gles/include
    ${NBGM_ROOT_DIR}/rendersystem/gles/include/private
    ${NBGM_ROOT_DIR}/rendersystem/gles2/include
    ${NBGM_ROOT_DIR}/rendersystem/gles2/include/private
    )
endif ()

# @TRICKY: remove unit test from build path...
# file(GLOB SRC_FOLDERS
#   "${NBGM_ROOT_DIR}/src/*"
#   "${NBGM_ROOT_DIR}/src/*/*")

# # todo: create function to add subfolders...
# foreach(item ${INC_FOLDERS} ${SRC_FOLDERS})
#   if(IS_DIRECTORY ${item})
#     list(APPEND HEADER_PATH ${item})
#   endif(IS_DIRECTORY ${item})
# endforeach()

foreach(item "${PROJECT_SOURCE_DIR}/nbpal")
  prepare_headers("${item}/include" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB SOURCES
  "${NBGM_ROOT_DIR}/nbgmmain/src/*.cpp"
  "${NBGM_ROOT_DIR}/nbgmmain/src/util/*.cpp"
  "${NBGM_ROOT_DIR}/nbgmmain/src/nbgmmanager/*.cpp"
  "${NBGM_ROOT_DIR}/nbgmmain/src/nbgmmapdata/*.cpp"
  "${NBGM_ROOT_DIR}/nbgmmain/src/nbgmservice/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/core/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/debug/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/nbre/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/renderpal/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/resource/*.cpp"
  "${NBGM_ROOT_DIR}/nbre/src/scene/*.cpp"
  "${THIRDPARTY_ROOT_DIR}/libjpeg/src/*.c"
  "${THIRDPARTY_ROOT_DIR}/libjpeg_turbo/src/*.c"
  )

if (PLATFORM STREQUAL "win32")
  file(GLOB  TMP_SOURCES
    "${NBGM_ROOT_DIR}/rendersystem/d3d9/src/*.cpp"
    "${NBGM_ROOT_DIR}/rendersystem/gl/src/*.cpp")
else ()
  file(GLOB_RECURSE  TMP_SOURCES
    "${NBGM_ROOT_DIR}/rendersystem/gles2/src/*.cpp"
    "${NBGM_ROOT_DIR}/rendersystem/gles/src/*.cpp")
endif ()

list(APPEND SOURCES ${TMP_SOURCES})

# 4. Set compiling flags

# 5. Generate library.
add_library(nbgm ${LIB_TYPE} ${SOURCES})
if (NOT STATIC_LIB)
 #@todo: figure out if GLESv2 is used by other platforms..
  target_link_libraries(nbgm palbase palgraphics GLESv2 EGL android GLESv1_CM)
endif ()

# 6. install...
install(
  TARGETS nbgm
  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

install(
  FILES ${PLATFORM_HEADER}
  DESTINATION "${PACKAGE_DIR}/include/${PLATFORM}"
  )

if(UNIT_TEST)
  message(STATUS "Not implemented yet...")
endif (UNIT_TEST)
