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
set(NBSERVICES_ROOT_DIR ${PROJECT_SOURCE_DIR}/nbservices)

message(STATUS "Configuring for nbservices:")
message(STATUS "\t\tNBSERVICES DIR:" ${NBSERVICES_ROOT_DIR})

# 2. Adding header search pathes

set(TOP_HEADER_PATH "${NBSERVICES_ROOT_DIR}/include")
set(HEADER_PATH "${NBSERVICES_ROOT_DIR}/include"
  "${NBSERVICES_ROOT_DIR}/include/protected"
  "${NBSERVICES_ROOT_DIR}/include/private"
  "${NBSERVICES_ROOT_DIR}/include/${PLATFORM}"
  "${NBSERVICES_ROOT_DIR}/src/map/tilemanager"
  "${NBSERVICES_ROOT_DIR}/src/map/layermanager"
  "${NBSERVICES_ROOT_DIR}/src/map/mapview"
  "${NBSERVICES_ROOT_DIR}/src/analytics"
  "${THIRDPARTY_ROOT_DIR}/libxml2/include"
  "${THIRDPARTY_ROOT_DIR}/libxml2/include/platforms/${PLATFORM}"
  "${THIRDPARTY_ROOT_DIR}/tinyxml"
  "${THIRDPARTY_ROOT_DIR}/lp150b15"
  "${THIRDPARTY_ROOT_DIR}/sqlite/include"
  "${THIRDPARTY_ROOT_DIR}/zlib"
  "${NBM_RELEASE}/include"
  "${NCDB_RELEASE}/include"
  )
set(NBS_SRC "${NBSERVICES_ROOT_DIR}/src")

if (${PLATFORM} STREQUAL "iphone")
  list(APPEND HEADER_PATH "${PROJECT_SOURCE_DIR}/nbui/src/${PLATFORM}/common")
endif(${PLATFORM} STREQUAL "iphone")


prepare_headers("${PROJECT_SOURCE_DIR}/nbgm/nbgmmain/include" HEADER_PATH)

file(GLOB COMPONENTS "${PROJECT_SOURCE_DIR}/*")
foreach(item ${COMPONENTS})
  prepare_headers("${item}/include" HEADER_PATH)
endforeach()

if (${PLATFORM} STREQUAL "win32")
  list(APPEND HEADER_PATH "${THIRDPARTY_ROOT_DIR}/glew-1.7.0/include")
endif ()

include_directories(${HEADER_PATH})

# 3. Adding source files...
function (get_source_files INPUT_DIR OUTPUT_VAR)
  set(OUT ${${OUTPUT_VAR}})
  file(GLOB_RECURSE TMP "${INPUT_DIR}/*.c" "${INPUT_DIR}/*.cpp")
  list(APPEND OUT ${TMP})
  set(${OUTPUT_VAR} ${OUT} PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE NB_PROTOCOL_SOURCES "${NBS_SRC}/protocol/*.cpp")

file(GLOB_RECURSE NB_BASE_SOURCES
  "${NBS_SRC}/nbcommon/*.cpp"
  "${NBS_SRC}/nbcommon/nbpersistentdata.c"
  "${NBS_SRC}/data/datautil.c"
  "${NBS_SRC}/data/data_string.c"
  "${NBS_SRC}/data/data_blob.c"
  "${NBS_SRC}/data/data_search_cookie.c"
  "${NBS_SRC}/util/gpsutil.c"
  )

foreach(item network qalog spatial)
  get_source_files("${NBS_SRC}/${item}" NB_BASE_SOURCES)
endforeach()

#move from protocol to base
foreach(item
  TpsElement
  NBProtocolBinaryData
  NBProtocolIdenParameters
  NBProtocolCarrierInfo
  NBProtocolDeviceInfo
  NBProtocolElementToXML
  NBProtocolFormattedIdenParameters
  NBProtocolLocaleInfo
  NBProtocolPlatformInfo
  NBProtocolProductInfo
  NBProtocolPushMessageInfo
  NBProtocolSessionInfo
  NBProtocolUserInfo
  )
  file(GLOB TMP_SOURCE "${NBS_SRC}/protocol/${item}*cpp")
  list(APPEND NB_BASE_SOURCES ${TMP_SOURCE})
  list(REMOVE_ITEM NB_PROTOCOL_SOURCES ${TMP_SOURCE})
endforeach()

#remove from base
foreach(item
  NBProtocolGpsProbesEvent
  )
  file(GLOB TMP_SOURCE "${NBS_SRC}/protocol/${item}*cpp")
  list(REMOVE_ITEM NB_BASE_SOURCES ${TMP_SOURCE})
