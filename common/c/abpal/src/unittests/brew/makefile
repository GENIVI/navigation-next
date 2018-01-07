#============================================================================
#  Name:
#    $(TARGET).MAK
#
#  Description:
#    Makefile to build the $(TARGET) downloadable module.
#
#   The following nmake targets are available in this makefile:
#
#     all           - make .elf and .mod image files (default)
#     clean         - delete object directory and image files
#     filename.o    - make object file
#
#   The above targets can be made with the following command:
#
#     nmake /f $(TARGET).mak [target]
#
#  Assumptions:
#    1. The environment variable ADSHOME is set to the root directory of the
#       arm tools.
#    2. The version of ADS is 1.2 or above.
#
#  Notes:
#    None.
#
#
#        Copyright c 2000-2001 QUALCOMM Incorporated.
#               All Rights Reserved.
#            QUALCOMM Proprietary/GTDR
#
#----------------------------------------------------------------------------
#============================================================================

#----------------------------------------------------------------------------
# Tools and SDK directories
#----------------------------------------------------------------------------
BREW_HOME    = $(BREWDIR)


#----------------------------------------------------------------------------
# BREW Module-specific settings
#----------------------------------------------------------------------------

TARGET         = abpal_gpstest

OBJS           = aeeappgen.o \
                 aeemodgen.o \
                 aeemediautil.o \
                 abpal_gpstest.o

OBJS_DIR       = .\armobjs

APP_INCLUDES = -I$(NIMCORE_SDK_BREW_INC) \
               -I$(NIMCORE_SDK_BREW_INC)\brew

ARM_LIBDIRS = "$(RVCT30LIB)" $(NIMCORE_SDK_BREW_BIN)\brew315\abpal.o $(NIMCORE_SDK_BREW_BIN)\brew315\nbpal.o

#-------------------------------------------------------------------------------
# Target file name and type definitions
#-------------------------------------------------------------------------------

EXETYPE    = elf                # Target image file format
MODULE     = mod                # Downloadable module extension

#-------------------------------------------------------------------------------
# Target compile time symbol definitions
#
#-------------------------------------------------------------------------------

# Tells the SDK source stuffs that we're building a dynamic app.
DYNAPP          = -DDYNAMIC_APP

#-------------------------------------------------------------------------------
# Software tool and environment definitions
#-------------------------------------------------------------------------------

AEEINCPATH = $(BREW_HOME)\inc

ARMBIN   = "$(RVCT30BIN)"
ARMCC    = $(ARMBIN)\armcc      # ARM ADS ARM 32-bit inst. set ANSI CPP compiler
ARMCPP   = $(ARMBIN)\armcpp     # ARM ADS ARM 32-bit inst. set ANSI CPP compiler
LD       = $(ARMBIN)\armlink    # ARM ADS linker
HEXTOOL  = $(ARMBIN)\fromelf    # ARM utility to create hex file from image. 
                                # This one allows position-dependent code required for C++

OBJ_CMD    = -o                 # Command line option to specify output filename

#-------------------------------------------------------------------------------
# Processor architecture options
#-------------------------------------------------------------------------------

#CPU = --cpu ARM7TDMI             # ARM7TDMI target processor
CPU  = --cpu 5TE                  # ARM926EJ-S target processor

#-------------------------------------------------------------------------------
# ARM Procedure Call Standard (APCS) options
#-------------------------------------------------------------------------------

APCS = --apcs=/adsabi

#-------------------------------------------------------------------------------
# Compiler output options
#-------------------------------------------------------------------------------

OUT = -c                        # Object file output only

#-------------------------------------------------------------------------------
# Compiler/assembler debug options
#-------------------------------------------------------------------------------

DBG = $(DBG_CFG)                # Debug option enable/disable. See loccfg.mak

#-------------------------------------------------------------------------------
# Compiler optimization options
#-------------------------------------------------------------------------------

OPT = -Ospace -O2               # Full compiler optimization for space

#-------------------------------------------------------------------------------
# Compiler code generation options
#-------------------------------------------------------------------------------

END = --littleend                # Compile for little endian memory architecture

# Generates one ELF section for each function in the source file
SPLIT = --split_sections

# Disable these ARM warnings.
# Warning 1-D: last line of file ends without a new line. Present in many CCC headers.
DISABLED_WARNINGS = --diag_suppress 1

# No C++ exceptions allowed (we can't pick up ARM RTL stuff into BREW DLL, it won't link)
NO_EXCEPTIONS = --no_exceptions --no_exceptions_unwind

CODE = $(END) $(SPLIT) $(DISABLED_WARNINGS)

#-------------------------------------------------------------------------------
# Include file search path options
#-------------------------------------------------------------------------------

INC = -I. -I$(AEEINCPATH) $(APP_INCLUDES)

#-------------------------------------------------------------------------------
# Linker options
#-------------------------------------------------------------------------------

