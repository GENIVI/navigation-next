# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=kml

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

# Next lines are for C++ projects only

EXTRA_SUFFIXES+=cxx cc

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-E -M -Wl,-z relro -Wl,--as-needed -Wl,-z,now

VFLAG_g=-gstabs+

CCFLAGS+=-w9 -D_REENTRANT -DDATADIR -fpermissive -fPIC \
	-D_QNXMAPS -Dunix -fstack-protector-strong -v -g

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=${QNX_TARGET}/../target-override/usr/include  \
	$(QNX_TARGET)/usr/include ${QNX_TARGET}/usr/include/cpp/c  \
	${QNX_TARGET}/usr/include/cpp  \
	$(PROJECT_ROOT)/../../../kml/third_party/boost_1_34_1  \
	$(PROJECT_ROOT)/../../../kml/third_party/uriparser-0.7.5/include  \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib  \
	$(PROJECT_ROOT)/../../.. $(PROJECT_ROOT)/../../../base  \
	$(PROJECT_ROOT)/../../../dom  \
	$(PROJECT_ROOT)/../../../engine

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../../kml/base \
	$(PROJECT_ROOT)/../../../kml/dom \
	$(PROJECT_ROOT)/../../../kml/engine \
	$(PROJECT_ROOT)/../../../kml/third_party/uriparser-0.7.5/lib

SRCV+=$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/ioapi \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/iomem_simple \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/miniunz \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/minizip \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/mztools \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/unzip \
	$(PROJECT_ROOT)/../../../kml/third_party/zlib-1.2.3/contrib/minizip/zip

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

