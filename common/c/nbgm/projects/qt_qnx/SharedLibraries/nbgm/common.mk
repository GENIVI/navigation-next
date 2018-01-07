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
	-D_REENTRANT \
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
	$(PROJECT_ROOT)/../../../../nbgmmain/src/nbgm  \
	$(PROJECT_ROOT)/../../../../nbgmmain/src/qt_qnx  \
	$(PROJECT_ROOT)/../../../../nbgmmain/src/nbgmmanager  \
	$(PROJECT_ROOT)/../../../../nbgmmain/src/nbgmmapdata  \
	$(PROJECT_ROOT)/../../../../nbgmmain/src/nbgmservice  \
	$(PROJECT_ROOT)/../../../../nbgmmain/src/util  \
	$(PROJECT_ROOT)/../../../../rendersystem/gles2/src  \
	$(PROJECT_ROOT)/../../../../nbre/src/core  \
	$(PROJECT_ROOT)/../../../../nbre/src/debug  \
	$(PROJECT_ROOT)/../../../../nbre/src/nbre  \
	$(PROJECT_ROOT)/../../../../nbre/src/renderpal  \
	$(PROJECT_ROOT)/../../../../nbre/src/resource  \
	$(PROJECT_ROOT)/../../../../nbre/src/scene  \
	$(PROJECT_ROOT)/../../../../nbre/src/util


#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.

EXTRA_INCVPATH+= \
	$(NIMNBIPAL_INC)  \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)/qt_qnx/unzip  \
	$(PROJECT_ROOT)/../../../../include/qt_qnx  \
	$(PROJECT_ROOT)/../../../../nbgmmain/include  \
	$(PROJECT_ROOT)/../../../../nbgmmain/include/qt_qnx  \
	$(PROJECT_ROOT)/../../../../nbgmmain/include/protected  \
	$(PROJECT_ROOT)/../../../../nbgmmain/include/private  \
	$(PROJECT_ROOT)/../../../../rendersystem/gles2/include/private  \
	$(PROJECT_ROOT)/../../../../rendersystem/gles2/include/  \
	$(QNX_TARGET)/usr/include  \
	$(QNX_TARGET)/usr/include/freetype2  \
	$(QNX_TARGET)/usr/include/freetype2\freetype  \
	$(NIMCORE_INC)  \
	$(THIRDPARTY_MAIN)/libjpeg-turbo/include  \
	$(PROJECT_ROOT)/../../../../nbre/include  \
	$(PROJECT_ROOT)/../../../../nbre/include/qt_qnx  \
	$(PROJECT_ROOT)/../../../../nbre/include/protected  \
	$(PROJECT_ROOT)/../../../../nbre/include/private  \
	$(PROJECT_ROOT)/../../../../nbre/include/debug

EXCLUDE_OBJS+=pngtest.o nbrejpegcodec.o

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

