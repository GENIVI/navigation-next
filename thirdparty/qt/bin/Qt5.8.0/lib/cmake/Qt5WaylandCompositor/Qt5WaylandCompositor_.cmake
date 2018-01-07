
add_library(Qt5:: MODULE IMPORTED)

_populate_WaylandCompositor_plugin_properties( RELEASE "wayland-graphics-integration-server/libdrm-egl-server.so")

list(APPEND Qt5WaylandCompositor_PLUGINS Qt5::)
