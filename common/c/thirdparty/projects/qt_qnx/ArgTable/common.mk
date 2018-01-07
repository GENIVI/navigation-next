# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=argtable

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-w9 -O0 -D__GNU_LIBRARY__ -v -g

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-E -M -Wl,-z,now

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
SRCVPATH+=$(PROJECT_ROOT)/../../../ArgTable/Sources
SRCS+=arg_dbl arg_end arg_file arg_int arg_lit arg_rem arg_rex  \
arg_str argtable2 getopt getopt1

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../../ArgTable/include

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

