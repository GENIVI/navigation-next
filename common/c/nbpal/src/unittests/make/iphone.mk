# define APP_NAME to build executible
APP_NAME := unittests

SDKVER = 3.1.3
IPHONESDK := /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(SDKVER).sdk

# root directory of the current project
PROJ_ROOT := $(realpath $(CURDIR)/../../..)
CORETECH_ROOT := $(shell echo $(PROJ_ROOT) | sed 's/\/$(PROJ_NAME).*$$//g')
NBIPAL_ROOT := $(PROJ_ROOT)

# find branch
BRANCH ?= $(notdir $(PROJ_ROOT))
THIRDPARTY_ROOT  := $(CORETECH_ROOT)/thirdparty/$(BRANCH)

TARGET ?= iPhoneSimulator
OBJ_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)/unittests

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

# define STATIC_LIB_NAME to build static library (archive, .a)
STATIC_LIB_NAME :=

# define SHARED_LIB_NAME to buid shared library (dynamic, .so)
SHARED_LIB_NAME :=

UNITTESTS_ROOT := $(PROJ_ROOT)/src/unittests
NBIPAL_LIB_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := $(UNITTESTS_ROOT)
SRC_DIRS += $(UNITTESTS_ROOT)/iphone

# list the source files that should be excluded from the build
EXCLUDE_SRCS := 

# where header files are located.
INCLUDE_DIRS := $(UNITTESTS_ROOT)
INCLUDE_DIRS += $(THIRDPARTY_ROOT)/CUnit/include
INCLUDE_DIRS += $(NBIPAL_ROOT)/include
INCLUDE_DIRS += $(NBIPAL_ROOT)/include/iphone

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=

# extra directories to invoke make before libary or application is built.
DEPENDS_DIR := $(THIRDPARTY_ROOT)/projects/make/CUnit

# extra objects to link into the library or application
EXTRA_OBJS := $(THIRDPARTY_ROOT)/output/$(PLATFORM)/$(TARGET)/CUnit/libCUnit.a

# extra directory to invoke make AFTER library or application is built
EXTRA_TARGETS_DIR :=

# extra targets to make AFTER library or application is built
# define these targets in this makefile
EXTRA_TARGETS :=

DEBUG ?= 1
export DEBUG

# CFLAGS is passed to compiler
ifneq ($(DEBUG),0)
    CFLAGS += -g
else
    CFLAGS += -O2
endif
CFLAGS += --sysroot=$(IPHONESDK) -O0 -Wall -fPIC -x objective-c -std=c99 -D__IPHONE_OS_VERSION_MIN_REQUIRED=30200

# LDFLAGS is passed to linker
ifneq ($(DEBUG),0)
EXTRA_OBJS += $(NBIPAL_LIB_ROOT)/libnbpal.a
LDFLAGS += -lstdc++ -lpthread -ObjC -framework Foundation
else
LDFLAGS += -L$(NBIPAL_LIB_ROOT) -lnbpal -lpthread -lstdc++ -ObjC -framework Foundation
endif

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

# set GENERATE_DEPENDENCY to use gcc to generate .d dependency file
GENERATE_DEPENDENCY := 1

ARCH = $(shell uname -m)
PREFIX = /usr/bin
COMPILER_PREFIX = i686-apple-darwin10-
CROSS_COMPILE	= /Developer/Platforms/iPhoneSimulator.platform/Developer$(PREFIX)/

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)$(COMPILER_PREFIX)gcc-4.2.1
CXX		= $(CROSS_COMPILE)$(COMPILER_PREFIX)g++-4.2.1
CPP		= $(CC) -E
AR		= $(PREFIX)/ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
LIBTOOL 	= $(CROSS_COMPILE)libtool
AWK		= awk
PYTHON		= python2.5
RM = rm -rf

export ARCH CROSS_COMPILE
export AS LD CC CXX CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK PYTHON LIBTOOL

#set default target to all
.PHONY: default_target
default_target: all

.PHONY: test
test:
	@echo Running $(PROJ_NAME) unittest
	$(Q)LD_LIBRARY_PATH=$(NBIPAL_LIB_ROOT):$(LD_LIBRARY_PATH) $(OBJ_ROOT)/unittests

.PHONY: memcheck_test
memcheck_test:
	@echo Running $(PROJ_NAME) unittest with Valgrind
	$(Q)LD_LIBRARY_PATH=$(NBIPAL_LIB_ROOT):$(LD_LIBRARY_PATH) valgrind -v --leak-check=full --show-reachable=yes $(OBJ_ROOT)/unittests
