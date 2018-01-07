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

# 1. Append other headers..
file(GLOB SRC_FOLDERS
  "${NBPAL_ROOT_DIR}/src/common/*"
  "${NBPAL_ROOT_DIR}/src/${PLATFORM}/*"
  "${NBPAL_ROOT_DIR}/src/posix/pal"
  )

set(IPHONE_HEADERS "")

foreach(item ${SRC_FOLDERS})
  if(IS_DIRECTORY ${item})
    list(APPEND IPHONE_HEADERS ${item})
  endif(IS_DIRECTORY ${item})
endforeach()

include_directories(${IPHONE_HEADERS})

# 3. Adding source files...
file(GLOB_RECURSE BASE_SOURCES
  "${NBPAL_ROOT_DIR}/src/common/*"
  "${NBPAL_ROOT_DIR}/src/iphone/clock/*"
  "${NBPAL_ROOT_DIR}/src/iphone/database/*"
  "${NBPAL_ROOT_DIR}/src/iphone/file/*"
  "${NBPAL_ROOT_DIR}/src/iphone/myplaces/*"
  "${NBPAL_ROOT_DIR}/src/iphone/net/*"
  "${NBPAL_ROOT_DIR}/src/iphone/pal/*"
  "${NBPAL_ROOT_DIR}/src/iphone/radio/*"
  "${NBPAL_ROOT_DIR}/src/iphone/synchronization/palatomic.m"
  "${NBPAL_ROOT_DIR}/src/iphone/taskqueue/*"
  "${NBPAL_ROOT_DIR}/src/iphone/timer/*"
  "${NBPAL_ROOT_DIR}/src/iphone/unzip/*"
  "${NBPAL_ROOT_DIR}/src/iphone/util/*"
  "${NBPAL_ROOT_DIR}/src/util/*"
  "${NBPAL_ROOT_DIR}/src/posix/testlog/*"
  "${NBPAL_ROOT_DIR}/src/posix/pal/*"
  "${NBPAL_ROOT_DIR}/src/posix/synchronization/*"
  "${THIRDPARTY_ROOT_DIR}/sqlite/Sources/*"
  )

file(GLOB_RECURSE GRAPHICS_SOURCES
  "${NBPAL_ROOT_DIR}/src/iphone/font/*"
  "${NBPAL_ROOT_DIR}/src/iphone/gl/*"
  "${NBPAL_ROOT_DIR}/src/iphone/triangulation/*"
  "${THIRDPARTY_ROOT_DIR}/glues/*"
  "${THIRDPARTY_ROOT_DIR}/lp150b15/*"
)

list(REMOVE_ITEM GRAPHICS_SOURCES
  "${THIRDPARTY_ROOT_DIR}/lp150b15/pngtest.c"
  )

# Third party dependencies.
get_freetype_info(FT_HEADERS FT_SOURCES)

include_directories(
  "${THIRDPARTY_ROOT_DIR}/sqlite/include"
  "${THIRDPARTY_ROOT_DIR}/glues"
  "${THIRDPARTY_ROOT_DIR}/lp150b15"
  ${FT_HEADERS})

# 4. Set compiling flags
set(FRAMEWORKS "")
foreach(item "CoreFoundation" "UIKit" "CoreGraphics" "OpenGLES" "CoreTelephony"
    "SystemConfiguration" "CFNetwork" "Security")
  set(FRAMEWORKS "${FRAMEWORKS} -framework  ${item}")
endforeach()

if (STATIC_LIB)
  set(CMAKE_STATIC_LINKER_FLAGS "${FRAMEWORKS}")
else ()
  set(CMAKE_SHARED_LINKER_FLAGS "${FRAMEWORKS}")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${FRAMEWORKS}")

# 5. Generate library.
add_library(palbase ${LIB_TYPE} ${BASE_SOURCES})
add_library(palgraphics ${LIB_TYPE} ${GRAPHICS_SOURCES} ${FT_SOURCES})

if(NOT STATIC_LIB)
    target_link_libraries(palbase z)
    target_link_libraries(palgraphics z)
endif()

if(UNIT_TEST)
  file(GLOB TEST_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/*")

  file(GLOB_RECURSE TEST_PLATFORM_SOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/${PLATFORM}/*")

  file(GLOB_RECURSE RESOURCES
    "${NBPAL_ROOT_DIR}/src/unittests/${PLATFORM}/*.xib")

  set(APP_TYPE MACOSX_BUNDLE)

  include_directories("${NBPAL_ROOT_DIR}/src/unittests/")

  # add cunit..
  add_executable(nbpal_test  ${APP_TYPE}
    ${TEST_SOURCES} ${TEST_PLATFORM_SOURCES}  ${RESOURCES})

  # We should link frameworks into app, instead of into nbpal library.
  target_link_libraries(nbpal_test palbase palgraphics cunit stdc++)
    set_target_properties(nbpal_test PROPERTIES
      MACOSX_BUNDLE_GUI_IDENTIFIER "com.tcs.nbpal_test"
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
      RESOURCE "${RESOURCES}"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
      XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER YES
      XCODE_ATTRIBUTE_GCC_PREFIX_HEADER ${CMAKE_CURRENT_LIST_DIR}/unittest_Prefix.pch
      XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
      XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 6.0
      )
endif(UNIT_TEST)

# 6. install...
install(
  TARGETS palbase
  DESTINATION "${PACKAGE_DIR}/lib"
  )

install(
  TARGETS palgraphics
  DESTINATION "${PACKAGE_DIR}/lib"
  )
