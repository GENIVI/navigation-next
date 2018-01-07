# define APP_NAME to build executible
APP_NAME := systemtests

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

# define STATIC_LIB_NAME to build static library (archive, .a)
STATIC_LIB_NAME :=

# define SHARED_LIB_NAME to buid shared library (dynamic, .so)
SHARED_LIB_NAME :=

#try to guess where NBIPAL is located.
ifeq ($(NBIPAL_ROOT),)
NBIPAL_ROOT := $(realpath $(PROJ_ROOT)/../../nbpal/$(BRANCH))
endif

ifeq ($(NBIPAL_ROOT),)
NBIPAL_ROOT := $(realpath $(PROJ_ROOT)/../../../nbpal/$(BRANCH))
endif

ifeq ($(NBIPAL_ROOT),)
$(warning $(NBIPAL_ROOT))
$(error Unable to guess where NBIPAL is located, please set environment variable NBIPAL_ROOT explictly)
endif

#try to guess where ABPAL is located.
ifeq ($(ABPAL_ROOT),)
ABPAL_ROOT := $(realpath $(PROJ_ROOT)/../../abpal/$(BRANCH))
endif

ifeq ($(ABPAL_ROOT),)
ABPAL_ROOT := $(realpath $(PROJ_ROOT)/../../../abpal/$(BRANCH))
endif

ifeq ($(ABPAL_ROOT),)
$(warning $(ABPAL_ROOT))
$(error Unable to guess where NBIPAL is located, please set environment variable ABPAL_ROOT explictly)
endif

#try to guess where NBCORESERVICES is located.
ifeq ($(NIMCORESERVICES_ROOT),)
NIMCORESERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../coreservices/$(BRANCH))
endif

ifeq ($(NIMCORESERVICES_ROOT),)
NIMCORESERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../../coreservices/$(BRANCH))
endif

ifeq ($(NIMCORESERVICES_ROOT),)
$(error Unable to guess where NIMCORESERVICES is located, please set environment variable NIMCORESERVICES_ROOT explictly)
endif

#try to guess where NIMNBSERVICES is located.
ifeq ($(NIMNBSERVICES_ROOT),)
NIMNBSERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../nbservices/$(BRANCH))
endif

ifeq ($(NIMNBSERVICES_ROOT),)
NIMNBSERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../../nbservices/$(BRANCH))
endif

ifeq ($(NIMNBSERVICES_ROOT),)
$(error Unable to guess where NIMNBSERVICES is located, please set environment variable NIMNBSERVICES_ROOT explictly)
endif

#try to guess where NIMABSERVICES is located.
ifeq ($(NIMABSERVICES_ROOT),)
NIMABSERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../abservices/$(BRANCH))
endif

ifeq ($(NIMABSERVICES_ROOT),)
NIMABSERVICES_ROOT := $(realpath $(PROJ_ROOT)/../../../abservices/$(BRANCH))
endif

ifeq ($(NIMABSERVICES_ROOT),)
$(error Unable to guess where NIMNBSERVICES is located, please set environment variable NIMABSERVICES_ROOT explictly)
endif

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := $(PROJ_ROOT)/src
SRC_DIRS += $(PROJ_ROOT)/src/linux

# list the source files that should be excluded from the build
EXCLUDE_SRCS := $(PROJ_ROOT)/src/testroute.c

# where header files are located.
INCLUDE_DIRS := $(NBIPAL_ROOT)/include
INCLUDE_DIRS += $(NBIPAL_ROOT)/include/linux
INCLUDE_DIRS += $(ABPAL_ROOT)/include
INCLUDE_DIRS += $(ABPAL_ROOT)/include/linux
INCLUDE_DIRS += $(NIMCORESERVICES_ROOT)/include
INCLUDE_DIRS += $(NIMCORESERVICES_ROOT)/include/linux
INCLUDE_DIRS += $(NIMNBSERVICES_ROOT)/include
INCLUDE_DIRS += $(NIMNBSERVICES_ROOT)/include/linux
INCLUDE_DIRS += $(NIMABSERVICES_ROOT)/include
INCLUDE_DIRS += $(NIMABSERVICES_ROOT)/include/linux
INCLUDE_DIRS += $(THIRDPARTY_ROOT)/CUnit/include
INCLUDE_DIRS += $(THIRDPARTY_ROOT)/ArgTable/include
INCLUDE_DIRS += $(PROJ_ROOT)/src

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=

# extra directories to invoke make before libary or application is built.
DEPENDS_DIR := $(THIRDPARTY_ROOT)/projects/make/CUnit
DEPENDS_DIR += $(THIRDPARTY_ROOT)/projects/make/ArgTable

# extra objects to link into the library or application
EXTRA_OBJS := $(THIRDPARTY_ROOT)/output/$(PLATFORM)/$(TARGET)/CUnit/libCUnit.a
EXTRA_OBJS += $(THIRDPARTY_ROOT)/output/$(PLATFORM)/$(TARGET)/ArgTable/libArgTable.a

# extra directory to invoke make AFTER library or application is built
EXTRA_TARGETS_DIR :=

# extra targets to make AFTER library or application is built
# define these targets in this makefile
EXTRA_TARGETS := copy_data_files

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
CFLAGS += -D__unused="__attribute__((unused))" -DDEBUG_LOG_ALL
CFLAGS += -DTEST_NEW_API
CFLAGS += -fno-strict-aliasing

