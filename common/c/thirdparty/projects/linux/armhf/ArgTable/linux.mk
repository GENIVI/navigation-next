# define APP_NAME to build executible
APP_NAME :=

PROJ_ROOT := $(realpath $(CURDIR)/../../../..)

TARGET ?= armhf
OBJ_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)/ArgTable

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

# define STATIC_LIB_NAME to build static library (archive, .a)
STATIC_LIB_NAME := lib$(PROJ_NAME).a

# define SHARED_LIB_NAME to buid shared library (dynamic, .so)
SHARED_LIB_NAME := lib$(PROJ_NAME).so

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
CFLAGS += -DHAVE_STDLIB_H -DNDEBUG

# LDFLAGS is passed to linker
LDFLAGS :=

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

else ifeq ($(TARGET),armhf)
CROSS_COMPILE ?= arm-linux-gnueabihf-

else
INSTALL_PREFIX ?= /
LD_LIBRARY_PATH := $(LD_LIBRARY_PATH)
endif

export INSTALL_PREFIX
export LD_LIBRARY_PATH
export PKG_CONFIG_PATH

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

