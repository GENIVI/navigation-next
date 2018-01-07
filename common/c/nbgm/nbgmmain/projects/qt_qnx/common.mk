# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=nbgm

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

# Next lines are for C++ projects only
EXTRA_SUFFIXES+=cxx cpp

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-lang-c++ -Wl,-E -M, -Wl,-z relro -Wl,--as-needed -Wl,-z,now

VFLAG_g=-gstabs+

#===== CCFLAGS - add the flags to the C compiler command line.
CCFLAGS+=-w9 -fPIC -D_REENTRANT -fstack-protector-strong -v -g -DENABLE_LOGGING_TO_OUTPUT

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src  \
	$(PROJECT_ROOT)/../../src/nbgm  \
	$(PROJECT_ROOT)/../../src/qt_qnx  \
	$(PROJECT_ROOT)/../../src/nbgmmanager \
	$(PROJECT_ROOT)/../../src/nbgmmapdata  \
	$(PROJECT_ROOT)/../../src/nbgmservice  \
	$(PROJECT_ROOT)/../../src/util \
	$(PROJECT_ROOT)/../../../rendersystem/gles/src \
	$(PROJECT_ROOT)/../../../rendersystem/gles2/src

EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src  \
	$(PROJECT_ROOT)/../../../nbre/src/core  \
	$(PROJECT_ROOT)/../../../nbre/src/debug \
	$(PROJECT_ROOT)/../../../nbre/src/nbre  \
	$(PROJECT_ROOT)/../../../nbre/src/renderpal  \
	$(PROJECT_ROOT)/../../../nbre/src/resource  \
	$(PROJECT_ROOT)/../../../nbre/src/scene  \
	$(PROJECT_ROOT)/../../../nbre/src/util

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/private  \
	$(NIMNBIPAL_INC) \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)/qt_qnx/unzip  \
	$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/qt_qnx  \
	$(PROJECT_ROOT)/../../include/protected  \
	$(PROJECT_ROOT)/../../include/private  \
	$(PROJECT_ROOT)/../../../rendersystem/gles/include/private \
	$(PROJECT_ROOT)/../../../rendersystem/gles/include/ \
	$(PROJECT_ROOT)/../../../rendersystem/gles2/include/private \
	$(PROJECT_ROOT)/../../../rendersystem/gles2/include/ \
	$(PROJECT_ROOT)/../../../nbre/include/  \
	$(PROJECT_ROOT)/../../../nbre/include/protected \
	$(PROJECT_ROOT)/../../../nbre/include/private  \
	$(PROJECT_ROOT)/../../../nbre/include/debug  \
	$(PROJECT_ROOT)/../../../nbre/include/qt_qnx  \
	$(QNX_TARGET)/usr/include  \
	$(QNX_TARGET)/usr/include/libpng  \
	$(QNX_TARGET)/usr/include/freetype2 \
	$(QNX_TARGET)/usr/include/freetype2\freetype \
	$(NIMCORE_INC) \
	$(THIRDPARTY_MAIN)/libjpeg-turbo/include
	
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../../nbre/include  \
	$(PROJECT_ROOT)/../../../nbre/include/qt_qnx  \
	$(PROJECT_ROOT)/../../../nbre/include/protected  \
	$(PROJECT_ROOT)/../../../nbre/include/private  \
	$(PROJECT_ROOT)/../../../nbre/include/debug

EXCLUDE_OBJS+=pngtest.o nbrejpegcodec.o


include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

