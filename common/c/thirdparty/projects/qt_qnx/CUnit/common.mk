# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=cunit

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-w9 -O0 -v -g

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-E -M -Wl,-z,now

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../../CUnit/Sources  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Automated  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Basic  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Console  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Curses  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Framework  \
	$(PROJECT_ROOT)/../../../CUnit/Sources/Test

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../../CUnit/include

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

