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

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+= \
	$(PROJECT_ROOT)/../../../../speex/private  \
	$(PROJECT_ROOT)/../../../../speex/libogg-1.3.0/src  \
	$(PROJECT_ROOT)/../../../../speex/speex-1.2rc1/libspeex

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+= \
	$(QNX_TARGET)/usr/include/freetype2  \
	$(PROJECT_ROOT)/../../../../speex/public  \
	$(PROJECT_ROOT)/../../../../speex/qt_qnx  \
	$(PROJECT_ROOT)/../../../../speex/speex-1.2rc1/include  \
	$(PROJECT_ROOT)/../../../../speex/speex-1.2rc1/include/speex  \
	$(PROJECT_ROOT)/../../../../speex/libogg-1.3.0/include  \
	$(PROJECT_ROOT)/../../../../speex/libogg-1.3.0/include/ogg  \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)

#===== EXTRA_CLEAN - additional files to delete when cleaning the project.
EXTRA_CLEAN+=*.ii

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