endforeach()

get_source_files("${THIRDPARTY_ROOT_DIR}/libxml2" NB_ANALYTICS_SOURCES)
get_source_files("${NBS_SRC}/analytics/" NB_ANALYTICS_SOURCES)

#move from protocol to analytics
foreach(item
  NBProtocolAnalyticsEvent
  NBProtocolSearchQueryEvent
  NBProtocolSearchDetailEvent
  NBProtocolImpressionEvent
  NBProtocolMapEvent
  NBProtocolPlaceMessageEvent
  NBProtocolFacebookUpdateEvent
  NBProtocolCallEvent
  NBProtocolAddFavoritesEvent
  NBProtocolRouteRequestEvent
  NBProtocolRouteReplyEvent
  NBProtocolRouteStateEvent
  NBProtocolGpsProbesEvent
  NBProtocolFeedbackEvent
  NBProtocolAppErrorEvent
  NBProtocolWifiProbesEvent
  NBProtocolWeburlEvent
  NBProtocolArrivalEvent
  NBProtocolTransactionEvent
  NBProtocolAnalyticsEventMetadata
  NBProtocolAnnouncementEvent
  NBProtocolUserActionEvent
  NBProtocolSessionStartEvent
  NBProtocolSettingsChangeEvent
  NBProtocolShareEvent
  NBProtocolGeneralSettings
  NBProtocolNavigationSettings
  NBProtocolRouteSettings
  NBProtocolMapSettings
  NBProtocolTrafficAnanouncementInfo
  NBProtocolSearchEventCookie
  NBProtocolContentDisplayed
  NBProtocolIssueType
  NBProtocolErrorCategory
  NBProtocolTransaction
  TpsTransactionSourceHandler
  TpsAtlasbookProtocol
  NBProtocolReplyHeaders
  NBProtocolCouponOffer
  NBProtocolVendorContent
  NBProtocolPoiContent
  NBProtocolProxMatch
  NBProtocolSingleSearch
  TpsSingleSearchSourceHandler
  )
  file(GLOB TMP_SOURCE "${NBS_SRC}/protocol/${item}*cpp")
  list(APPEND NB_ANALYTICS_SOURCES ${TMP_SOURCE})
  list(REMOVE_ITEM NB_PROTOCOL_SOURCES ${TMP_SOURCE})
endforeach()

file(GLOB_RECURSE NB_DATA_SOURCES
  "${NBS_SRC}/data/*.c"
  "${NBS_SRC}/nbcommon/nbutility.c"  )

foreach(item gps location locationservices wifiprobes)
  get_source_files("${NBS_SRC}/${item}" NB_LOCATION_SOURCES)
endforeach()

file(GLOB_RECURSE NB_EC_SOURCES
  "${NBS_SRC}/enhancedcontent/*.c"
  "${NBS_SRC}/enhancedcontent/*.cpp")

file(GLOB_RECURSE NB_MAP_SOURCES
  "${NBS_SRC}/map/*.c"
  "${NBS_SRC}/map/*.cpp"
  "${THIRDPARTY_ROOT_DIR}/tinyxml/*.c"
  "${THIRDPARTY_ROOT_DIR}/tinyxml/*.cpp"
  "${PROJECT_SOURCE_DIR}/nbui/src/${PLATFORM}/*.c"
  "${PROJECT_SOURCE_DIR}/nbui/src/${PLATFORM}/*.cpp"
  "${PROJECT_SOURCE_DIR}/nbui/src/${PLATFORM}/*.m"
  "${PROJECT_SOURCE_DIR}/nbui/src/${PLATFORM}/*.mm"
  "${NBS_SRC}/nav/enhanced_content/PrefetchProcessor.cpp"
  "${NBS_SRC}/nbcommon/nbsolarcalculator.c"
  "${NBS_SRC}/nbcommon/nbrouteid.c" #@todo: this should be in nbbase.
  "${NBS_SRC}/util/*.cpp"
  )

file(GLOB_RECURSE NB_GEOCODE_SOURCES
  "${NBS_SRC}/geocode/*.c"
  "${NBS_SRC}/geocode/*.cpp")

file(GLOB NB_NAV_SOURCES
  "${NBS_SRC}/nav/*.c"
  "${NBS_SRC}/nav/*.cpp"
  "${NBS_SRC}/nbcommon/nbrouteid.c"
  "${NBS_SRC}/nbcommon/nbformat.c"
  "${NBS_SRC}/nbcommon/nbpointiteration.c"
  "${NBS_SRC}/util/format.c"
  )

