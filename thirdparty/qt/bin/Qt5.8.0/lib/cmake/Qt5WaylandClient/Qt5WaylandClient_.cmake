
add_library(Qt5:: MODULE IMPORTED)

_populate_WaylandClient_plugin_properties( RELEASE "wayland-shell-integration/libivi-shell.so")

list(APPEND Qt5WaylandClient_PLUGINS Qt5::)
