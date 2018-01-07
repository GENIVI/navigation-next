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
set(NAVUI_ROOT_DIR ${PROJECT_SOURCE_DIR}/navui)

message(STATUS "Configuring for navui:")
message(STATUS "NAVUI DIR:" ${NAVUI_ROOT_DIR})

# 2. Adding header search pathes
file(GLOB COMMPONENTS "${PROJECT_SOURCE_DIR}/*")

set(TOP_HEADER_PATH "${NAVUI_ROOT_DIR}/include/")
set(HEADER_PATH "${NAVUI_ROOT_DIR}/include/"
  "${THIRDPARTY_ROOT_DIR}/tinyxml")

file(GLOB INC_FOLDERS "${NAVUI_ROOT_DIR}/include/*")
file(GLOB SRC_FOLDERS "${NAVUI_ROOT_DIR}/src/*")

# todo: create function to add subfolders...
foreach(item ${INC_FOLDERS} ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

foreach(item ${COMMPONENTS})
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

list(APPEND HEADER_PATH
  "${PROJECT_SOURCE_DIR}/thirdparty/${BRANCH}/tinyxml")

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES "${NAVUI_ROOT_DIR}/src/*.cpp"
  "${THIRDPARTY_ROOT_DIR}/tinyxml/*.c"
  "${THIRDPARTY_ROOT_DIR}/tinyxml/*.cpp")

# 4. Generate library.
add_library(navui ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(navui nav)
endif ()

# 5. install...
install(
  TARGETS navui
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/navui"
  )

install(
  DIRECTORY "${TOP_HEADER_PATH}/widgets"
  DESTINATION "${PACKAGE_DIR}/include/navui"
  )