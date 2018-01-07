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
set(SEARCH_ROOT_DIR ${PROJECT_SOURCE_DIR}/search)

message(STATUS "Configuring for search:")
message(STATUS "\t\tSEARCH DIR:" ${SEARCH_ROOT_DIR})
message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

# 2. Adding header search pathes
file(GLOB COMMPONENTS "${PROJECT_SOURCE_DIR}/*")

set(TOP_HEADER_PATH "${SEARCH_ROOT_DIR}/include/")
set(HEADER_PATH
  "${SEARCH_ROOT_DIR}/include/"
  "${THIRDPARTY_ROOT_DIR}/sqlite/include")

file(GLOB INC_FOLDERS "${SEARCH_ROOT_DIR}/include/*")
file(GLOB SRC_FOLDERS "${SEARCH_ROOT_DIR}/src/*")

foreach(item ${INC_FOLDERS} ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

set(TOP "${SEARCH_ROOT_DIR}/src/datamanager")
set(DIRS ${TOP}
  "${TOP}/db"
  "${TOP}/dao/interface"
  "${TOP}/dao/dbdao")

foreach(item ${DIRS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif()
endforeach()

foreach(item ${COMMPONENTS})
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES "${SEARCH_ROOT_DIR}/src/*.cpp")

# 4. Generate library.
add_library(search ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(search nbcommon nbanalytics)
endif ()

#@todo: move this into common part ?
if (${PLATFORM} STREQUAL "win32")
    set_target_properties(search PROPERTIES COMPILE_FLAGS "/wd4996")
endif ()

# 5. install...
install(
  TARGETS search
  LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
  ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
  )

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/search"
  )
