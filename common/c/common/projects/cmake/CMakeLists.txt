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
set(NBCOMMON_ROOT_DIR ${PROJECT_SOURCE_DIR}/common)

message(STATUS "Configuring for nbcommon:")
message(STATUS "\t\tNBCOMMON DIR:" ${NBCOMMON_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes

set(TOP_HEADER_PATH "${NBCOMMON_ROOT_DIR}/include/")
set(HEADER_PATH "${NBCOMMON_ROOT_DIR}/include/"
  "${NBCOMMON_ROOT_DIR}/include/protected"
  "${NBCOMMON_ROOT_DIR}/include/private"
  "${NBCOMMON_ROOT_DIR}/include/${PLATFORM}")

file(GLOB SRC_FOLDERS "${NBCOMMON_ROOT_DIR}/src/*")

foreach(item
    "${PROJECT_SOURCE_DIR}/nbpal"
    "${PROJECT_SOURCE_DIR}/coreservices"
    "${PROJECT_SOURCE_DIR}/nbservices"
    "${PROJECT_SOURCE_DIR}/abservices"
    )
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES
  "${NBCOMMON_ROOT_DIR}/src/*.c"
  "${NBCOMMON_ROOT_DIR}/src/*.cpp"
)

# 4. Generate library.
add_library(nbcommon ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(nbcommon abservices nbprotocol)
endif ()

# 5. install...
install(
  TARGETS nbcommon
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/nbcommon"
  )