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
set(CORESERVICES_ROOT_DIR ${PROJECT_SOURCE_DIR}/coreservices)

message(STATUS "")
message(STATUS "Configuring for coreservices:")
message(STATUS "\t\tCORESERVICES DIR:" ${CORESERVICES_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes
get_parent(PROJECT_SOURCE_DIR DEPOT_ROOT 3)
set(NBM_RELEASE "${DEPOT_ROOT}/core/releases/nbm/${PLATFORM}/nbm_${NBM_VERSION}_${PLATFORM}")

file(GLOB PUB_HEADERS
  "${CORESERVICES_ROOT_DIR}/include/*.h"
  )

file(GLOB PLATFORM_HEADER
  "${CORESERVICES_ROOT_DIR}/include/${PLATFORM}/*.h"
  )

set(HEADER_PATH "${CORESERVICES_ROOT_DIR}/include"
  "${CORESERVICES_ROOT_DIR}/include/protected"
  "${CORESERVICES_ROOT_DIR}/include/private"
  "${CORESERVICES_ROOT_DIR}/include/${PLATFORM}"
  )

# @TRICKY: remove unit test from build path...
file(GLOB SRC_FOLDERS
  "${CORESERVICES_ROOT_DIR}/src/*"
  "${CORESERVICES_ROOT_DIR}/src/*/*")

# todo: create function to add subfolders...
foreach(item ${INC_FOLDERS} ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

foreach(item "${PROJECT_SOURCE_DIR}/nbpal")
  prepare_headers("${item}/include" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB SOURCE_DIRS "${CORESERVICES_ROOT_DIR}/src/*")
list(REMOVE_ITEM SOURCE_DIRS "${CORESERVICES_ROOT_DIR}/src/unittests")
file(GLOB SOURCES "${CORESERVICES_ROOT_DIR}/src/*.c")
foreach(item ${SOURCE_DIRS})
  file(GLOB TMP_SOURCES "${item}/*.cpp"  "${item}/*.c")
  list(APPEND SOURCES ${TMP_SOURCES})
endforeach()

# 4. Set compiling flags
add_definitions("-DUNUSED_NDEFINED")
add_definitions("-DCSL_BUILD")

# 5. Generate library.
add_library(coreservices ${LIB_TYPE} ${SOURCES})
if (NOT STATIC_LIB)
  target_link_libraries(coreservices palbase)
endif ()

# 6. install...
install(
  TARGETS coreservices
  # LIBRARY
  DESTINATION "${PACKAGE_DIR}/lib"
  )

install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

install(
  FILES ${PLATFORM_HEADER}
  DESTINATION "${PACKAGE_DIR}/include/${PLATFORM}/"
  )


if(UNIT_TEST)
  #todo: add codes to compile unit test.
  message(STATUS "Not implemented yet...")
endif(UNIT_TEST)
