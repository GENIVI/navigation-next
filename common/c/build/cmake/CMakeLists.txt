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

project (CoreSDK)
cmake_minimum_required(VERSION 2.8.12)

# Options..
option (PLATFORM "Platform ..."  OFF)
option (STATIC_LIB "Build for static library ..."  OFF)
option (UNIT_TEST "Compile unit test or not ..."  OFF)
option (SYSTEM_TEST "Compile system test or not ..."  OFF)

option (DEBUG_CMAKE "Show or hide message for debug cmake scripts..."  OFF)

if(NOT PLATFORM) # update platform if it is not specified.
  string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM)
  #todo: We may need to add a target/platform mapping here.
endif()

if(STATIC_LIB)
  set(LIB_TYPE STATIC)
else()
  set(LIB_TYPE SHARED)
endif()

message(STATUS "PLATFORM: " ${PLATFORM})

# Functions used while building CCC.

# get_parent -- get n'th parent directory.
function(get_parent input output ...)
  set(${output} ${${input}}  PARENT_SCOPE)
  set(tmp ${${input}})
  set(nth 1)
  if(${ARGC} GREATER 2)
    list(GET ARGV 2 nth)
  endif()
  # message(STATUS "nth: " ${nth} ", input" ${${input}})

  while(${nth} GREATER 0)
    get_filename_component(tmp ${tmp} DIRECTORY)
    math(EXPR nth "${nth} - 1")
  endwhile()

  set(${output} ${tmp} PARENT_SCOPE)
endfunction()

# check header paths...
function (prepare_headers INPUT OUTPUT)
  if(DEFINED ${INPUT})
    set(TOP ${${INPUT}})
  else()
    set(TOP ${INPUT})
  endif()

  set(DIRS ${TOP}
    "${TOP}/protected"
    "${TOP}/generated"
    "${TOP}/private"
    "${TOP}/${PLATFORM}")

  set(OUT ${${OUTPUT}})

  foreach(item ${DIRS})
    if(IS_DIRECTORY ${item})
      list(APPEND OUT ${item})
    endif()
  endforeach()
  set(${OUTPUT} ${OUT} PARENT_SCOPE)
endfunction()

# function to debug headers...
function (show_list MSG HEADERS)
  if (DEBUG_CMAKE)
    foreach(item ${${HEADERS}})
      message(STATUS ${MSG} ":" ${item})
    endforeach()
  endif (DEBUG_CMAKE)
endfunction()

# Preparing variables...
get_parent(CMAKE_SOURCE_DIR CCC_BUILD_DIR 1)
get_parent(CCC_BUILD_DIR CCC_ROOT_DIR 2)
get_parent(PROJECT_SOURCE_DIR DEPOT_ROOT 6)
get_filename_component(BRANCH ${CCC_BUILD_DIR} NAME)

set(CCC_CMAKE_TOP ${PROJECT_SOURCE_DIR})

file(STRINGS "${CCC_BUILD_DIR}/version_${PLATFORM}.txt" BUILD_NUMBER)
file(STRINGS "${CCC_BUILD_DIR}/nbm_${PLATFORM}_version.txt" NBM_VERSION)
file(STRINGS "${CCC_BUILD_DIR}/ncdb_${PLATFORM}_version.txt" NCDB_VERSION)
if (ARCHITECTURE)
  set(PACKAGE_DIR "${CCC_BUILD_DIR}/CoreSDK_${BUILD_NUMBER}_${PLATFORM}_${ARCHITECTURE}_internal")
else ()
  set(PACKAGE_DIR "${CCC_BUILD_DIR}/CoreSDK_${BUILD_NUMBER}_${PLATFORM}_internal")
endif ()
set(THIRDPARTY_ROOT_DIR "${PROJECT_SOURCE_DIR}/thirdparty")
set(NBM_RELEASE "${DEPOT_ROOT}/core/releases/nbm/${PLATFORM}/nbm_${NBM_VERSION}_${PLATFORM}")
set(NCDB_RELEASE "${DEPOT_ROOT}/core/releases/ncdb/${PLATFORM}/ncdb_${NCDB_VERSION}_${PLATFORM}")

message(STATUS "TOP DIR:" ${CCC_BUILD_DIR})
message(STATUS "CCC ROOT:" ${PROJECT_SOURCE_DIR})
message(STATUS "BRANCH:" ${BRANCH})
message(STATUS "CCC_VERSION: " ${BUILD_NUMBER})
message(STATUS "NBM_VERSION: " ${NBM_VERSION})
message(STATUS "NCDB_VERSION: " ${NCDB_VERSION})
message(STATUS "PACK_DIR: " ${PACKAGE_DIR})
message(STATUS "THIRD_PARTY: " ${THIRDPARTY_ROOT_DIR})
message(STATUS "NBM_RELEASE: " ${NBM_RELEASE})
message(STATUS "NCDB_RELEASE: " ${NCDB_RELEASE})

# Load platform specified generic settings.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM}.cmake")
  message(STATUS "${PLATFORM}.cmake")
  include ("${PLATFORM}.cmake")
endif ()


if (CMAKE_BUILD_TYPE STREQUAL "Release" OR
    CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  add_definitions("-DNDEBUG")
endif ()

if(UNIT_TEST OR SYSTEM_TEST)
  #@todo: build cu here.
  set(CU_HEADER_PATH "${THIRDPARTY_ROOT_DIR}/CUnit/include")
  file(GLOB CU_FRAMEWORK "${THIRDPARTY_ROOT_DIR}/CUnit/Sources/Framework/*")
  file(GLOB CU_BASIC "${THIRDPARTY_ROOT_DIR}/CUnit/Sources/Basic/*")
  file(GLOB CU_AUTOMATED "${THIRDPARTY_ROOT_DIR}/CUnit/Sources/Automated/*")
  include_directories(${CU_HEADER_PATH})
  add_library(cunit ${LIB_TYPE}
    ${CU_FRAMEWORK} ${CU_BASIC} ${CU_AUTOMATED})
endif()

# Search all components that can be compiled using cmake.
file(GLOB COMPONENTS "${PROJECT_SOURCE_DIR}/*")
foreach(item ${COMPONENTS})
  set(path "${item}/projects/cmake")
  if(IS_DIRECTORY ${path})
    message(STATUS "Adding ${path} to sub directory...")
    get_filename_component(CMP ${item} NAME)
    add_subdirectory(${path} ${CMAKE_CURRENT_BINARY_DIR}/${CMP})
  else()
    message(STATUS "${path} not valid for CMake...")
  endif()
endforeach()
