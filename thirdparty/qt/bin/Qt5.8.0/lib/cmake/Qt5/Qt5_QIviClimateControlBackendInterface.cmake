
add_library(Qt5::QIviClimateControlBackendInterface MODULE IMPORTED)

_populate__plugin_properties(QIviClimateControlBackendInterface RELEASE "qtivi/libvehiclefunction_simulator.so")

list(APPEND Qt5_PLUGINS Qt5::QIviClimateControlBackendInterface)
