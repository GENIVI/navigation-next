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
set(MAP_ROOT_DIR ${PROJECT_SOURCE_DIR}/map)

message(STATUS "Configuring for map:")
message(STATUS "\t\tMAP DIR:" ${MAP_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes

set(TOP_HEADER_PATH "${MAP_ROOT_DIR}/include")
set(HEADER_PATH "${MAP_ROOT_DIR}/include")

prepare_headers("${PROJECT_SOURCE_DIR}/nbgm/nbgmmain/include" HEADER_PATH)

file(GLOB COMPONENTS "${PROJECT_SOURCE_DIR}/*")
foreach(item ${COMPONENTS})
  prepare_headers("${item}/include" HEADER_PATH)
endforeach()

# FIXME: Header search path of Components in client/c/core should be limited to:
#   1. Public header of other components of same level: nbpal/include .
#   2. Protected header of other components of same level: nbpal/include/protected/ .
#   3. Platform header of other components of same level: nbpal/include/iPhone/ .
#   It should not access any private headers (nbpal/include/private) or header files
#   located in src folder.
include_directories(${PROJECT_SOURCE_DIR}/nbservices/src/map/layermanager)
include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES "${MAP_ROOT_DIR}/src/*.cpp")

# 4. Generate library.
# add_definitions("-DNB_BUILD")
add_library(map ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(map nbmap)
endif ()

# 5. install...
install(
  TARGETS map
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

if(UNIT_TEST)
  #todo: add codes to compile unit test.
  message(STATUS "Not implemented yet...")
endif(UNIT_TEST)