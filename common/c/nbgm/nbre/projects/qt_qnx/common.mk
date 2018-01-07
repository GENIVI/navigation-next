# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=nbre

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src  \
	$(PROJECT_ROOT)/../../src/core  \
	$(PROJECT_ROOT)/../../src/debug \
	$(PROJECT_ROOT)/../../src/nbre  \
	$(PROJECT_ROOT)/../../src/renderpal  \
	$(PROJECT_ROOT)/../../src/resource  \
	$(PROJECT_ROOT)/../../src/scene  \
	$(PROJECT_ROOT)/../../src/util

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
	$(PROJECT_ROOT)/../../include/debug  \
	$(PROJECT_ROOT)/../../../dependence\freetype-2.4.4\include  \
	$(QNX_TARGET)/usr/include  \
	$(QNX_TARGET)/usr/include/libpng

EXCLUDE_OBJS+=pngtest.o

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-Wp,-MMD,$(basename $@).d -fPIC -DNBRE_BUILD

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

