
#============================================================================
#                           DEFAULT SUFFIX RULES
#============================================================================

OBJ_FILE = $(OBJ_CMD) $(@F)				# Output object file specification

.SUFFIXES : .$(OBJ) .dep .c

#--------------------------------------------------------------------------
# C code inference rules
#----------------------------------------------------------------------------

.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SUPPORT_DIR)}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(CC) $(CFLAGS) $(OBJ_FILE) $(SUPPORT_DIR)\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\include}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\include\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\pal}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\pal\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\clock}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\clock\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\timer}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\timer\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\util}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\util\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\file}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\file\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\brew\net}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\net\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

AEEAppGen.$(OBJ) : $(SUPPORT_DIR)\AEEAppGen.c
AEEAppGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEAppGen.h

AEEModGen.$(OBJ) : $(SUPPORT_DIR)\AEEModGen.c
AEEModGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEModGen.h

#===============================================================================
#                           
#===============================================================================

SUPPORT_OBJS =	AEEAppGen.$(OBJ) AEEModGen.$(OBJ)

UTIL = vector.$(OBJ)

PAL_FILE = palfile.$(OBJ) fileutil.$(OBJ)
PAL_NET = palnet.$(OBJ) netutil.$(OBJ)
PAL_TIMER = paltimer.$(OBJ) timerimpl.$(OBJ)
PAL_CLOCK = palclock.$(OBJ)
PAL = palimpl.$(OBJ)

NIMNBIPAL = $(UTIL) $(PAL) $(PAL_CLOCK) $(PAL_TIMER) $(PAL_FILE) $(PAL_NET)
