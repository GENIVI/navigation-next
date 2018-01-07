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

if (${PLATFORM} STREQUAL "android_ndk")

  # 1. Preparing variables...
  set(JNI_ROOT_DIR ${PROJECT_SOURCE_DIR}/jni)

  message(STATUS "Configuring for jni:")
  message(STATUS "\t\tJNI DIR:" ${JNI_ROOT_DIR})
  message(STATUS "\t\tCCC ROOT:" ${PROJECT_SOURCE_DIR})

  # 2. Adding header search pathes
  get_parent(PROJECT_SOURCE_DIR DEPOT_ROOT 3)

  set(TOP_HEADER_PATH "${JNI_ROOT_DIR}/include")
  set(HEADER_PATH
    "${JNI_ROOT_DIR}/include"
    "${JNI_ROOT_DIR}/include/ltkcommon"
    "${JNI_ROOT_DIR}/include/network"
    "${JNI_ROOT_DIR}/include/map3d"
    "${THIRDPARTY_ROOT_DIR}/libxml2/include"
    "${THIRDPARTY_ROOT_DIR}/libxml2/include/platforms/${PLATFORM}"
    )

  foreach(item
      "${PROJECT_SOURCE_DIR}/nbpal"
      "${PROJECT_SOURCE_DIR}/coreservices"
      "${PROJECT_SOURCE_DIR}/nbservices"
      "${PROJECT_SOURCE_DIR}/abpal"
      "${PROJECT_SOURCE_DIR}/abservices"
      "${PROJECT_SOURCE_DIR}/common"
      "${PROJECT_SOURCE_DIR}/map"
      "${PROJECT_SOURCE_DIR}/nbui"
      )
    prepare_headers("${item}/include" HEADER_PATH)
  endforeach()

  prepare_headers("${PROJECT_SOURCE_DIR}/nbgm/nbgmmain/include" HEADER_PATH)

  include_directories(${HEADER_PATH})

  # 3 Add targets.

  file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")

  # ltkcommon.so
  file(GLOB_RECURSE LTKCOMMON_SOURCES
    "${JNI_ROOT_DIR}/src/ltkcommon/*.cpp"
    "${PROJECT_SOURCE_DIR}/common/src/Qalog.cpp"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/*.c"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/analyticslibxmlparser.cpp"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/analyticsengineimpl.cpp"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/analyticsxml.cpp"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/analyticsengine.cpp"
    "${PROJECT_SOURCE_DIR}/nbservices/src/analytics/analyticsdbstorage.cpp"
    "${THIRDPARTY_ROOT_DIR}/libxml2/*.c"
    )

  # network.so
  file(GLOB_RECURSE NETWORK_SOURCES
    "${JNI_ROOT_DIR}/src/network/*.cpp"
    )

  # mapkit.so
  file(GLOB_RECURSE MAP3D_SOURCES
    "${JNI_ROOT_DIR}/src/map3d/*.cpp"
    )

  add_library(ltkcommon SHARED ${LTKCOMMON_SOURCES})
  add_library(network SHARED ${NETWORK_SOURCES})
  add_library(mapkit SHARED ${MAP3D_SOURCES})

  add_dependencies(ltkcommon nbcommon)

  if (NOT STATIC_LIB)
    target_link_libraries(ltkcommon abservices)
    target_link_libraries(network nbbase)
    target_link_libraries(mapkit map)
  endif()

  install(
    FILES ${PUB_HEADERS}
    DESTINATION "${PACKAGE_DIR}/include/")

  install(
    TARGETS ltkcommon
    LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
    ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
    )

  install(
    TARGETS network
    LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
    ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
    )

  install(
    TARGETS mapkit
    LIBRARY  DESTINATION "${PACKAGE_DIR}/lib"
    ARCHIVE  DESTINATION "${PACKAGE_DIR}/lib"
    )

  # 5 unit test.
  if(UNIT_TEST)
    #todo: add codes to compile unit test.
    message(STATUS "Not implemented yet...")
  endif(UNIT_TEST)

endif()