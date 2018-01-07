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
get_parent(CMAKE_CURRENT_BINARY_DIR NBUI_ROOT_DIR 2)

message(STATUS "Configuring for nbui:")
message(STATUS "\t\tNBUI DIR:" ${NBUI_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes
get_parent(PROJECT_SOURCE_DIR DEPOT_ROOT 3)
set(NBM_RELEASE "${DEPOT_ROOT}/core/releases/nbm/${PLATFORM}/nbm_${NBM_VERSION}_${PLATFORM}")

#@todo: libxml and libpng may be shipped with OS...

set(TOP_HEADER_PATH "${NBUI_ROOT_DIR}/include")
set(HEADER_PATH "${NBUI_ROOT_DIR}/include"
  "${NBUI_ROOT_DIR}/include/protected"
  "${NBUI_ROOT_DIR}/include/private"
  "${NBUI_ROOT_DIR}/include/${PLATFORM}"
  )

prepare_headers("${PROJECT_SOURCE_DIR}/nbgm/${BRANCH}/nbgmmain/include" HEADER_PATH)

file(GLOB COMPONENTS "${PROJECT_SOURCE_DIR}/*")
foreach(item ${COMPONENTS})
  prepare_headers("${item}/include" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES
  "${PROJECT_SOURCE_DIR}/nbui/${BRANCH}/src/${PLATFORM}/*.c"
  "${PROJECT_SOURCE_DIR}/nbui/${BRANCH}/src/${PLATFORM}/*.cpp"
  )

# 4. Generate library.
#@TRICKY@: make it a static library to avoid link errors...
add_library(nbui STATIC ${SOURCES})
target_link_libraries(nbui GLESv2 EGL android)

# 5. install...
install(
  TARGETS nbui
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

install(
  DIRECTORY "${TOP_HEADER_PATH}/${PLATFORM}"
  DESTINATION "${PACKAGE_DIR}/include/"
  )

if(UNIT_TEST)
  #todo: add codes to compile unit test.
  message(STATUS "Not implemented yet...")
endif(UNIT_TEST)