LINK_CMD = -o                    #Command line option to specify output file
                                 #on linking
LINK_ORDER = --first AEEMod_Load

#-------------------------------------------------------------------------------
# HEXTOOL options
#-------------------------------------------------------------------------------
OUTPUT = --output

#-------------------------------------------------------------------------------
# Compiler flag definitions
#-------------------------------------------------------------------------------

CFLAGS0 = $(OUT) $(DYNAPP) $(NBI_BREW) $(CPU) $(APCS) $(CODE) $(DBG) $(PREDEFS)
CFLAGS  = $(CFLAGS0) $(OPT)
CPPFLAGS = $(CFLAGS) $(NO_EXCEPTIONS)
#-------------------------------------------------------------------------------
# Linker flag definitions
#-------------------------------------------------------------------------------

# DLL entry point
ENTRY = --entry AEEMod_Load

# Turn off RW data compression
NO_DATA_COMPRESSION = --datacompressor=off

# Report conditions, that might result in failure, as errors
STRICT = --strict

# Makes relocatable ELF images
RELOCATABLE = --reloc

# Split RO and RW/ZI sections
LSPLIT = --split

# No C++ exceptions allowed (we can't pick up ARM RTL stuff into BREW DLL, it won't link)
LNO_EXCEPTIONS = --no_exceptions

# Specifying the load region. Our image conforms to "Simple Image Type 1" from RVCT Linker Guide.
# It says this image is suitable for systems that load programs into RAM.
RO_BASE = --ro_base 0

# VFE is Virtual Function Elimination. is a technique that enables the linker
# to identify more unused sections.
# This flag Forces the linker to ignore any extra information supplied by the compiler.
VFE_OFF = --vfemode=off

LFLAGS = $(STRICT) $(ENTRY) $(NO_DATA_COMPRESSION) $(RELOCATABLE) $(VFE_OFF) $(RO_BASE) $(LSPLIT) $(LNO_EXCEPTIONS)#
USRLPATH = --userlibpath $(OBJS_DIR)
LPATH = --libpath $(ARM_LIBDIRS)

#----------------------------------------------------------------------------
# Default target
#----------------------------------------------------------------------------

all : $(TARGET).$(MODULE)

#----------------------------------------------------------------------------
# Clean target
#----------------------------------------------------------------------------

# The object subdirectory, target image file, and target hex file are deleted.

clean :
	@echo ---------------------------------------------------------------
	@echo CLEAN
	-del /f $(TARGET).$(MODULE)
	cd $(OBJS_DIR)
	-del /f $(OBJS)
	@echo ---------------------------------------------------------------

#============================================================================
#                           DEFAULT SUFFIX RULES
#============================================================================

# The following are the default suffix rules used to compile all objects that
# are not specifically included in one of the module specific rules defined
# in the next section.

# The following macros are used to specify the output object file, MSG_FILE
# symbol definition and input source file on the compile line in the rules
# defined below.

SRC_CPP_FILE = $(@F:.o=.cpp)      # Input source file specification
SRC_C_FILE   = $(@F:.o=.c)      # Input source file specification
OBJ_FILE = $(OBJ_CMD) $(OBJS_DIR)\$(@F)   # Output object file specification

.SUFFIXES :
.SUFFIXES : .o .dep .c .cpp


#--------------------------------------------------------------------------
# C code inference rules
#----------------------------------------------------------------------------
#.cpp.o:
#    @echo ---------------------------------------------------------------
#    @echo OBJECT $(@F)
#    $(ARMCPP) $(CPPFLAGS) $(INC) $(OBJ_FILE) $(SRC_CPP_FILE)
#    @echo ---------------------------------------------------------------

.c.o:
    @echo ---------------------------------------------------------------
    @echo $(SRC_C_FILE)
    @echo OBJECT $(@F)
    $(ARMCC) $(CFLAGS) $(INC) $(OBJ_FILE) $(SRC_C_FILE)
    @echo ---------------------------------------------------------------

#===============================================================================
#                           MODULE SPECIFIC RULES
#===============================================================================

APP_OBJS =  $(OBJS)

#----------------------------------------------------------------------------
# Lib file targets
#----------------------------------------------------------------------------

$(TARGET).$(MODULE) : $(TARGET).$(EXETYPE)
    @echo ---------------------------------------------------------------
    @echo TARGET $@
    $(HEXTOOL)  $(TARGET).$(EXETYPE) $(OUTPUT) $(TARGET).$(MODULE)
    -del /f $(TARGET).$(EXETYPE)

$(TARGET).$(EXETYPE) : $(APP_OBJS)
    @echo ---------------------------------------------------------------
    @echo TARGET $@
    $(LD) $(LINK_CMD) $(TARGET).$(EXETYPE) $(LINK_ORDER) $(LFLAGS) $(LPATH) $(USRLPATH) $(APP_OBJS)

