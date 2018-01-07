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

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src  \
	$(PROJECT_ROOT)/../../src/nbgmcore  \
	$(PROJECT_ROOT)/../../src/nbgmmanager \
	$(PROJECT_ROOT)/../../src/nbgmmapkit  \
	$(PROJECT_ROOT)/../../src/nbgmnavkit  \
	$(PROJECT_ROOT)/../../src/nbgmrenderingengine  \
	$(PROJECT_ROOT)/../../src/nbgmrenderingengine/opengles  \
	$(PROJECT_ROOT)/../../src/nbgmresource  \
	$(PROJECT_ROOT)/../../src/nbgmservice  \
	$(PROJECT_ROOT)/../../src/util  \
	$(PROJECT_ROOT)/../../src/nbgm  \
	$(PROJECT_ROOT)/../../src/platfrom_common	\
	$(PROJECT_ROOT)/../../../rendersystem/gles2/src

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/private $(NIMNBIPAL_INC)  \
	$(NIMNBIPAL_INC)/qt_qnx $(NIMNBIPAL_INC)/qt_qnx/unzip  \
	$(NIMNBIPAL_INC)/private $(PROJECT_ROOT)/../../include/pal  \
	$(PROJECT_ROOT)/../../include/qt_qnx  \
	$(PROJECT_ROOT)/../../include/protected  \
	$(PROJECT_ROOT)/../../src/platfrom_common \
	$(PROJECT_ROOT)/../../../rendersystem/gles2/include/private \
	$(PROJECT_ROOT)/../../../rendersystem/gles2/include/ \
	$(QNX_TARGET)/usr/include

EXCLUDE_OBJS+=pngtest.o nbrejpegcodec.o

#===== CCFLAGS - add the flags to the C compiler command line.
CCFLAGS+=-Wp,-MMD,$(basename $@).d -fPIC -DNBGM_MAP3D -fstack-protector-strong -Os -DENABLE_LOGGING_TO_OUTPUT

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-z relro -Wl,--as-needed

#===== EXTRA_CLEAN - additional files to delete when cleaning the project.
EXTRA_CLEAN+=*.d

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

-include *.d

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

