
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


{$(SOL_DIR)\src\nbcommon}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\nbcommon\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------
        
{$(SOL_DIR)\src\data}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\data\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\geocode}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\geocode\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\map}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\map\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\nav}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\nav\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\network}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\network\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\proxpoi}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\proxpoi\$(SRC_FILE)
        @cmd /c echo ---------------------------------------------------------------

{$(SOL_DIR)\src\spatial}.c.$(OBJ):
        @cmd /c echo ---------------------------------------------------------------
        @cmd /c echo OBJECT $(@F)
        $(TCC) $(CFLAGS) $(OBJ_FILE) $(SOL_DIR)\src\spatial\$(SRC_FILE)
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

NB_COMMON = nimnbformat.$(OBJ) nimnbstate.$(OBJ) nimnbutil.$(OBJ)
NB_DATA = data_address.$(OBJ) data_area.$(OBJ) data_blob.$(OBJ) data_cache_contents.$(OBJ) data_cache_item.$(OBJ) data_category.$(OBJ) \
	  data_country_info.$(OBJ) data_cross_street.$(OBJ) data_detour_avoid.$(OBJ) data_email.$(OBJ) data_event.$(OBJ) data_event_content.$(OBJ) \
	  data_event_performance.$(OBJ) data_file.$(OBJ) data_format_element.$(OBJ) data_formatted_text.$(OBJ) data_fuel_pricesummary.$(OBJ) \
          data_fuelproduct.$(OBJ) data_fueltype.$(OBJ) data_geocode_query.$(OBJ) data_geocode_reply.$(OBJ) data_gps.$(OBJ) \
          data_icon.$(OBJ) data_image.$(OBJ) data_iter_command.$(OBJ) data_iter_result.$(OBJ) data_line.$(OBJ) \
          data_location.$(OBJ) data_locmatch.$(OBJ) data_map_cache_contents.$(OBJ) data_map_descriptor.$(OBJ) data_map_query.$(OBJ) \
          data_map_reply.$(OBJ) data_map_source.$(OBJ) data_map_style.$(OBJ) data_nav_maneuver.$(OBJ) data_nav_progress.$(OBJ) \
          data_nav_query.$(OBJ) data_nav_reply.$(OBJ) data_orgdest.$(OBJ) data_pair.$(OBJ) data_phone.$(OBJ) \
          data_place.$(OBJ) data_point.$(OBJ) data_polyline.$(OBJ) data_polyline_pack.$(OBJ) data_position.$(OBJ) \
          data_price.$(OBJ) data_profile_query.$(OBJ) data_profile_reply.$(OBJ) data_proxmatch.$(OBJ) data_proxmatch_ambiguous.$(OBJ) \
          data_proxmatch_content.$(OBJ) data_proxmatch_summary.$(OBJ) data_proxpoi_query.$(OBJ) data_proxpoi_reply.$(OBJ) data_rating.$(OBJ) \
          data_recalc.$(OBJ) data_result_style.$(OBJ) data_retrieve_confirmed.$(OBJ) data_reverse_geocode_query.$(OBJ) data_reverse_geocode_reply.$(OBJ) \
          data_road.$(OBJ) data_roadinfo.$(OBJ) data_route_map.$(OBJ) data_route_number_info.$(OBJ) data_route_polyline.$(OBJ) \
          data_route_style.$(OBJ) data_search_filter.$(OBJ) data_sliceres.$(OBJ) data_speed_camera.$(OBJ) data_string.$(OBJ) \
          data_traffic_flow.$(OBJ) data_traffic_flow_item.$(OBJ) data_traffic_incident.$(OBJ) data_traffic_incidents.$(OBJ) data_traffic_map_reply.$(OBJ) \
          data_traffic_map_road.$(OBJ) data_traffic_notify_query.$(OBJ) data_traffic_notify_reply.$(OBJ) data_traffic_record_identifier.$(OBJ) data_traffic_region.$(OBJ) \
          data_turn_map.$(OBJ) data_upgrade_reply.$(OBJ) data_url.$(OBJ) data_vector_map.$(OBJ) data_vector_tile.$(OBJ) \
          data_vector_tile_corridor.$(OBJ) data_vector_tile_descriptor.$(OBJ) data_vector_tile_query.$(OBJ) data_vector_tile_reply.$(OBJ) datautil.$(OBJ) \
          data_weather_conditions.$(OBJ) data_weather_forecast.$(OBJ)
          
NB_GEOCODE = nimgeocode.$(OBJ)

NB_MAP = nimmap.$(OBJ)

NB_NAV = instructset.$(OBJ) nav_avg_speed.$(OBJ) navaudio.$(OBJ) navcore.$(OBJ) navutil.$(OBJ) nimdirections.$(OBJ) nimnav.$(OBJ) vector_tile_manager.$(OBJ)

NB_NETWORK = nimnbnetwork.$(OBJ)

NB_PROXPOI = nimproxpoi.$(OBJ)

NB_MAIN = sputil.$(OBJ) vector.$(OBJ) cache.$(OBJ) format.$(OBJ) idenutil.$(OBJ)

NB = $(NB_COMMON) $(NB_DATA) $(NB_GEOCODE) $(NB_MAP) $(NB_NAV) $(NB_NETWORK) $(NB_PROXPOI) $(NB_MAIN)