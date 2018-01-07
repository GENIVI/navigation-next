SHELL = cmd.exe /C

M2C := echo #We dont want to use this app anyway which is for FORTAN

# define APP_NAME to build executible
BREW_APP_NAME := unittests

PROJ_ROOT := ../../..
CORETECH_PATH := $(CORETECH_ROOT) #supplied by user set enviromnment variable
CORETECH_DIR = $(subst \,/,$(strip $(CORETECH_PATH)))
THIRDPARTY_ROOT  := $(CORETECH_DIR)/thirdparty
CC_PATH ?= $(THIRDPARTY_ROOT)/arm/brew/bin/
MK_BREW_VERSION ?= 315

OBJ_ROOT := $(PROJ_ROOT)/output/$(PLATFORM)/brew$(MK_BREW_VERSION)/unittests
OBJ_DIR := $(subst /,\,$(OBJ_ROOT))
CHECK_OBJ_ROOT := $(shell mkdir $(OBJ_DIR))

UNITTESTS_ROOT := $(PROJ_ROOT)/src/unittests
NBIPAL_LIB_ROOT ?= $(PROJ_ROOT)/projects/make/$(PLATFORM)

# where source files are located, all *.cpp and *.c files under these
# directorires will be included in the build
SRC_DIRS := ../brew
SRC_DIRS += ..

# list the source files that should be excluded from the build
EXCLUDE_SRCS := ../main.c

# where header files are located.
INCLUDE_DIRS := $(UNITTESTS_ROOT)
INCLUDE_DIRS += $(THIRDPARTY_ROOT)/CUnit/include

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
BREW_INCLUDE_DIRS += $(PROJ_ROOT)/resources/brew
BREW_INCLUDE_DIRS += $(THIRDPARTY_ROOT)/arm/windows

PLATFORM_INCLUDE_DIRS := $(BREW_INCLUDE_DIRS)
PLATFORM_INCLUDE_DIRS += $(PROJ_ROOT)/include
PLATFORM_INCLUDE_DIRS += $(PROJ_ROOT)/include/brew

NIMNBIPAL_PATH    := $(NIMNBIPAL_INC) #supplied by user set enviromnment variable
NIMNBIPAL_DIR := $(subst \,/,$(strip $(NIMNBIPAL_PATH)))
NIMNBIPAL_DIR += $(NIMNBIPAL_DIR)/$(PLATFORM)

NIMABPAL_PATH    := $(NIMABPAL_INC) #supplied by user set enviromnment variable
NIMABPAL_DIR := $(subst \,/,$(strip $(NIMABPAL_PATH)))
NIMABPAL_DIR += $(NIMABPAL_DIR)/$(PLATFORM)

INCLUDE_DIRS += $(PLATFORM_INCLUDE_DIRS)
INCLUDE_DIRS += $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/brew
INCLUDE_DIRS += $(PROJ_ROOT)/include/protected
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/src/util
INCLUDE_DIRS += $(NIMNBIPAL_DIR)
INCLUDE_DIRS += $(NIMABPAL_DIR)

# extra tragets to make before library or application are built.
# define these targets in this makefile
DEPENDS_TARGETS :=

# extra directories to invoke make before libary or application is built.
DEPENDS_DIR := $(THIRDPARTY_ROOT)/projects/make/CUnit

# extra objects to link into the library or application
EXTRA_OBJS := $(THIRDPARTY_ROOT)/output/brew/CUnit/cunit.o
EXTRA_OBJS += $(PROJ_ROOT)/output/$(PLATFORM)/brew$(MK_BREW_VERSION)/coreservices.o
EXTRA_OBJS += $(CORETECH_DIR)/nbpal/main/output/$(PLATFORM)/brew$(MK_BREW_VERSION)/nbpal.o

# extra directory to invoke make AFTER library or application is built
EXTRA_TARGETS_DIR :=

EXTRA_TARGETS :=

DEBUG ?= 0
export DEBUG

# CFLAGS is passed to compiler
ifneq ($(DEBUG),0)
    CFLAGS += -g
else
    CFLAGS += -O2
endif

CPU      := --cpu 5TE 
ROPI     := ropi 
INTERWRK := interwork 
APCS     := --apcs /$(ROPI)/$(INTERWRK)
OPT      := -Otime 
END 	 := --littleend 
ZA  	 := --split_sections 
CODE	 := $(END) $(ZA)
WARN	 := -W 
ROPILINK := --ropi 
LINK_OR  := --first AEEMod_Load
BIN      := --bin --output
LFLAGS 	 := --entry 0x8000
DYNAPP   := -DDYNAMIC_APP

PLATFORM_COMPILE_FLAGS := $(CPU) $(CODE) $(DYNAPP) $(WARN) -DAPP_CLSID=$(GEN_APP_CLS_ID)

CFLAGS += $(PLATFORM_COMPILE_FLAGS)

LDFLAGS :=
LFLAGS = --entry 0x8000#
LINK_ORDER = --first AEEMod_Load

# extra files or directories to be moved by 'make clean'
CLEAN_FILES :=

# extra files or directories to be moved by 'make distclean'
DISTCLEAN_FILES :=

ARCH		?= $(shell uname -m)
CROSS_COMPILE	?=

# Make variables (CC, etc...)
TCC		= $(CC_PATH)tcc
ACC		= $(CC_PATH)armcc
ARMLINK		= $(CC_PATH)armlink
HEXTOOL		= $(CC_PATH)fromelf
AWK		= awk
RM = del /F /S /Q

export ARCH CROSS_COMPILE
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE AWK TCC ACC ARMLINK HEXTOOL

CC := $(TCC)
LD := $(ARMLINK)

#set default target to all
.PHONY: default_target
default_target: all

$(OBJ_ROOT)/AEEAppGen.o : ../brew/AEEAppGen.c
	@echo Compiling $(notdir $<)	
	$Q$(ACC) $(CFLAGS) -c -o $@ $<
$(OBJ_ROOT)/AEEModGen.o : ../brew/AEEModGen.c
	@echo Compiling $(notdir $<)
	$Q$(ACC) $(CFLAGS) -c -o $@ $<
