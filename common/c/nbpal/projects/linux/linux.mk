# define APP_NAME to build executible
APP_NAME :=

# root directory of the current project
PROJ_ROOT := $(realpath $(CURDIR)/../..)
CORETECH_ROOT    := $(shell echo $(PROJ_ROOT) | sed 's/\/$(PROJ_NAME).*$$//g')

# find branch
BRANCH ?= $(notdir $(PROJ_ROOT))
THIRDPARTY_ROOT  := $(CORETECH_ROOT)/thirdparty/$(BRANCH)
TARGET ?= x86
OBJ_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

VERSION_FILE := $(PROJ_ROOT)/include/$(PLATFORM)/palversioninfo.h

# define STATIC_LIB_NAME to build static library (archive, .a)
STATIC_LIB_NAME := lib$(PROJ_NAME).a

# define SHARED_LIB_NAME to buid shared library (dynamic, .so)
SHARED_LIB_NAME := lib$(PROJ_NAME).so

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := $(PROJ_ROOT)/src/linux/clock
SRC_DIRS += $(PROJ_ROOT)/src/linux/file
SRC_DIRS += $(PROJ_ROOT)/src/linux/net
SRC_DIRS += $(PROJ_ROOT)/src/linux/net/rpcnetwork
SRC_DIRS += $(PROJ_ROOT)/src/linux/net/rpcnetwork/gen-cpp
SRC_DIRS += $(PROJ_ROOT)/src/linux/pal
SRC_DIRS += $(PROJ_ROOT)/src/linux/timer
SRC_DIRS += $(PROJ_ROOT)/src/linux/util
SRC_DIRS += $(PROJ_ROOT)/src/linux/display
SRC_DIRS += $(PROJ_ROOT)/src/linux/font
SRC_DIRS += $(PROJ_ROOT)/src/linux/uitaskmanager
SRC_DIRS += $(PROJ_ROOT)/src/linux/synchronization
SRC_DIRS += $(PROJ_ROOT)/src/linux/myplaces
SRC_DIRS += $(PROJ_ROOT)/src/linux/imageblender
SRC_DIRS += $(PROJ_ROOT)/src/util
SRC_DIRS += $(PROJ_ROOT)/src/unittest
SRC_DIRS += $(PROJ_ROOT)/src/posix/taskqueue
SRC_DIRS += $(PROJ_ROOT)/src/linux/radio
SRC_DIRS += $(PROJ_ROOT)/src/common/finishfunction
SRC_DIRS += $(PROJ_ROOT)/src/common/net
SRC_DIRS += $(PROJ_ROOT)/src/common/posix
SRC_DIRS += $(PROJ_ROOT)/src/common/gl
SRC_DIRS += $(PROJ_ROOT)/src/common/unzip
SRC_DIRS += $(PROJ_ROOT)/src/common/database
SRC_DIRS += $(PROJ_ROOT)/src/common/taskqueue
SRC_DIRS += $(PROJ_ROOT)/src/linux/testlog
SRC_DIRS += $(PROJ_ROOT)/src/linux/uitaskqueue

# list the source files that should be excluded from the build
EXCLUDE_SRCS :=

# where header files are located.
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/linux
INCLUDE_DIRS +=   \
	$(PROJ_ROOT)/src/posix/util \
	$(PROJ_ROOT)/src/posix/taskqueue \
	$(PROJ_ROOT)/src/linux/taskqueue \
         $(PROJ_ROOT)/src/common/database  \
         $(PROJ_ROOT)/src/common/finishfunction  \
         $(PROJ_ROOT)/src/common/taskqueue  \
         $(PROJ_ROOT)/src/common/net \
         $(PROJ_ROOT)/src/common/posix \
         $(PROJ_ROOT)/src/common/gl \
         $(PROJ_ROOT)/src/linux/font \
		 $(PROJ_ROOT)/src/common/unzip\
	$(THIRDPARTY_ROOT)/sqlite/include \
	$(THIRDPARTY_ROOT)/freetype-2.4.4/include

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=
# extra directories to invoke make before libary or application is built.
DEPENDS_DIR := $(THIRDPARTY_ROOT)/projects/linux/${TARGET}/sqlite

# extra objects to link into the library or application
EXTRA_OBJS := $(THIRDPARTY_ROOT)/output/$(PLATFORM)/$(TARGET)/sqlite/libsqlite.a

# extra directory to invoke make AFTER library or application is built
EXTRA_TARGETS_DIR := #$(PROJ_ROOT)/src/unittests/make

