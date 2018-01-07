
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

{$(SOL_DIR)\src\data}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\data\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\main}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\main\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

        
AEEAppGen.$(OBJ) : $(SUPPORT_DIR)\AEEAppGen.c
AEEAppGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEAppGen.h

AEEModGen.$(OBJ) : $(SUPPORT_DIR)\AEEModGen.c
AEEModGen.$(OBJ) : $(SUPPORT_INCDIR)\AEEModGen.h

#===============================================================================
#                           
#===============================================================================

SUPPORT_OBJS =	AEEAppGen.$(OBJ) AEEModGen.$(OBJ)

AB_DATA = data_asr_datastore_query.$(OBJ) data_asr_datastore_reply.$(OBJ) data_asr_field_data.$(OBJ) data_asr_recentloc.$(OBJ) data_asr_reco_query.$(OBJ) \
	  data_asr_reco_reply.$(OBJ) data_asr_stats_query.$(OBJ) data_asr_stats_reply.$(OBJ) data_asr_timing.$(OBJ) data_asr_utterance.$(OBJ) \
	  data_message.$(OBJ) data_message_confirm.$(OBJ) data_message_confirm_query.$(OBJ) data_message_nag.$(OBJ) data_message_reply.$(OBJ) \
	  data_place_message.$(OBJ) data_place_msg_banner.$(OBJ) data_place_msg_error.$(OBJ) data_place_msg_id.$(OBJ) data_place_msg_recipient.$(OBJ) \
	  data_place_msg_recipient_result.$(OBJ) data_send_place_message_query.$(OBJ) data_send_place_message_reply.$(OBJ) data_sync_add_item.$(OBJ) data_sync_assign_item_id.$(OBJ) \
	  data_sync_delete_item.$(OBJ) data_sync_error.$(OBJ) data_sync_modify_item.$(OBJ) data_sync_places_db_query.$(OBJ) data_sync_places_db_reply.$(OBJ) \
	  data_sync_places_db_status_query.$(OBJ) data_sync_places_db_status_reply.$(OBJ) data_sync_places_query.$(OBJ) data_sync_places_reply.$(OBJ) data_sync_places_status_query.$(OBJ) \
	  data_sync_places_status_reply.$(OBJ)
	  
AB = $(AB_DATA)