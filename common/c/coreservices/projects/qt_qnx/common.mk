# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=coreservices

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

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/protected  \
	$(PROJECT_ROOT)/../../include/private  \
	$(PROJECT_ROOT)/../../include/qt_qnx $(NIMNBIPAL_INC)  \
	$(NIMNBIPAL_INC)/private $(NIMNBIPAL_INC)/qt_qnx

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src/logging  \
	$(PROJECT_ROOT)/../../src/network  \
	$(PROJECT_ROOT)/../../src/qalog  \
	$(PROJECT_ROOT)/../../src/tps  \
	$(PROJECT_ROOT)/../../src/util

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