# extra targets to make AFTER library or application is built
# define these targets in this makefile
EXTRA_TARGETS :=

DEBUG ?= 0
export DEBUG

# CFLAGS is passed to compiler
ifneq ($(DEBUG),0)
    CFLAGS += -g
else
    CFLAGS += -O2
endif
CFLAGS += -Wall -fPIC
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fvisibility=default

# LDFLAGS is passed to linker
#LDFLAGS += -lstdc++

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

# set GENERATE_DEPENDENCY to use gcc to generate .d dependency file
GENERATE_DEPENDENCY := 1

ifeq ($(TARGET),limo-x86)
INSTALL_PREFIX ?= /usr/local/lgmlp-dev/simulator/file-system/bootstrap/8.04_hardy
PKG_CONFIG_PATH:=/opt/limo/lib/pkgconfig:/opt/lgmlp-dev/lib/pkgconfig:$(PKG_CONFIG_PATH)
LD_LIBRARY_PATH := /opt/lgmlp-dev/simulator/lib:/opt/limo/lib:$(INSTALL_PREFIX)/usr/local/lib:$(INSTALL_PREFIX)/usr/lib:$(LD_LIBRARY_PATH)

else ifeq ($(TARGET),webos-arm)
WEBOS_SDK_ROOT ?= /opt/PalmCarrierSDK
CROSS_COMPILE?=arm-none-linux-gnueabi-
INSTALL_PREFIX ?= $(WEBOS_SDK_ROOT)/ccc
PKG_CONFIG_PATH :=$(WEBOS_SDK_ROOT)/staging/arm-none-linux-gnueabi/lib/pkgconfig:$(PKG_CONFIG_PATH)
LD_LIBRARY_PATH := $(WEBOS_SDK_ROOT)/staging/arm-none-linux-gnueabi/lib:$(INSTALL_PREFIX)/usr/local/lib:$(INSTALL_PREFIX)/usr/lib:$(LD_LIBRARY_PATH)
CFLAGS += -Wno-multichar -D_REENTRANT -pthread -isystem $(WEBOS_SDK_ROOT)/staging/arm-none-linux-gnueabi/include -fno-omit-frame-pointer -D_DEBUG -DERROR_CHECK_LEVEL=2 -march=armv6
PATH := $(WEBOS_SDK_ROOT)/arm-2007q3/bin:$(PATH)
export PATH

else ifeq ($(TARGET),webos-x86)
GLIB_2_14_4_ROOT ?= $(HOME)/local/glib-2.14.4
PKG_CONFIG_PATH :=$(GLIB_2_14_4_ROOT)/lib/pkgconfig:$(PKG_CONFIG_PATH)
LD_LIBRARY_PATH := $(GLIB_2_14_4_ROOT)/lib:$(INSTALL_PREFIX)/usr/local/lib:$(INSTALL_PREFIX)/usr/lib:$(LD_LIBRARY_PATH)

else ifeq ($(TARGET),arm)
CROSS_COMPILE?=arm-linux-gnueabi-
else ifeq ($(TARGET),armhf)
CROSS_COMPILE?=arm-linux-gnueabihf-

else
INSTALL_PREFIX ?= /
LD_LIBRARY_PATH := $(LD_LIBRARY_PATH)
endif

export INSTALL_PREFIX
export LD_LIBRARY_PATH
export PKG_CONFIG_PATH

GLIB_CFLAGS ?= $(shell PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --cflags glib-2.0 gobject-2.0 gthread-2.0)
GLIB_LIBS ?= $(shell PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs glib-2.0 gobject-2.0 gthread-2.0)

CFLAGS += $(GLIB_CFLAGS)
#LDFLAGS += $(GLIB_LIBS)

export GLIB_CFLAGS GLIB_LIBS

ARCH		?= $(shell uname -m)
CROSS_COMPILE	?=

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
PYTHON		= python
RM = rm -rf

export ARCH CROSS_COMPILE
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK PYTHON

#set default target to all
.PHONY: default_target
default_target: all

.PHONY: test
test:
	$(Q)$(MAKE) -C $(PROJ_ROOT)/src/unittests/make $@

.PHONY: memcheck_test
memcheck_test:
	$(Q)$(MAKE) -C $(PROJ_ROOT)/src/unittests/make $@

.PHONY: install
install:
	$(Q)cp $(OBJ_ROOT)/lib*.so* $(INSTALL_PREFIX)/usr/lib

