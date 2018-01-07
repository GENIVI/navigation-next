# define APP_NAME to build executible
APP_NAME :=

SHELL = cmd.exe /C

PROJ_ROOT := ../..
CORETECH_PATH    := $(CORETECH_ROOT) #supplied by user set enviromnment variable
CORETECH_DIR = $(subst \,/,$(strip $(CORETECH_PATH)))
THIRDPARTY_ROOT  := $(THIRDPARTY_MAIN)
CC_PATH ?= $(THIRDPARTY_ROOT)/arm/brew/bin/
MK_BREW_VERSION ?= 315

OUT_ROOT := $(PROJ_ROOT)/output/$(PLATFORM)
OUT_DIR := $(subst /,\,$(OUT_ROOT))
CHECK_OUT_ROOT := $(shell mkdir $(OUT_DIR))

OBJ_ROOT := $(PROJ_ROOT)/output/$(PLATFORM)/brew$(MK_BREW_VERSION)
OBJ_DIR := $(subst /,\,$(OBJ_ROOT))
CHECK_OBJ_ROOT := $(shell mkdir $(OBJ_DIR))

VERSION_FILE := $(PROJ_ROOT)/include/$(PLATFORM)/nbversioninfo.h
VERSION_FILE := $(subst /,\,$(strip $(VERSION_FILE)))

# define STATIC_LIB_NAME to build static library (archive, .a)
PARTIAL_LIB_NAME := $(PROJ_NAME).o

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := $(PROJ_ROOT)/src/data
SRC_DIRS += $(PROJ_ROOT)/src/geocode
SRC_DIRS += $(PROJ_ROOT)/src/gps
SRC_DIRS += $(PROJ_ROOT)/src/main
SRC_DIRS += $(PROJ_ROOT)/src/map
SRC_DIRS += $(PROJ_ROOT)/src/nav
SRC_DIRS += $(PROJ_ROOT)/src/nbcommon
SRC_DIRS += $(PROJ_ROOT)/src/network
SRC_DIRS += $(PROJ_ROOT)/src/proxpoi
SRC_DIRS += $(PROJ_ROOT)/src/qalog
SRC_DIRS += $(PROJ_ROOT)/src/spatial
SRC_DIRS += $(PROJ_ROOT)/src/util
SRC_DIRS += $(PROJ_ROOT)/src/vectortile
SRC_DIRS += $(PROJ_ROOT)/src/rastermap
SRC_DIRS += $(PROJ_ROOT)/src/rastertile
SRC_DIRS += $(PROJ_ROOT)/src/analytics
SRC_DIRS += $(PROJ_ROOT)/src/location
SRC_DIRS += $(PROJ_ROOT)/src/locationservices
SRC_DIRS += $(PROJ_ROOT)/src/wifiprobes
SRC_DIRS += $(PROJ_ROOT)/src/enhancedcontent
SRC_DIRS += $(PROJ_ROOT)/src/tileservice
SRC_DIRS += $(PROJ_ROOT)/src/mobilecoupons
SRC_DIRS += $(PROJ_ROOT)/src/publictransitmanager
SRC_DIRS += $(PROJ_ROOT)/src/singlesearch

# list the source files that should be excluded from the build
EXCLUDE_SRCS := $(PROJ_ROOT)/src/qalog/qalog.c
EXCLUDE_SRCS += $(PROJ_ROOT)/src/qalog/nimqalog.c
EXCLUDE_SRCS += $(PROJ_ROOT)/src/main/main.cpp

ifeq ($(MK_BREW_VERSION),213)
MK_BREW_SDK_INC := $(BREW213_SDK_INC)
MK_BREW_INC := $(BREW213_INC)
endif

ifeq ($(MK_BREW_VERSION),312)
MK_BREW_SDK_INC := $(BREW312_SDK_INC)
MK_BREW_INC := $(BREW312_INC)
endif

ifeq ($(MK_BREW_VERSION),314)
MK_BREW_SDK_INC := $(BREW314_SDK_INC)
MK_BREW_INC := $(BREW314_INC)
endif

ifeq ($(MK_BREW_VERSION),315)
MK_BREW_SDK_INC := $(BREW315_SDK_INC)
MK_BREW_INC := $(BREW315_INC)
endif