# LDFLAGS is passed to linker
ifneq ($(DEBUG),0)
EXTRA_OBJS += $(NIMABSERVICES_ROOT)/output/linux/$(TARGET)/libabservices.a
EXTRA_OBJS += $(NIMNBSERVICES_ROOT)/output/linux/$(TARGET)/libnbservices.a
EXTRA_OBJS += $(NIMCORESERVICES_ROOT)/output/linux/$(TARGET)/libcoreservices.a
EXTRA_OBJS += $(NBIPAL_ROOT)/output/linux/$(TARGET)/libnbpal.a
EXTRA_OBJS += $(ABPAL_ROOT)/output/linux/$(TARGET)/libabpal.a
LDFLAGS += -lstdc++
else
LDFLAGS += -L$(NIMABSERVICES_ROOT)/output/linux/$(TARGET) -labservices
LDFLAGS += -L$(NIMNBSERVICES_ROOT)/output/linux/$(TARGET) -lnbservices
LDFLAGS += -L$(NIMCORESERVICES_ROOT)/output/linux/$(TARGET) -lcoreservices
LDFLAGS += -L$(NBIPAL_ROOT)/output/linux/$(TARGET) -lnbpal
LDFLAGS += -L$(ABPAL_ROOT)/output/linux/$(TARGET) -labpal
endif

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
LDFLAGS += $(GLIB_LIBS)

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
PYTHON		= python2.4
RM = rm -rf

export ARCH CROSS_COMPILE
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK PYTHON

#set default target to all
.PHONY: default_target
default_target: all

.PHONY: copy_data_files
copy_data_files:
	$(Q)cp -f $(NBIPAL_ROOT)/output/linux/$(TARGET)/libnbpal* $(OBJ_ROOT)
	$(Q)cp -f $(ABPAL_ROOT)/output/linux/$(TARGET)/libabpal* $(OBJ_ROOT)
	$(Q)cp -f $(NIMCORESERVICES_ROOT)/output/linux/$(TARGET)/libcoreservices* $(OBJ_ROOT)
	$(Q)cp -f $(NIMNBSERVICES_ROOT)/output/linux/$(TARGET)/libnbservices* $(OBJ_ROOT)
	$(Q)cp -f $(NIMABSERVICES_ROOT)/output/linux/$(TARGET)/libabservices* $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/*qcp $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/*csv $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/*gps $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/*wav $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/qalogs/* $(OBJ_ROOT)
	$(Q)cp -f $(PROJ_ROOT)/tpslib/compiled/*.tpl $(OBJ_ROOT)
	$(Q)rm -f $(PROJ_ROOT)/supportfiles/navfileset/directions/config.tps
	$(Q)rm -f $(PROJ_ROOT)/supportfiles/navfileset/michelle/config.tps
	$(Q)rm -f $(PROJ_ROOT)/supportfiles/navfileset/routing-images/config.tps
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/directions/directions-en-us.sexp $(PROJ_ROOT)/supportfiles/navfileset/directions/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/michelle/voices.sexp $(PROJ_ROOT)/supportfiles/navfileset/michelle/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/routing-images/images.sexp $(PROJ_ROOT)/supportfiles/navfileset/routing-images/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl
	$(Q)cp -f -a $(PROJ_ROOT)/supportfiles/navfileset/* $(OBJ_ROOT)

.PHONY: test
test:
	@echo Running $(APP_NAME)
	$(Q)cd $(OBJ_ROOT) && LD_LIBRARY_PATH=$(NBIPAL_ROOT)/output/linux/$(TARGET):$(ABPAL_ROOT)/output/linux/$(TARGET):$(NIMCORESERVICES_ROOT)/output/linux/$(TARGET):$(NIMNBSERVICES_ROOT)/output/linux/$(TARGET):$(NIMABSERVICES_ROOT)/output/linux/$(TARGET):$(LD_LIBRARY_PATH) ./$(APP_NAME)

.PHONY: memcheck_test
memcheck_test:
	@echo Running $(APP_NAME) with Valgrind
	$(Q)cd $(OBJ_ROOT) && LD_LIBRARY_PATH=$(NBIPAL_ROOT)/output/linux/$(TARGET):$(ABPAL_ROOT)/output/linux/$(TARGET):$(NIMCORESERVICES_ROOT)/output/linux/$(TARGET):$(NIMNBSERVICES_ROOT)/output/linux/$(TARGET):$(NIMABSERVICES_ROOT)/output/linux/$(TARGET):$(LD_LIBRARY_PATH) valgrind -v --leak-check=full --show-reachable=yes --num-callers=50 ./$(APP_NAME)

.PHONY: install
install:
	$(Q)mkdir -p $(INSTALL_PREFIX)/root
	$(Q)cp -f $(OBJ_ROOT)/systemtests $(INSTALL_PREFIX)/root
	$(Q)cp -f $(PROJ_ROOT)/supportfiles/* $(INSTALL_PREFIX)/root
	$(Q)cp -f $(PROJ_ROOT)/tpslib/compiled/*.tpl $(INSTALL_PREFIX)/root
	$(Q)cp -f -a $(PROJ_ROOT)/supportfiles/navfileset/* $(INSTALL_PREFIX)/root

