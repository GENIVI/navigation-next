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
set(ABSERVICES_ROOT_DIR ${PROJECT_SOURCE_DIR}/abservices)

message(STATUS "Configuring for abservices:")
message(STATUS "\t\tABSERVICES DIR:" ${ABSERVICES_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes

file(GLOB PUB_HEADERS "${ABSERVICES_ROOT_DIR}/include/*.h")
file(GLOB PLATFORM_HEADER "${ABSERVICES_ROOT_DIR}/include/${PLATFORM}/*.h")

set(HEADER_PATH "${ABSERVICES_ROOT_DIR}/include/"
  "${ABSERVICES_ROOT_DIR}/include/protected"
  "${ABSERVICES_ROOT_DIR}/include/private"
  "${ABSERVICES_ROOT_DIR}/include/${PLATFORM}"
  "${THIRDPARTY_ROOT_DIR}/speex/include"
  "${THIRDPARTY_ROOT_DIR}/speex/${PLATFORM}"
)

foreach(item
    "${PROJECT_SOURCE_DIR}/nbpal"
    "${PROJECT_SOURCE_DIR}/abpal"
    "${PROJECT_SOURCE_DIR}/coreservices"
    "${PROJECT_SOURCE_DIR}/nbservices"
    )
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES
  "${ABSERVICES_ROOT_DIR}/src/*.*"
)

# 4. Generate library.
add_library(abservices ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(abservices nbdata)
endif ()

# 5. install...
install(
  TARGETS abservices
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