ifeq ($(MK_BREW_VERSION),)
MK_BREW_SDK_INC := $(BREW_SDK_INC)
MK_BREW_INC := $(BREW_INC)
endif

BREW_INCLUDE_DIRS := $(MK_BREW_SDK_INC)
BREW_INCLUDE_DIRS += $(MK_BREW_INC)
BREW_INCLUDE_DIRS += $(THIRDPARTY_ROOT)/arm/windows

PLATFORM_INCLUDE_DIRS := $(BREW_INCLUDE_DIRS)
PLATFORM_INCLUDE_DIRS += $(PROJ_ROOT)/resources/brew

NIMNBIPAL_PATH    := $(NIMNBIPAL_INC) #supplied by user set enviromnment variable
NIMNBIPAL_DIR := $(subst \,/,$(strip $(NIMNBIPAL_PATH)))
NIMNBIPAL_DIR += $(NIMNBIPAL_DIR)/$(PLATFORM)

NIMABPAL_PATH    := $(NIMABPAL_INC) #supplied by user set enviromnment variable
NIMABPAL_DIR := $(subst \,/,$(strip $(NIMABPAL_PATH)))
NIMABPAL_DIR += $(NIMABPAL_DIR)/$(PLATFORM)

NIMCORE_PATH    := $(NIMCORE_INC) #supplied by user set enviromnment variable
NIMCORE_DIR := $(subst \,/,$(strip $(NIMCORE_PATH)))
NIMCORE_PRIVATE_DIR := $(NIMCORE_DIR)/private
NIMCORE_PROTECTED_DIR := $(NIMCORE_DIR)/protected
NIMCORE_BREW_DIR := $(NIMCORE_DIR)/$(PLATFORM)

INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/brew
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/protected
INCLUDE_DIRS += $(PLATFORM_INCLUDE_DIRS)
INCLUDE_DIRS += $(NIMNBIPAL_DIR)
INCLUDE_DIRS += $(NIMABPAL_DIR)
INCLUDE_DIRS += $(NIMCORE_DIR)
INCLUDE_DIRS += $(NIMCORE_PRIVATE_DIR)
INCLUDE_DIRS += $(NIMCORE_PROTECTED_DIR)
INCLUDE_DIRS += $(NIMCORE_BREW_DIR)

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=

# extra directories to invoke make before libary or application is built.
DEPENDS_DIR :=

# extra objects to link into the library or application
EXTRA_OBJS :=

# extra directory to invoke make AFTER library or application is built
#EXTRA_TARGETS_DIR := $(PROJ_ROOT)/src/unittests/make
EXTRA_TARGETS_DIR :=

# extra targets to make AFTER library or application is built
# define these targets in this makefile
EXTRA_TARGETS :=

# CFLAGS is passed to compiler

CPU      := --cpu 5TE 
APCS     := --apcs /ropi/interwork
OPT      := -Otime 
CODE	 := --littleend  --split_sections 
WARN	 := -W 
ROPILINK := --ropi 
CLSID := -DAPP_CLSID=$(GEN_APP_CLS_ID)
UNUSED := -DUNUSED_NDEFINED
NDEBUG := -DNDEBUG #temporarily adding

PLATFORM_COMPILE_FLAGS := $(CPU) $(OPT) $(CODE) $(DYNAPP) $(WARN) $(APCS) $(UNUSED) $(NDEBUG)

CFLAGS += $(PLATFORM_COMPILE_FLAGS)

# LDFLAGS is passed to linker
#LDFLAGS := 

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

ARCH		?= $(shell uname -m)
CROSS_COMPILE	?=

PYTHON ?= $(PYTHONPATH)
ifeq ($(PYTHON),)
$(error PYTHONPATH environment variable not set, please set.)
else
PYTHONDIR := $(subst \,/,$(strip $(PYTHON)))
endif

# Make variables (CC, etc...)
TCC		= $(CC_PATH)tcc
ACC		= $(CC_PATH)armcc
ACPP		= $(CC_PATH)armcpp
ARMLINK		= $(CC_PATH)armlink
LD		= $(CC_PATH)ld
CC		= $(CC_PATH)gcc
AWK		= awk
PYTHON		= $(PYTHONDIR)/python
RM = del /F /S /Q

CC := $(TCC)
LD := $(ARMLINK)

export ARCH CROSS_COMPILE
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK TCC ACC ARMLINK PYTHON ACPP

