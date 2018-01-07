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
set(NAV_ROOT_DIR ${PROJECT_SOURCE_DIR}/nav)

message(STATUS "Configuring for nav:")
message(STATUS "\t\tNAV DIR:" ${NAV_ROOT_DIR})

# 2. Adding header search pathes
file(GLOB COMMPONENTS "${PROJECT_SOURCE_DIR}/*")

set(TOP_HEADER_PATH "${NAV_ROOT_DIR}/include/")
set(HEADER_PATH "${NAV_ROOT_DIR}/include/")

file(GLOB INC_FOLDERS "${NAV_ROOT_DIR}/include/*")
file(GLOB SRC_FOLDERS "${NAV_ROOT_DIR}/src/*")

# todo: create function to add subfolders...
foreach(item ${INC_FOLDERS} ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND HEADER_PATH ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

foreach(item ${COMMPONENTS})
  prepare_headers("${item}/include/" HEADER_PATH)
endforeach()

include_directories(${HEADER_PATH})

# 3. Adding source files...
file(GLOB_RECURSE SOURCES "${NAV_ROOT_DIR}/src/*.cpp")

# 4. Generate library.
add_library(nav ${LIB_TYPE} ${SOURCES})

if (NOT STATIC_LIB)
  target_link_libraries(nav nbnav nbcommon)
  set_target_properties(nav PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nav PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nav PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
endif ()

#@todo: move this into common part ?
if (${PLATFORM} STREQUAL "win32")
    set_target_properties(nav PROPERTIES COMPILE_FLAGS "/wd4996")
endif ()

# 5. install...

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/nav"
  )