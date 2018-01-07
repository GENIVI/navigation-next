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

set(IPHONE_HEADERS "")
# todo: create function to add subfolders...
foreach(item ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

# append third party headers/sources
list(APPEND HEADER_PATH ${THIRDPARTY_ROOT_DIR}/sqlite/include)


# FIXME: files src/linux/uitaskmanager/ should be updated and move QT related
#        definitions into separated directory!
  find_package(Qt4 4.4.3 REQUIRED QtCore)
  include(${QT_USE_FILE})
  list(APPEND HEADER_PATH "${TOP_HEADER_PATH}/boost")

include_directories(${HEADER_PATH})

# 3. Adding source files...
message(STATUS "Platform: ${PLATFORM}")
file(GLOB_RECURSE COMMON_SOURCES
  "${NBPAL_ROOT_DIR}/src/common/*"
  "${NBPAL_ROOT_DIR}/src/${PLATFORM}/*"
  "${NBPAL_ROOT_DIR}/src/util/*"
  )

# Add platform specified source here.
file(GLOB_RECURSE ADDITIONAL_SOURCES
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/*"
  )

# Third party dependencies.
include_directories(
  "${NBPAL_ROOT_DIR}/src/posix/taskqueue/"
  "${THIRDPARTY_ROOT_DIR}/sqlite/include"
  "${THIRDPARTY_ROOT_DIR}/glues"
  )

foreach(item "${THIRDPARTY_ROOT_DIR}/sqlite/Sources")
  file(GLOB_RECURSE TMP_SOURCE "${item}/*")
  list(APPEND ADDITIONAL_SOURCES ${TMP_SOURCE})
endforeach()

get_freetype_info(FT_HEADERS FT_SOURCES)

# 5. Generate library.
add_library(nbpal ${LIB_TYPE} ${COMMON_SOURCES} ${ADDITIONAL_SOURCES} ${FT_SOURCES})

#@todo: we may need to compile zlib for some platforms.
target_link_libraries(nbpal z rt ${QT_LIBRARIES} X11)

if(UNIT_TEST)
  include (FindPkgConfig)
  pkg_check_modules(GLIB2 glib-2.0)
  if(NOT GLIB2_FOUND)
    message( FATAL_ERROR  "Unit Test requires glib-2.0, but it's not installed.")
  endif()

  include_directories(
    ${GLIB2_INCLUDE_DIRS}
    )
  list(APPEND LIBRARIES ${GLIB2_LIBRARIES})

  file(GLOB TEST_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/*.cpp"
    "${NBPAL_ROOT_DIR}/src/unittests/*.c")

  file(GLOB_RECURSE TEST_PLATFORM_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/${PLATFORM}/*.c"
    "${NBPAL_ROOT_DIR}/src/unittests/${PLATFORM}/*.cpp")

  include_directories("${NBPAL_ROOT_DIR}/src/unittests/")

  # add cunit..
  add_executable(nbpal_test  ${APP_TYPE}
    ${TEST_SOURCES} ${TEST_PLATFORM_SOURCES}  ${RESOURCES})

  target_link_libraries(nbpal_test nbpal cunit ${GLIB2_LIBRARIES})
endif(UNIT_TEST)

# 6. install...
install(
  TARGETS nbpal
  DESTINATION "${PACKAGE_DIR}/lib"
  )
