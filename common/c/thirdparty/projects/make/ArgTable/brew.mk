SHELL:=cmd.exe /C
export SHELL

# define APP_NAME to build executible
APP_NAME :=

PROJ_ROOT := ../../..
CC_PATH ?= $(PROJ_ROOT)/arm/brew/bin/
OBJ_ROOT := $(PROJ_ROOT)/output/$(PLATFORM)/$(PROJ_NAME)


OBJ_DIR := $(subst /,\,$(OBJ_ROOT))
CHECK_OBJ_ROOT := $(shell mkdir $(OBJ_DIR))

# define STATIC_LIB_NAME to build static library (archive, .a)
PARTIAL_LIB_NAME := $(PROJ_NAME).o

CPU      := --cpu 5TE 
APCS     := --apcs /ropi/interwork
OPT      := -Otime 
CODE	 := --littleend  --split_sections 
WARN	 := -W 
ROPILINK := --ropi
CLSID    := -DAPP_CLSID=$(GEN_APP_CLS_ID)

PLATFORM_COMPILE_FLAGS := $(CPU) $(OPT) $(CODE) $(DYNAPP) $(WARN) $(APCS)

ARCH		?= $(shell uname -m)
CROSS_COMPILE	?=

# Make variables (CC, etc...)
TCC		= $(CC_PATH)tcc
ACC		= $(CC_PATH)armcc
ARMLINK		= $(CC_PATH)armlink
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
AWK		= awk
RM = del /F /S /Q


CC := $(TCC)
LD := $(ARMLINK)

export ARCH CROSS_COMPILE
export TCC ACC ARMLINK LD CC AWK 

ARM_INCLUDE_DIRS += $(PROJ_ROOT)/arm/windows

PLATFORM_INCLUDE_DIRS += $(ARM_INCLUDE_DIRS)

INCLUDE_DIRS := $(ARM_INCLUDE_DIRS)
CFLAGS := $(PLATFORM_COMPILE_FLAGS)
