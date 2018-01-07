# This is an automatically generated makefile.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qmacros.mk

# QNX Internal Start
EXTRA_SRCVPATH += \
    $(PROJECT_ROOT)/../../../libjpeg-turbo/src
EXTRA_INCVPATH += \
    $(QNX_TARGET)/usr/include \
	$(PROJECT_ROOT)/../../../libjpeg-turbo/include \
	$(PROJECT_ROOT)/../../../libjpeg-turbo/include/qt_qnx \

LDFLAGS+=-lang-c++ -Wl,-z relro -Wl,--as-needed -Wl,-z,now
CCFLAGS+=-fstack-protector-strong -v -g

EXTRA_SUFFIXES+=cxx cpp
EXTRA_CLEAN+=*.ii

ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

# QNX Internal End

include $(MKFILES_ROOT)/qtargets.mk
OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
