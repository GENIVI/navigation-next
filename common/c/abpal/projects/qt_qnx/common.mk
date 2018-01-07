# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=abpal

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-w9 -fPIC -fstack-protector-strong -v -g -DENABLE_LOGGING_TO_OUTPUT

ifdef INSTRUMENT	
CCFLAGS+= -g -O0 -finstrument-functions
endif

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-E -M, -Wl,-z relro -Wl,--as-needed -Wl,-z,now

ifdef INSTRUMENT
LDFLAGS+= -lprofilingS
endif


#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+= \
	$(PROJECT_ROOT)/../../src/qt_qnx/gps  \
	$(PROJECT_ROOT)/../../src/qt_qnx/audio  \
	$(PROJECT_ROOT)/../../src/util/combineraac  \
	$(PROJECT_ROOT)/../../src/util

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/private  \
	$(PROJECT_ROOT)/../../include/qt_qnx  \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)/private  \
	$(NIMNBIPAL_INC)/nbpal  \
	$(NIMNBIPAL_INC)  \
	$(QNX_TARGET)/usr/include

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

