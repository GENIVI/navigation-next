# define APP_NAME to build executible
APP_NAME :=

PROJ_ROOT := $(realpath $(CURDIR)/../../..)

SDKVER = 3.1.3
IPHONESDK := /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(SDKVER).sdk

TARGET ?= iPhoneSimulator
OBJ_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)/CUnit

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

# define IPHONE_STATIC_LIB_NAME to build static library (archive, .a)
IPHONE_STATIC_LIB_NAME := lib$(PROJ_NAME).a

DEBUG ?= 1
export DEBUG

# CFLAGS is passed to compiler
ifneq ($(DEBUG),0)
    CFLAGS += -g
else
    CFLAGS += -O2
endif
CFLAGS += --sysroot=$(IPHONESDK) -x c
CFLAGS += -mfix-and-continue -mmacosx-version-min=10.5 -gdwarf-2 -Wall -std=c99 -funroll-loops -pipe
CFLAGS += -Wno-trigraphs -fpascal-strings -fasm-blocks -fmessage-length=0
CFLAGS += -D__IPHONE_OS_VERSION_MIN_REQUIRED=30200

LIBTOOL_FLAGS = -static -arch_only $(ARCH) -syslibroot $(IPHONESDK)

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

# set GENERATE_DEPENDENCY to use gcc to generate .d dependency file
GENERATE_DEPENDENCY := 1

ARCH		= $(shell uname -m)

PREFIX = /usr/bin/
CROSS_COMPILE	= /Developer/Platforms/iPhoneSimulator.platform/Developer
PLATFORM_PREFIX = i686-apple-darwin10-

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)$(PREFIX)as
LD		= $(CROSS_COMPILE)$(PREFIX)ld
CC		= $(CROSS_COMPILE)$(PREFIX)$(PLATFORM_PREFIX)gcc-4.2.1
CXX		= $(CROSS_COMPILE)$(PREFIX)$(PLATFORM_PREFIX)g++-4.2.1
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)$(PREFIX)ar
NM		= $(CROSS_COMPILE)$(PREFIX)nm
STRIP		= $(CROSS_COMPILE)$(PREFIX)strip
OBJCOPY		= $(CROSS_COMPILE)$(PREFIX)objcopy
OBJDUMP		= $(CROSS_COMPILE)$(PREFIX)objdump
LIBTOOL		= $(CROSS_COMPILE)$(PREFIX)libtool
AWK		= awk
PYTHON		= python2.5
RM = rm -rf

export ARCH CROSS_COMPILE
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK PYTHON LIBTOOL

