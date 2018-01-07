message(STATUS "Loading configuration for Android...")

add_definitions(-DANDROID -DSUPPORT_GLES -DSUPPORT_GLES20 -DGL_GLEXT_PROTOTYPES=1
  -DFLOATING_POINT)

# reset and rename nbm library
set(NBM_RELEASE
  "${DEPOT_ROOT}/core/releases/nbm/android/nbm_${NBM_VERSION}_android")

if (ANDROID_STL)
  if (ANDROID_STL STREQUAL "stlport_shared")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti")
    link_directories("${NBM_RELEASE}/libs/portstl")
  else()
    link_directories("${NBM_RELEASE}/libs")
  endif ()
endif ()

macro(COPY_NBM DEST)
  message(STATUS "Copying ${NBM_RELEASE}/libs/portstl/libNBM.so --> ${DEST}")
  file(COPY ${NBM_RELEASE}/libs/portstl/libNBM.so  ${DEST})
endmacro()


