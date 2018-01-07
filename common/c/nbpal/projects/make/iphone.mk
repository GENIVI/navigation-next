# define APP_NAME to build executible
APP_NAME :=

SDKVER = 3.1.3
IPHONESDK := /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(SDKVER).sdk

# root directory of the current project
PROJ_ROOT := $(realpath $(CURDIR)/../..)
CORETECH_ROOT    := $(shell echo $(PROJ_ROOT) | sed 's/\/$(PROJ_NAME).*$$//g')

# find branch
BRANCH ?= $(notdir $(PROJ_ROOT))
THIRDPARTY_ROOT  := $(CORETECH_ROOT)/thirdparty/$(BRANCH)

#TARGET ?= x86
TARGET ?= iPhoneSimulator
OBJ_ROOT ?= $(PROJ_ROOT)/output/$(PLATFORM)/$(TARGET)

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

VERSION_FILE := $(PROJ_ROOT)/include/$(PLATFORM)/palversioninfo.h

# define IPHONE_STATIC_LIB_NAME to build static library (archive, .a)
IPHONE_STATIC_LIB_NAME := lib$(PROJ_NAME).a

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := $(PROJ_ROOT)/src/iphone/clock
SRC_DIRS += $(PROJ_ROOT)/src/iphone/file
SRC_DIRS += $(PROJ_ROOT)/src/iphone/net
SRC_DIRS += $(PROJ_ROOT)/src/iphone/pal
SRC_DIRS += $(PROJ_ROOT)/src/iphone/timer
SRC_DIRS += $(PROJ_ROOT)/src/iphone/util
SRC_DIRS += $(PROJ_ROOT)/src/util

# list the source files that should be excluded from the build
EXCLUDE_SRCS := 

# where header files are located.
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/iphone

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=

# extra directories to invoke make before libary or application is built.
DEPENDS_DIR :=

# extra objects to link into the library or application
EXTRA_OBJS :=

# extra directory to invoke make AFTER library or application is built
EXTRA_TARGETS_DIR := $(PROJ_ROOT)/src/unittests/make

# extra targets to make AFTER library or application is built
# define these targets in this makefile
EXTRA_TARGETS :=

DEBUG ?= 1
export DEBUG

# CFLAGS is passed to compiler
ifneq ($(DEBUG),0)
    CFLAGS += -g -DDEBUG_MUTEX
	CCFLAGS += -g
else
    CFLAGS += -O2
	CCFLAGS += -O2
endif

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

# set GENERATE_DEPENDENCY to use gcc to generate .d dependency file
GENERATE_DEPENDENCY := 1

CFLAGS	+= $(foreach d,$(INCLUDE_DIRS),-I$(d))
CFLAGS  += --sysroot=$(IPHONESDK) \
		   -std=c99 \
		   -W -Wall \
		   -funroll-loops \
		   -miphoneos-version-min=3.0 \
		   -Wno-unused-parameter \
		   -Wno-sign-compare

CCFLAGS	+= $(foreach d,$(INCLUDE_DIRS),-I$(d))
CCFLAGS += --sysroot=$(IPHONESDK) \
		   -W -Wall \
		   -miphoneos-version-min=3.0 \
		   -x objective-c++

LIBTOOL_FLAGS = -static -arch_only i386 -syslibroot $(IPHONESDK) -framework Foundation

LDFLAGS = -lobjc -lpthread \
		  -bind_at_load \
		  -multiply_defined suppress \
		  -w
LDFLAGS += -framework CoreFoundation
LDFLAGS += -framework Foundation
LDFLAGS += -framework UIKit
LDFLAGS += -framework CoreGraphics

#LDFLAGS += -framework AddressBookUI
#LDFLAGS += -framework AddressBook
#LDFLAGS += -framework QuartzCore
#LDFLAGS += -framework GraphicsServices
#LDFLAGS += -framework CoreSurface
#LDFLAGS += -framework CoreAudio
#LDFLAGS += -framework Celestial
#LDFLAGS += -framework AudioToolbox
#LDFLAGS += -framework WebCore
#LDFLAGS += -framework WebKit
#LDFLAGS += -framework SystemConfiguration
#LDFLAGS += -framework CFNetwork
#LDFLAGS += -framework MediaPlayer
#LDFLAGS += -framework OpenGLES
#LDFLAGS += -framework OpenAL

LDFLAGS += -F"$(IPHONESDK)/System/Library/Frameworks"
LDFLAGS += -F"$(IPHONESDK)/System/Library/PrivateFrameworks"

ARCH			= $(shell uname -m)
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