file(GLOB_RECURSE NB_SEAECH_SOURCES
  "${NBS_SRC}/proxpoi/*.c"
  "${NBS_SRC}/singlesearch/*.c"
  "${NBS_SRC}/singlesearch/*.cpp")

file(GLOB_RECURSE NB_RASTERMAP_SOURCES
  "${NBS_SRC}/rastermap/*.c")

# 4. Generate library.
add_definitions("-DNB_BUILD")
add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")

#link_directories("${NBM_RELEASE}/libs")
link_directories("${NCDB_RELEASE}/lib")

add_library(nbanalytics ${LIB_TYPE} ${NB_ANALYTICS_SOURCES})
add_library(nbbase ${LIB_TYPE} ${NB_BASE_SOURCES})
add_library(nbdata ${LIB_TYPE} ${NB_DATA_SOURCES})
add_library(nbec ${LIB_TYPE} ${NB_EC_SOURCES})
add_library(nblocation ${LIB_TYPE} ${NB_LOCATION_SOURCES})
add_library(nbprotocol ${LIB_TYPE} ${NB_PROTOCOL_SOURCES})
add_library(nbnav ${LIB_TYPE} ${NB_NAV_SOURCES} ${NB_SEAECH_SOURCES})
add_library(nbmap ${LIB_TYPE} ${NB_MAP_SOURCES}  ${NB_SEAECH_SOURCES} ${NB_RASTERMAP_SOURCES})
add_library(nbgeocode ${LIB_TYPE} ${NB_GEOCODE_SOURCES})

file(MAKE_DIRECTORY ${PACKAGE_DIR}/lib/)
if (NOT STATIC_LIB)
  add_custom_target(copyNCDB
    COMMAND ${CMAKE_COMMAND} -E
    copy ${NCDB_RELEASE}/lib/libncdb.a ${PACKAGE_DIR}/lib/
    )
  set(CMAKE_SHARED_LINKER_FLAGS "-lncdb -framework UIKit -framework OpenGLES -framework QuartzCore -framework CoreGraphics")
  target_link_libraries(nbanalytics nbprotocol)
  target_link_libraries(nbbase palbase palgraphics coreservices)
  target_link_libraries(nbdata nbbase)
  target_link_libraries(nbec nbanalytics)
  target_link_libraries(nblocation nbanalytics abpal)
  target_link_libraries(nbprotocol nbdata)
  target_link_libraries(nbnav nbec nblocation)
  target_link_libraries(nbmap nbgm ncdb nbanalytics)
  target_link_libraries(nbgeocode nbdata)
  set_target_properties(nbanalytics PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbbase PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbdata PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbec PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nblocation PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbprotocol PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbnav PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbmap PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
  set_target_properties(nbgeocode PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")

  set_target_properties(nbanalytics PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbbase PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbdata PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbec PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nblocation PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbprotocol PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbnav PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbmap PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
  set_target_properties(nbgeocode PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

  set_target_properties(nbanalytics PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbbase PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbdata PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbec PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nblocation PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbprotocol PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbnav PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbmap PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")
  set_target_properties(nbgeocode PROPERTIES XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/$(EXECUTABLE_PATH)")

  add_dependencies(nbmap copyNCDB)
else ()
  add_custom_target(copyNCDB
    COMMAND ${CMAKE_COMMAND} -E
    copy ${NCDB_RELEASE}/lib/libncdb.a ${PACKAGE_DIR}/lib/
  )

  target_link_libraries(nbmap ncdb)

  add_dependencies(nbmap copyNCDB)
endif ()

if (${PLATFORM} STREQUAL "win32")
  set_target_properties(nbanalytics PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbbase PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbdata PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbec PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nblocation PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbprotocol PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbnav PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbmap PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
  set_target_properties(nbgeocode PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4706 /wd4996")
endif ()

# 5. install...

file(GLOB PUB_HEADERS "${TOP_HEADER_PATH}/*.h")
install(
  FILES ${PUB_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

install(
  DIRECTORY "${TOP_HEADER_PATH}/${PLATFORM}"
  DESTINATION "${PACKAGE_DIR}/include/"
  )

file(GLOB NBUI_HEADERS "${PROJECT_SOURCE_DIR}/nbui/include/*.h")
install(
  FILES ${NBUI_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/"
  )

file(GLOB NBUI_PLATFORM_HEADERS "${PROJECT_SOURCE_DIR}/nbui/include/${PLATFORM}/*.h")
install(
  FILES ${NBUI_PLATFORM_HEADERS}
  DESTINATION "${PACKAGE_DIR}/include/${PLATFORM}"
  )

if(UNIT_TEST)
  #todo: add codes to compile unit test.
  message(STATUS "Not implemented yet...")
endif(UNIT_TEST)
