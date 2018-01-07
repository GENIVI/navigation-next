# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=abservices

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/private  \
	$(PROJECT_ROOT)/../../include/qt_qnx $(NIMNB_INC)  \
	$(NIMNB_INC)/protected $(NIMNB_INC)/private  \
	$(NIMNB_INC)/qt_qnx $(NIMCORE_INC)  \
	$(NIMCORE_INC)/protected $(NIMCORE_INC)/private  \
	$(NIMCORE_INC)/qt_qnx $(NIMABPAL_INC)  \
	$(NIMABPAL_INC)/private $(NIMABPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC) $(NIMNBIPAL_INC)/private  \
	$(NIMNBIPAL_INC)/qt_qnx \
	$(NIMSPEEX_INC)

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
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src/data  \
	$(PROJECT_ROOT)/../../src/datastore  \
	$(PROJECT_ROOT)/../../src/ers  \
	$(PROJECT_ROOT)/../../src/fileset  \
	$(PROJECT_ROOT)/../../src/license  \
	$(PROJECT_ROOT)/../../src/motd  \
	$(PROJECT_ROOT)/../../src/msg  \
	$(PROJECT_ROOT)/../../src/profile  \
	$(PROJECT_ROOT)/../../src/qalog  \
	$(PROJECT_ROOT)/../../src/sms  \
	$(PROJECT_ROOT)/../../src/speech  \
	$(PROJECT_ROOT)/../../src/subscription  \
	$(PROJECT_ROOT)/../../src/sync  \
	$(PROJECT_ROOT)/../../src/util  \
	$(PROJECT_ROOT)/../../src/version \
	$(PROJECT_ROOT)/../../src/oneshotasr

#===== EXTRA_LIBVPATH - a space-separated list of directories to search for library files.
EXTRA_LIBVPATH+=$(PROJECT_ROOT)/../../../thirdparty/feature_130725_map3d_main/projects/qt_qnx/speex/arm/a.le-v7

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=speex

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

