#
# All shared libraries use the same setting
#

#
# Do not statically linked into multiple shared libraries
# that can cause memory corruption, see ndk docs CPLUSPLUS-SUPPORT.html
# under "Static runtimes" section.
#
APP_STL := gnustl_static

#
# Map3D branches cpp code requires exception.
#
#APP_CPPFLAGS += -fexceptions

#
# Map3D uses smart_prt which requires typeid.
#
APP_CPPFLAGS += -frtti

