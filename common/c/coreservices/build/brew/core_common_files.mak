
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
        
#--------------------------------------------------------------------------
# SRC DIR
#----------------------------------------------------------------------------


{$(SOL_DIR)\src\gps}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\gps\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\network}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\network\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\tps}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\tps\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\util}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\util\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

AEEAppGen.$(OBJ) : $(SUPPORT_DIR)\AEEAppGen.c
AEEAppGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEAppGen.h

AEEModGen.$(OBJ) : $(SUPPORT_DIR)\AEEModGen.c
AEEModGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEModGen.h

#===============================================================================
#                           
#===============================================================================

SUPPORT_OBJS =	AEEAppGen.$(OBJ) AEEModGen.$(OBJ)

CORE_GPS = gpscriteriaset.$(OBJ) gpshist.$(OBJ) gpsutil.$(OBJ)
CORE_NET = ipexp.$(OBJ)
CORE_NETWORK = nimnetwork.$(OBJ)
CORE_TPS = nimnet.$(OBJ) tpsdebug.$(OBJ) tpseltgen.$(OBJ) tpseltmut.$(OBJ) tpslib_preload.$(OBJ) tpspack.$(OBJ) tpsunpack.$(OBJ)
CORE_UTIL = bq.$(OBJ) csdict.$(OBJ) dynbuf.$(OBJ) fsalloc.$(OBJ) heapsort.$(OBJ) ht.$(OBJ) inflate.$(OBJ) intpack.$(OBJ) nimerrorutil.$(OBJ) sha1.$(OBJ) util.$(OBJ) vec.$(OBJ) zinflate.$(OBJ)
#MAIN = vector.$(OBJ) brewutil.$(OBJ) heapsort.$(OBJ) dynbuf.$(OBJ)

#CORE = $(CORE_GPS) $(CORE_NETWORK) $(CORE_TPS) $(CORE_UTIL) $(MAIN)

CORE = $(CORE_GPS) $(CORE_NETWORK) $(CORE_TPS) $(CORE_UTIL)