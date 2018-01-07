
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

{$(SOL_DIR)\src\brew\util}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\util\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\brew\audio}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\audio\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\brew\gps}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\brew\gps\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

AEEAppGen.$(OBJ) : $(SUPPORT_DIR)\AEEAppGen.c
AEEAppGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEAppGen.h

AEEModGen.$(OBJ) : $(SUPPORT_DIR)\AEEModGen.c
AEEModGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEModGen.h

#===============================================================================
#                           
#===============================================================================

SUPPORT_OBJS =	AEEAppGen.$(OBJ) AEEModGen.$(OBJ)

UTIL = vector.$(OBJ) brewutil.$(OBJ) heapsort.$(OBJ) dynbuf.$(OBJ)

PAL_GPS = palgps.$(OBJ) gpscontext.$(OBJ) gps_iposdet.$(OBJ) gps_ilpsposdet.$(OBJ) gps_iemuposdet.$(OBJ)
PAL_AUDIO= palaudio.$(OBJ) qcp_combine.$(OBJ) qcp_player.$(OBJ)

NIMABPAL = $(UTIL) $(PAL_GPS) $(PAL_AUDIO)
