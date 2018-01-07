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

message(WARNING "library output directory needs to updated!")

add_definitions(-DSUPPORT_GLES -DSUPPORT_GLES20 -DGL_GLEXT_PROTOTYPES=1 -DFLOATING_POINT)
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++98")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion")

# Only one of the below can be set to true at same time.
#option(BUILD_PLATFORM_IPHONE "Enable IPhoneSDK Device support" ON)
#option(BUILD_PLATFORM_IPHONE_SIMULATOR "Enable IPhoneSDK Simulator support" OFF)
message(STATUS "BUILD PLATFORM_IPHONE=${BUILD_PLATFORM_IPHONE}")
message(STATUS "BUILD PLATFORM_IPHONE_SIMULATOR=${BUILD_PLATFORM_IPHONE_SIMULATOR}")

if ((NOT BUILD_PLATFORM_IPHONE) AND (NOT BUILD_PLATFORM_IPHONE_SIMULATOR))
  message(FATAL_ERROR "Configure: Supports only simulator or iPhone devices ...")
endif ()

if (BUILD_PLATFORM_IPHONE AND BUILD_PLATFORM_IPHONE_SIMULATOR)
  message(FATAL_ERROR "Configure: BUILD_PLATFORM_IPHONE and "
    "BUILD_PLATFORM_IPHONE_SIMULATOR can not be set at same time...")
endif ()

set (IPHONE_SDKVER "8.1" CACHE STRING "IOS SDK-Version")
set (IPHONE_VERSION_MIN "7.0" CACHE STRING "IOS minimum os version")
set (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.0" CACHE STRING "Set the minimum deployment target value." FORCE)

if(BUILD_PLATFORM_IPHONE)
  execute_process(
    COMMAND xcrun --sdk iphoneos --show-sdk-path
    OUTPUT_VARIABLE SDK_PATH)
  get_parent(SDK_PATH IPHONE_SDKROOT 1)

  set (IPHONE_SDKROOT "${IPHONE_SDKROOT}/iPhoneOS${IPHONE_SDKVER}.sdk")

  if(${IPHONE_VERSION_MIN} LESS "7.0")
    set(CMAKE_OSX_ARCHITECTURES "armv6;armv7" CACHE STRING "Build architectures for iOS" FORCE)
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -miphoneos-version-min=${IPHONE_VERSION_MIN} -mno-thumb -pipe -no-cpp-precomp"
      CACHE STRING "Flags used by the compiler during all build types." FORCE)
  else()
    set(CMAKE_OSX_ARCHITECTURES
      "$(ARCHS_STANDARD)"
      CACHE STRING "Build architectures for iOS" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=${IPHONE_VERSION_MIN} -pipe -no-cpp-precomp"
      CACHE STRING "Flags used by the compiler during all build types." FORCE)
  endif()
else()
  execute_process(
    COMMAND xcrun --sdk iphonesimulator --show-sdk-path
    OUTPUT_VARIABLE SDK_PATH
    )

  get_parent(SDK_PATH IPHONE_SDKROOT 1)

  set (IPHONE_SDKROOT "${IPHONE_SDKROOT}/iPhoneSimulator${IPHONE_SDKVER}.sdk")

  #simulator uses i386 architectures
  set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" CACHE STRING "Build architectures for iOS Simulator" FORCE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-thumb -pipe -no-cpp-precomp"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

if (IS_DIRECTORY ${IPHONE_SDKROOT})
  message(STATUS "OK: ${IPHONE_SDKROOT}")
else ()
  message(FATAL_ERROR "SDK: ${IPHONE_SDKROOT} does not exist!")
endif ()

set(CMAKE_OSX_SYSROOT "${IPHONE_SDKROOT}" CACHE STRING "System root for iOS" FORCE)

#use the IPhone windowing system
set(WINDOWING_SYSTEM "IOS"
  CACHE STRING "Forced IPhone windowing system on iOS"  FORCE)
set(DEFAULT_IMAGE_PLUGIN_FOR_OSX "imageio"
  CACHE STRING "Forced imageio default image plugin for iOS" FORCE)

# Apple iOS: Find OpenGLES
find_library(OPENGLES_LIBRARY OpenGLES)
