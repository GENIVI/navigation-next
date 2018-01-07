# This is an automatically generated makefile.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qmacros.mk

# QNX Internal Start
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../../speex/private \
	$(PROJECT_ROOT)/../../../speex/libogg-1.3.0/src \
	$(PROJECT_ROOT)/../../../speex/speex-1.2rc1/libspeex \
LDFLAGS+=-lang-c++ -Wl,-z relro -Wl,--as-needed -Wl,-z,now
CCFLAGS+=-fstack-protector-strong -v -g
EXTRA_INCVPATH+=C:/bbndk-2.0.0-beta2/target/target-override/usr/include \
	$(QNX_TARGET)/usr/include/freetype2 \
	$(PROJECT_ROOT)/../../../speex/public \
	$(PROJECT_ROOT)/../../../speex/qt_qnx \
	$(PROJECT_ROOT)/../../../speex/speex-1.2rc1/libspeex \
	$(PROJECT_ROOT)/../../../speex/speex-1.2rc1/include \
	$(PROJECT_ROOT)/../../../speex/speex-1.2rc1/include/speex \
	$(PROJECT_ROOT)/../../../speex/libogg-1.3.0/include \
	$(PROJECT_ROOT)/../../../speex/libogg-1.3.0/include/ogg \
	$(NIMNBIPAL_INC)/qt_qnx \
	$(NIMNBIPAL_INC)
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
