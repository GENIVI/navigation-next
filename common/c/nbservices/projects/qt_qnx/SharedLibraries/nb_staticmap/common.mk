# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=$(PROJECT_ROOT)/../../../../../../build/$(BRANCH)/version_ccc.c

# Next lines are for C++ projects only

EXTRA_SUFFIXES+=cxx cpp

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+= \
	-lang-c++  \
	-Wl,-z  \
	 relro  \
	-Wl,--as-needed  \
	-Wl,-z,now

VFLAG_g=-gstabs+

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+= \
	-fPIC  \
	-shared \
	-D_FORTIFY_SOURCE=2 \
	-fstack-protector-strong  \
	-g  \
	-DENABLE_LOGGING_TO_OUTPUT

ifdef INSTRUMENT	
CCFLAGS+= -g -O0 -finstrument-functions
LDFLAGS+= -lprofilingS
endif

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+= \
	$(PROJECT_ROOT)/../../../../include  \
	$(PROJECT_ROOT)/../../../../include/protected  \
	$(PROJECT_ROOT)/../../../../include/private  \
	$(PROJECT_ROOT)/../../../../include/qt_qnx  \
	$(NIMCORE_INC)/protected  \
	$(NIMCORE_INC)/private  \
	$(NIMCORE_INC)/qt_qnx  \
	$(NIMCORE_INC)  \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)/private  \
	$(NIMNBIPAL_INC)/nbpal  \
	$(NIMNBIPAL_INC)/boost  \
	$(NIMNBIPAL_INC)  \
	$(NIMABPAL_INC)  \
	$(NIMABPAL_INC)/qt_qnx

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../../../src/rastermap

ifdef INSTRUMENT
LIBS += profilingS
endif

#===== VERSION_TAG_SO - version tag for SONAME. Use it only if you don't like SONAME_VERSION
override VERSION_TAG_SO=

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

