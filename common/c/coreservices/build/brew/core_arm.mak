#-------------------------------------------------------------------------------
# Target file name and type definitions
#-------------------------------------------------------------------------------

!INCLUDE core_common.mak

OBJ             = o        # Object file suffix

#-------------------------------------------------------------------------------
# Target compile time symbol definitions
#-------------------------------------------------------------------------------

DYNAPP          = -DDYNAMIC_APP

#-------------------------------------------------------------------------------
# Software tool and environment definitions
#-------------------------------------------------------------------------------

TCC		= tcc
CC		= armcc       # ARM ADS ARM 32-bit inst. set ANSI C compiler
LD              = armlink     # ARM ADS linker
HEXTOOL         = fromelf     # ARM ADS utility to create hex file from image

OBJ_CMD         = -o          # Command line option to specify output filename

#-------------------------------------------------------------------------------
# Processor architecture options
#-------------------------------------------------------------------------------

CPU             = --cpu 5TE   # ARM926EJ-S target processor

#-------------------------------------------------------------------------------
# ARM Procedure Call Standard (APCS) options
#-------------------------------------------------------------------------------

ROPI     = ropi               # Read-Only(code) Position independence
INTERWRK = interwork          # Allow ARM-Thumb interworking

APCS = --apcs /$(ROPI)/$(INTERWRK)

#-------------------------------------------------------------------------------
# Additional compile time error checking options
#-------------------------------------------------------------------------------

#CHK = -fa                       # Check for data flow anomolies

#-------------------------------------------------------------------------------
# Compiler output options
#-------------------------------------------------------------------------------

OUT = -c                        # Object file output only

#-------------------------------------------------------------------------------
# Compiler optimization options
#-------------------------------------------------------------------------------

OPT = -Otime -O2                 # Full compiler optimization for time

#-------------------------------------------------------------------------------
# Compiler code generation options
#-------------------------------------------------------------------------------

END = --littleend                # Compile for little endian memory architecture
ZA  = --split_sections           # LDR may only access 32-bit aligned addresses

CODE = $(END) $(ZA)

#-------------------------------------------------------------------------------
# Compiler warning options
#-------------------------------------------------------------------------------

WARN = -W			  # Prevent the warning about -zas4 being deprecated

#-------------------------------------------------------------------------------
# Linker options
#-------------------------------------------------------------------------------

LINK_CMD = -o                     #Command line option to specify output file 
                                  #on linking

ROPILINK = --ropi                 #Link image as Read-Only Position Independent

LINK_ORDER = --first AEEMod_Load

#-------------------------------------------------------------------------------
# HEXTOOL options
#-------------------------------------------------------------------------------

BINFORMAT = --bin --output

#-------------------------------------------------------------------------------
# Compiler flag definitions
#-------------------------------------------------------------------------------

CFLAGS0 = $(OUT) $(DYNAPP) $(INC) $(CPU) $(APCS) $(CODE) $(CHK) $(DBG) $(WARN)
CFLAGS1  = $(OPT) $(QALOG) $(NETWORK) $(CACHE) $(DEBUGUI) $(GPS) $(GPSFILTER) $(SMS) $(NIMDEFINES)
CFLAGS = $(CFLAGS0) $(CFLAGS1) -DAPP_CLSID=$(GEN_APP_CLS_ID) -DUNUSED_NDEFINED

#-------------------------------------------------------------------------------
# Linker flag definitions
#-------------------------------------------------------------------------------

LFLAGS = --entry 0x8000#

#----------------------------------------------------------------------------
# Default target
#----------------------------------------------------------------------------

all : coreservices
coreservices :  coreservices.$(OBJ)

#----------------------------------------------------------------------------
# Suffix rules for files
#----------------------------------------------------------------------------

SRC_FILE = $(@F:.o=.c)                       # Input source file specification
!INCLUDE core_common_files.mak          # All files

#----------------------------------------------------------------------------
# Lib file targets
#----------------------------------------------------------------------------

coreservices.$(OBJ) : $(CORE)
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo TARGET $@
        $(LD) --partial $(LINK_CMD) coreservices.$(OBJ) $(CORE)
        
# ------------------------------
# DO NOT EDIT BELOW THIS LINE
AEEAppGen.o : $(SUPPORT_DIR)\AEEAppGen.c
AEEAppGen.o : $(SUPPORT_INCDIR)\AEEAppGen.h

AEEModGen.o : $(SUPPORT_DIR)\AEEModGen.c
AEEModGen.o : $(SUPPORT_INCDIR)\AEEModGen.h
