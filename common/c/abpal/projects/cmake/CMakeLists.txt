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
set(ABPAL_ROOT_DIR ${PROJECT_SOURCE_DIR}/abpal)

message(STATUS "Configuring for abpal:")
message(STATUS "\t\tABPAL DIR:" ${ABPAL_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes

file(GLOB PUB_HEADERS "${ABPAL_ROOT_DIR}/include/*.h")
file(GLOB PLATFORM_HEADER "${ABPAL_ROOT_DIR}/include/${PLATFORM}/*.h")
set(HEADER_PATH "${ABPAL_ROOT_DIR}/include/"
  "${ABPAL_ROOT_DIR}/include/private"
  "${ABPAL_ROOT_DIR}/include/${PLATFORM}")

file(GLOB SRC_FOLDERS "${ABPAL_ROOT_DIR}/src/*")

foreach(item
    "${PROJECT_SOURCE_DIR}/nbpal"
    "${PROJECT_SOURCE_DIR}/coreservices"
    )
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES
  "${ABPAL_ROOT_DIR}/src/util/*"
  "${ABPAL_ROOT_DIR}/src/${PLATFORM}/*"
  )

# 4. Generate library.
add_library(abpal ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(abpal palbase)
endif ()

# 5. install...
install(
  TARGETS abpal
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include"
  )

install(
  FILES ${PLATFORM_HEADER}
  DESTINATION "${PACKAGE_DIR}/include/${PLATFORM}"
  )
