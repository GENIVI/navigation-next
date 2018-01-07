LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := NBM

PROJ_ROOT := $(LOCAL_PATH)/../../../..
P4PATH :=    $(LOCAL_PATH)/../../../../../../../../..

LOCAL_SRC_FILES := $(P4PATH)/core/releases/nbm/android/nbm_1.0.0.16_android/libs/libNBM.a

include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)

LOCAL_MODULE := nbservices

# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../..
P4PATH :=    $(LOCAL_PATH)/../../../../../../../..
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/protected
INCLUDE_DIRS += $(PROJ_ROOT)/include/generated
INCLUDE_DIRS += $(PROJ_ROOT)/include/generated/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/private
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/protected
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/abpal/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/abpal/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbgm/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbgm/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbgm/$(BRANCH)/include/private
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbgm/$(BRANCH)/include/protected
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbui/$(BRANCH)/include/protected
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbservices/$(BRANCH)/src/protocol
INCLUDE_DIRS += $(P4PATH)/core/releases/nbm/android/nbm_1.0.0.16_android/include

LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))
LOCAL_CFLAGS += -DBOOST_EXCEPTION_DISABLE -DBOOST_NO_EXCEPTIONS -D_STLP_NO_EXCEPTIONS -DOS_ANDROID -D_STLP_USE_SIMPLE_NODE_ALLOC 

LOCAL_WHOLE_STATIC_LIBRARIES := NBM
LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

#LOCAL_LDLIBS += -lstdc++
#LOCAL_SHARED_LIBRARIES := stlport

#working
SRCS :=	../../../src/analytics/nbanalytics.c  \
	../../../src/analytics/nbanalyticseventsconfig.c  \
	../../../src/analytics/nbanalyticseventsdb.c  \
	../../../src/analytics/nbanalyticseventstailq.c  \
	../../../src/analytics/nbanalyticshandler.c  \
	../../../src/analytics/nbanalyticsinformation.c  \
	../../../src/analytics/nbanalyticsparameters.c  \
	../../../src/data/datautil.c  \
	../../../src/data/data_address.c  \
	../../../src/data/data_add_favorites_event.c  \
	../../../src/data/data_analytics_config.c  \
	../../../src/data/data_analytics_event.c  \
	../../../src/data/data_analytics_events_query.c  \
	../../../src/data/data_analytics_events_reply.c  \
	../../../src/data/data_analytics_event_destination.c  \
	../../../src/data/data_analytics_event_origin.c  \
	../../../src/data/data_analytics_event_place.c  \
	../../../src/data/data_app_discovery.c  \
	../../../src/data/data_app_errors_config.c  \
	../../../src/data/data_app_error_event.c  \
	../../../src/data/data_area.c  \
	../../../src/data/data_arrival_event.c  \
	../../../src/data/data_blob.c  \
	../../../src/data/data_box.c  \
	../../../src/data/data_cache_contents.c  \
	../../../src/data/data_cache_item.c  \
	../../../src/data/data_call_event.c  \
	../../../src/data/data_category.c  \
	../../../src/data/data_category_list_request.c  \
	../../../src/data/data_category_list_response.c  \
	../../../src/data/data_cdma.c  \
	../../../src/data/data_content_details.c  \
	../../../src/data/data_content_displayed.c  \
	../../../src/data/data_content_path.c  \
	../../../src/data/data_content_source.c  \
	../../../src/data/data_country_info.c  \
	../../../src/data/data_coupon_acquisition.c  \
	../../../src/data/data_coupon_analytics_action_event.c  \
	../../../src/data/data_coupon_availability.c  \
	../../../src/data/data_coupon_broad_category.c  \
	../../../src/data/data_coupon_category.c  \
	../../../src/data/data_coupon_data.c  \
	../../../src/data/data_coupon_error.c  \
	../../../src/data/data_coupon_events_config.c  \
	../../../src/data/data_coupon_query.c  \
	../../../src/data/data_coupon_reply.c  \
	../../../src/data/data_coupon_request.c  \
	../../../src/data/data_coupon_response.c  \
	../../../src/data/data_coupon_sub_category.c  \
	../../../src/data/data_cross_street.c  \
	../../../src/data/data_dataset.c  \
	../../../src/data/data_detour_avoid.c  \
	../../../src/data/data_email.c  \
	../../../src/data/data_error_category.c  \
	../../../src/data/data_event.c  \
	../../../src/data/data_event_categories.c  \
	../../../src/data/data_event_content.c  \
	../../../src/data/data_event_detail.c  \
	../../../src/data/data_event_performance.c  \
	../../../src/data/data_event_type.c  \
	../../../src/data/data_extended_content.c  \
	../../../src/data/data_extended_content_region.c  \
	../../../src/data/data_facebook_update_event.c  \
	../../../src/data/data_feedback_config.c  \
	../../../src/data/data_feedback_event.c  \
	../../../src/data/data_file.c  \
	../../../src/data/data_formatted_text.c  \
	../../../src/data/data_format_element.c  \
	../../../src/data/data_fuelproduct.c  \
	../../../src/data/data_fueltype.c  \
	../../../src/data/data_fuel_pricesummary.c  \
	../../../src/data/data_geocode_query.c  \
	../../../src/data/data_geocode_reply.c  \
	../../../src/data/data_geographic_position.c  \
	../../../src/data/data_golden_cookie.c  \
	../../../src/data/data_gold_events_config.c  \
	../../../src/data/data_gps.c  \
	../../../src/data/data_gps_offset.c  \
	../../../src/data/data_gps_probes_config.c  \
	../../../src/data/data_gps_probes_event.c  \
	../../../src/data/data_gsm.c  \
	../../../src/data/data_hours_of_operation.c  \
	../../../src/data/data_icon.c  \
	../../../src/data/data_image.c  \
	../../../src/data/data_image_urls.c  \
	../../../src/data/data_impression_event.c  \
	../../../src/data/data_issue_type.c  \
	../../../src/data/data_iter_command.c  \
	../../../src/data/data_iter_result.c  \
	../../../src/data/data_line.c  \
	../../../src/data/data_link.c  \
	../../../src/data/data_load_image_error.c  \
	../../../src/data/data_load_image_request.c  \
	../../../src/data/data_load_image_response.c  \
	../../../src/data/data_location.c  \
	../../../src/data/data_location_error.c  \
	../../../src/data/data_location_moved_point.c  \
	../../../src/data/data_location_position.c  \
	../../../src/data/data_location_query.c  \
	../../../src/data/data_location_reply.c  \
	../../../src/data/data_location_tile.c  \
	../../../src/data/data_locmatch.c  \
	../../../src/data/data_map_cache_contents.c  \
	../../../src/data/data_map_descriptor.c  \
	../../../src/data/data_map_event.c  \
	../../../src/data/data_map_query.c  \
	../../../src/data/data_map_reply.c  \
	../../../src/data/data_map_source.c  \
	../../../src/data/data_map_style.c  \
	../../../src/data/data_metadata.c  \
	../../../src/data/data_metadata_manifest_query.c  \
	../../../src/data/data_metadata_manifest_reply.c  \
	../../../src/data/data_metadata_source_query.c  \
	../../../src/data/data_metadata_source_reply.c  \
	../../../src/data/data_nav_invocation_config.c  \
	../../../src/data/data_nav_invocation_event.c  \
	../../../src/data/data_nav_maneuver.c  \
	../../../src/data/data_nav_progress.c  \
	../../../src/data/data_nav_query.c  \
	../../../src/data/data_nav_reply.c  \
	../../../src/data/data_operating_hours.c  \
	../../../src/data/data_orgdest.c  \
	../../../src/data/data_overall_rating.c  \
	../../../src/data/data_pair.c  \
	../../../src/data/data_phone.c  \
	../../../src/data/data_place.c  \
	../../../src/data/data_place_event_cookie.c  \
	../../../src/data/data_place_message_event.c  \
	../../../src/data/data_point.c  \
	../../../src/data/data_poi_content.c  \
	../../../src/data/data_poi_events_config.c  \
	../../../src/data/data_polyline.c  \
	../../../src/data/data_polyline_pack.c  \
	../../../src/data/data_position.c  \
	../../../src/data/data_price.c  \
	../../../src/data/data_projection_parameters.c  \
	../../../src/data/data_pronun_query.c  \
	../../../src/data/data_pronun_reply.c  \
	../../../src/data/data_proxmatch.c  \
	../../../src/data/data_proxmatch_ambiguous.c  \
	../../../src/data/data_proxmatch_content.c  \
	../../../src/data/data_proxmatch_summary.c  \
	../../../src/data/data_proxpoi_error.c  \
	../../../src/data/data_proxpoi_query.c  \
	../../../src/data/data_proxpoi_reply.c  \
	../../../src/data/data_proxy_api_key.c  \
	../../../src/data/data_raster_tile_data_source_query.c  \
	../../../src/data/data_raster_tile_data_source_reply.c  \
	../../../src/data/data_rating.c  \
	../../../src/data/data_realistic_sign.c  \
	../../../src/data/data_recalc.c  \
	../../../src/data/data_result_style.c  \
	../../../src/data/data_retrieve_confirmed.c  \
	../../../src/data/data_reverse_geocode_query.c  \
	../../../src/data/data_reverse_geocode_reply.c  \
	../../../src/data/data_road.c  \
	../../../src/data/data_roadinfo.c  \
	../../../src/data/data_route_corridor.c  \
	../../../src/data/data_route_map.c  \
	../../../src/data/data_route_number_info.c  \
	../../../src/data/data_route_polyline.c  \
	../../../src/data/data_route_reply_event.c  \
	../../../src/data/data_route_request_event.c  \
	../../../src/data/data_route_state_event.c  \
	../../../src/data/data_route_style.c  \
	../../../src/data/data_route_tracking_config.c  \
	../../../src/data/data_search_cookie.c  \
	../../../src/data/data_search_detail_event.c  \
	../../../src/data/data_search_event_cookie.c  \
	../../../src/data/data_search_filter.c  \
	../../../src/data/data_search_query.c  \
	../../../src/data/data_search_query_event.c  \
	../../../src/data/data_search_reply.c  \
	../../../src/data/data_sliceres.c  \
	../../../src/data/data_speed_camera.c  \
	../../../src/data/data_spelling_suggestion.c  \
	../../../src/data/data_store_coupon_data.c  \
	../../../src/data/data_store_data.c  \
	../../../src/data/data_store_request.c  \
	../../../src/data/data_store_response.c  \
	../../../src/data/data_string.c  \
	../../../src/data/data_suggest_match.c  \
	../../../src/data/data_tagline.c  \
	../../../src/data/data_text.c  \
	../../../src/data/data_traffic_flow.c  \
	../../../src/data/data_traffic_flow_item.c  \
	../../../src/data/data_traffic_incident.c  \
	../../../src/data/data_traffic_incidents.c  \
	../../../src/data/data_traffic_map_reply.c  \
	../../../src/data/data_traffic_map_road.c  \
	../../../src/data/data_traffic_notify_query.c  \
	../../../src/data/data_traffic_notify_reply.c  \
	../../../src/data/data_traffic_record_identifier.c  \
	../../../src/data/data_traffic_region.c  \
	../../../src/data/data_transit_poi.c  \
	../../../src/data/data_turn_map.c  \
	../../../src/data/data_url.c  \
	../../../src/data/data_url_args_template.c  \
	../../../src/data/data_use_navigator.c  \
	../../../src/data/data_vector_map.c  \
	../../../src/data/data_vector_tile.c  \
	../../../src/data/data_vector_tile_corridor.c  \
	../../../src/data/data_vector_tile_descriptor.c  \
	../../../src/data/data_vector_tile_query.c  \
	../../../src/data/data_vector_tile_reply.c  \
	../../../src/data/data_wanted_content.c  \
	../../../src/data/data_want_analytics_config.c  \
	../../../src/data/data_weather_conditions.c  \
	../../../src/data/data_weather_forecast.c  \
	../../../src/data/data_web_url_event.c  \
	../../../src/data/data_wifi.c  \
	../../../src/data/data_wifi_capabilities.c  \
	../../../src/data/data_wifi_probe.c  \
	../../../src/data/data_wifi_probes_config.c  \
	../../../src/data/data_wifi_probes_event.c  \
	../../../src/enhancedcontent/nbenhancedcontentdatasource.c  \
	../../../src/enhancedcontent/nbenhancedcontentdownloadmanager.c  \
	../../../src/enhancedcontent/nbenhancedcontentmanager.c  \
	../../../src/enhancedcontent/nbmanifesthandler.c  \
	../../../src/enhancedcontent/nbmanifestinformation.c  \
	../../../src/enhancedcontent/nbmanifestparameters.c  \
	../../../src/enhancedcontent/nbmetadatadatabaseoperator.c  \
	../../../src/enhancedcontent/nbmetadatahandler.c  \
	../../../src/enhancedcontent/nbmetadatainformation.c  \
	../../../src/enhancedcontent/nbmetadataparameters.c  \
	../../../src/geocode/nbgeocodehandler.c  \
	../../../src/geocode/nbgeocodeinformation.c  \
	../../../src/geocode/nbgeocodeparameters.c  \
	../../../src/geocode/nbreversegeocodehandler.c  \
	../../../src/geocode/nbreversegeocodeinformation.c  \
	../../../src/geocode/nbreversegeocodeparameters.c  \
	../../../src/gps/gpscriteriaset.c  \
	../../../src/gps/gpshist.c  \
	../../../src/gps/gpsutil.c  \
	../../../src/gps/nbgpsfilter.c  \
	../../../src/map/BinaryData.cpp \
	../../../src/map/CachingTileCallbackWithRequest.cpp \
	../../../src/map/CachingTileManager.cpp \
	../../../src/map/GenericTileCallback.cpp \
	../../../src/map/LamProcessor.cpp \
	../../../src/map/MapServicesConfiguration.cpp \
	../../../src/map/MapViewLayerCallback.cpp \
	../../../src/map/MapViewTileCallback.cpp \
	../../../src/map/NBGMViewController.cpp \
	../../../src/map/OffboardConvertTileManager.cpp \
	../../../src/map/OffboardConvertTileRequest.cpp \
	../../../src/map/OffboardLayerProvider.cpp \
	../../../src/map/OffboardTile.cpp \
	../../../src/map/OffboardTileManager.cpp \
	../../../src/map/OffboardTileRequest.cpp \
	../../../src/map/OffboardTileType.cpp \
	../../../src/map/OnboardLayerProvider.cpp \
	../../../src/map/RasterTileConverter.cpp \
	../../../src/map/TileCallbackWithRequest.cpp \
	../../../src/map/TileConverter.cpp \
	../../../src/map/UnifiedLayer.cpp \
	../../../src/map/UnifiedLayerManager.cpp \
	../../../src/nav/instructset.c  \
	../../../src/nav/nbguidanceinformation.c  \
	../../../src/nav/nbguidancemessage.c  \
	../../../src/nav/nbgpshistory.c  \
	../../../src/nav/navutil.c  \
	../../../src/nav/nav_avg_speed.c  \
	../../../src/nbcommon/nbcontext.cpp  \
	../../../src/nbcommon/workerqueuemanager.cpp  \
	../../../src/nbcommon/nbtaskqueue.cpp  \
	../../../src/nbcommon/httpdownloadmanager.cpp \
	../../../src/nbcommon/downloadrequest.cpp \
	../../../src/nbcommon/datastreamimplementation.cpp \
	../../../src/nbcommon/StringUtility.cpp  \
	../../../src/nbcommon/nbformat.c  \
	../../../src/nbcommon/nbpersistentdata.c  \
	../../../src/nbcommon/nbpointiteration.c  \
	../../../src/nbcommon/nbsolarcalculator.c  \
	../../../src/nbcommon/nbtristrip.c  \
	../../../src/nbcommon/nbutility.c  \
	../../../src/network/nbnetwork.c  \
	../../../src/qalog/nbqalog.c  \
	../../../src/qalog/nbqarecord.c  \
	../../../src/protocol/TpsAtlasbookProtocol.cpp \
	../../../src/spatial/nbspatial.c  \
	../../../src/spatial/nbvectortransform.c  \
	../../../src/spatial/spatialvector.c  \
	../../../src/spatial/transformutility.c  \
	../../../src/tileservice/nbtilemanager.c  \
	../../../src/util/format.c  \
	../../../src/util/nbversion.c  \
	../../../src/vectortile/nbvectortilehandler.c  \
	../../../src/vectortile/nbvectortileinformation.c  \
	../../../src/vectortile/nbvectortileinformationprivate.c  \
	../../../src/vectortile/nbvectortilemanager.c  \
	../../../src/vectortile/nbvectortileparameters.c  \
	../../../src/wifiprobes/nbwifiprobes.c  \
	
#full set	
#SRCS :=  ../../../src/analytics/nbanalytics.c  \
#	../../../src/analytics/nbanalyticseventsconfig.c  \
#	../../../src/analytics/nbanalyticseventsdb.c  \
#	../../../src/analytics/nbanalyticseventstailq.c  \
#	../../../src/analytics/nbanalyticshandler.c  \
#	../../../src/analytics/nbanalyticsinformation.c  \
#	../../../src/analytics/nbanalyticsparameters.c  \
#	../../../src/data/datautil.c  \
#	../../../src/data/data_address.c  \
#	../../../src/data/data_add_favorites_event.c  \
#	../../../src/data/data_analytics_config.c  \
#	../../../src/data/data_analytics_event.c  \
#	../../../src/data/data_analytics_events_query.c  \
#	../../../src/data/data_analytics_events_reply.c  \
#	../../../src/data/data_analytics_event_destination.c  \
#	../../../src/data/data_analytics_event_origin.c  \
#	../../../src/data/data_analytics_event_place.c  \
#	../../../src/data/data_app_discovery.c  \
#	../../../src/data/data_app_errors_config.c  \
#	../../../src/data/data_app_error_event.c  \
#	../../../src/data/data_area.c  \
#	../../../src/data/data_arrival_event.c  \
#	../../../src/data/data_blob.c  \
#	../../../src/data/data_box.c  \
#	../../../src/data/data_cache_contents.c  \
#	../../../src/data/data_cache_item.c  \
#	../../../src/data/data_call_event.c  \
#	../../../src/data/data_category.c  \
#	../../../src/data/data_category_list_request.c  \
#	../../../src/data/data_category_list_response.c  \
#	../../../src/data/data_cdma.c  \
#	../../../src/data/data_content_details.c  \
#	../../../src/data/data_content_displayed.c  \
#	../../../src/data/data_content_path.c  \
#	../../../src/data/data_content_source.c  \
#	../../../src/data/data_country_info.c  \
#	../../../src/data/data_coupon_acquisition.c  \
#	../../../src/data/data_coupon_analytics_action_event.c  \
#	../../../src/data/data_coupon_availability.c  \
#	../../../src/data/data_coupon_broad_category.c  \
#	../../../src/data/data_coupon_category.c  \
#	../../../src/data/data_coupon_data.c  \
#	../../../src/data/data_coupon_error.c  \
#	../../../src/data/data_coupon_events_config.c  \
#	../../../src/data/data_coupon_query.c  \
#	../../../src/data/data_coupon_reply.c  \
#	../../../src/data/data_coupon_request.c  \
#	../../../src/data/data_coupon_response.c  \
#	../../../src/data/data_coupon_sub_category.c  \
#	../../../src/data/data_cross_street.c  \
#	../../../src/data/data_dataset.c  \
#	../../../src/data/data_detour_avoid.c  \
#	../../../src/data/data_email.c  \
#	../../../src/data/data_error_category.c  \
#	../../../src/data/data_event.c  \
#	../../../src/data/data_event_categories.c  \
#	../../../src/data/data_event_content.c  \
#	../../../src/data/data_event_detail.c  \
#	../../../src/data/data_event_performance.c  \
#	../../../src/data/data_event_type.c  \
#	../../../src/data/data_extended_content.c  \
#	../../../src/data/data_extended_content_region.c  \
#	../../../src/data/data_facebook_update_event.c  \
#	../../../src/data/data_feedback_config.c  \
#	../../../src/data/data_feedback_event.c  \
#	../../../src/data/data_file.c  \
#	../../../src/data/data_formatted_text.c  \
#	../../../src/data/data_format_element.c  \
#	../../../src/data/data_fuelproduct.c  \
#	../../../src/data/data_fueltype.c  \
#	../../../src/data/data_fuel_pricesummary.c  \
#	../../../src/data/data_geocode_query.c  \
#	../../../src/data/data_geocode_reply.c  \
#	../../../src/data/data_geographic_position.c  \
#	../../../src/data/data_golden_cookie.c  \
#	../../../src/data/data_gold_events_config.c  \
#	../../../src/data/data_gps.c  \
#	../../../src/data/data_gps_offset.c  \
#	../../../src/data/data_gps_probes_config.c  \
#	../../../src/data/data_gps_probes_event.c  \
#	../../../src/data/data_gsm.c  \
#	../../../src/data/data_hours_of_operation.c  \
#	../../../src/data/data_icon.c  \
#	../../../src/data/data_image.c  \
#	../../../src/data/data_image_urls.c  \
#	../../../src/data/data_impression_event.c  \
#	../../../src/data/data_issue_type.c  \
#	../../../src/data/data_iter_command.c  \
#	../../../src/data/data_iter_result.c  \
#	../../../src/data/data_line.c  \
#	../../../src/data/data_link.c  \
#	../../../src/data/data_load_image_error.c  \
#	../../../src/data/data_load_image_request.c  \
#	../../../src/data/data_load_image_response.c  \
#	../../../src/data/data_location.c  \
#	../../../src/data/data_location_error.c  \
#	../../../src/data/data_location_moved_point.c  \
#	../../../src/data/data_location_position.c  \
#	../../../src/data/data_location_query.c  \
#	../../../src/data/data_location_reply.c  \
#	../../../src/data/data_location_tile.c  \
#	../../../src/data/data_locmatch.c  \
#	../../../src/data/data_map_cache_contents.c  \
#	../../../src/data/data_map_descriptor.c  \
#	../../../src/data/data_map_event.c  \
#	../../../src/data/data_map_query.c  \
#	../../../src/data/data_map_reply.c  \
#	../../../src/data/data_map_source.c  \
#	../../../src/data/data_map_style.c  \
#	../../../src/data/data_metadata.c  \
#	../../../src/data/data_metadata_manifest_query.c  \
#	../../../src/data/data_metadata_manifest_reply.c  \
#	../../../src/data/data_metadata_source_query.c  \
#	../../../src/data/data_metadata_source_reply.c  \
#	../../../src/data/data_nav_invocation_config.c  \
#	../../../src/data/data_nav_invocation_event.c  \
#	../../../src/data/data_nav_maneuver.c  \
#	../../../src/data/data_nav_progress.c  \
#	../../../src/data/data_nav_query.c  \
#	../../../src/data/data_nav_reply.c  \
#	../../../src/data/data_operating_hours.c  \
#	../../../src/data/data_orgdest.c  \
#	../../../src/data/data_overall_rating.c  \
#	../../../src/data/data_pair.c  \
#	../../../src/data/data_phone.c  \
#	../../../src/data/data_place.c  \
#	../../../src/data/data_place_event_cookie.c  \
#	../../../src/data/data_place_message_event.c  \
#	../../../src/data/data_point.c  \
#	../../../src/data/data_poi_content.c  \
#	../../../src/data/data_poi_events_config.c  \
#	../../../src/data/data_polyline.c  \
#	../../../src/data/data_polyline_pack.c  \
#	../../../src/data/data_position.c  \
#	../../../src/data/data_price.c  \
#	../../../src/data/data_projection_parameters.c  \
#	../../../src/data/data_pronun_query.c  \
#	../../../src/data/data_pronun_reply.c  \
#	../../../src/data/data_proxmatch.c  \
#	../../../src/data/data_proxmatch_ambiguous.c  \
#	../../../src/data/data_proxmatch_content.c  \
#	../../../src/data/data_proxmatch_summary.c  \
#	../../../src/data/data_proxpoi_error.c  \
#	../../../src/data/data_proxpoi_query.c  \
#	../../../src/data/data_proxpoi_reply.c  \
#	../../../src/data/data_proxy_api_key.c  \
#	../../../src/data/data_raster_tile_data_source_query.c  \
#	../../../src/data/data_raster_tile_data_source_reply.c  \
#	../../../src/data/data_rating.c  \
#	../../../src/data/data_realistic_sign.c  \
#	../../../src/data/data_recalc.c  \
#	../../../src/data/data_result_style.c  \
#	../../../src/data/data_retrieve_confirmed.c  \
#	../../../src/data/data_reverse_geocode_query.c  \
#	../../../src/data/data_reverse_geocode_reply.c  \
#	../../../src/data/data_road.c  \
#	../../../src/data/data_roadinfo.c  \
#	../../../src/data/data_route_corridor.c  \
#	../../../src/data/data_route_map.c  \
#	../../../src/data/data_route_number_info.c  \
#	../../../src/data/data_route_polyline.c  \
#	../../../src/data/data_route_reply_event.c  \
#	../../../src/data/data_route_request_event.c  \
#	../../../src/data/data_route_state_event.c  \
#	../../../src/data/data_route_style.c  \
#	../../../src/data/data_route_tracking_config.c  \
#	../../../src/data/data_search_cookie.c  \
#	../../../src/data/data_search_detail_event.c  \
#	../../../src/data/data_search_event_cookie.c  \
#	../../../src/data/data_search_filter.c  \
#	../../../src/data/data_search_query.c  \
#	../../../src/data/data_search_query_event.c  \
#	../../../src/data/data_search_reply.c  \
#	../../../src/data/data_sliceres.c  \
#	../../../src/data/data_speed_camera.c  \
#	../../../src/data/data_spelling_suggestion.c  \
#	../../../src/data/data_store_coupon_data.c  \
#	../../../src/data/data_store_data.c  \
#	../../../src/data/data_store_request.c  \
#	../../../src/data/data_store_response.c  \
#	../../../src/data/data_string.c  \
#	../../../src/data/data_suggest_match.c  \
#	../../../src/data/data_tagline.c  \
#	../../../src/data/data_text.c  \
#	../../../src/data/data_traffic_flow.c  \
#	../../../src/data/data_traffic_flow_item.c  \
#	../../../src/data/data_traffic_incident.c  \
#	../../../src/data/data_traffic_incidents.c  \
#	../../../src/data/data_traffic_map_reply.c  \
#	../../../src/data/data_traffic_map_road.c  \
#	../../../src/data/data_traffic_notify_query.c  \
#	../../../src/data/data_traffic_notify_reply.c  \
#	../../../src/data/data_traffic_record_identifier.c  \
#	../../../src/data/data_traffic_region.c  \
#	../../../src/data/data_transit_poi.c  \
#	../../../src/data/data_turn_map.c  \
#	../../../src/data/data_url.c  \
#	../../../src/data/data_url_args_template.c  \
#	../../../src/data/data_use_navigator.c  \
#	../../../src/data/data_vector_map.c  \
#	../../../src/data/data_vector_tile.c  \
#	../../../src/data/data_vector_tile_corridor.c  \
#	../../../src/data/data_vector_tile_descriptor.c  \
#	../../../src/data/data_vector_tile_query.c  \
#	../../../src/data/data_vector_tile_reply.c  \
#	../../../src/data/data_wanted_content.c  \
#	../../../src/data/data_want_analytics_config.c  \
#	../../../src/data/data_weather_conditions.c  \
#	../../../src/data/data_weather_forecast.c  \
#	../../../src/data/data_web_url_event.c  \
#	../../../src/data/data_wifi.c  \
#	../../../src/data/data_wifi_capabilities.c  \
#	../../../src/data/data_wifi_probe.c  \
#	../../../src/data/data_wifi_probes_config.c  \
#	../../../src/data/data_wifi_probes_event.c  \
#	../../../src/enhancedcontent/nbenhancedcontentdatasource.c  \
#	../../../src/enhancedcontent/nbenhancedcontentdownloadmanager.c  \
#	../../../src/enhancedcontent/nbenhancedcontentmanager.c  \
#	../../../src/enhancedcontent/nbmanifesthandler.c  \
#	../../../src/enhancedcontent/nbmanifestinformation.c  \
#	../../../src/enhancedcontent/nbmanifestparameters.c  \
#	../../../src/enhancedcontent/nbmetadatadatabaseoperator.c  \
#	../../../src/enhancedcontent/nbmetadatahandler.c  \
#	../../../src/enhancedcontent/nbmetadatainformation.c  \
#	../../../src/enhancedcontent/nbmetadataparameters.c  \
#	../../../src/geocode/nbgeocodehandler.c  \
#	../../../src/geocode/nbgeocodeinformation.c  \
#	../../../src/geocode/nbgeocodeparameters.c  \
#	../../../src/geocode/nbreversegeocodehandler.c  \
#	../../../src/geocode/nbreversegeocodeinformation.c  \
#	../../../src/geocode/nbreversegeocodeparameters.c  \
#	../../../src/gps/gpscriteriaset.c  \
#	../../../src/gps/gpshist.c  \
#	../../../src/gps/gpsutil.c  \
#	../../../src/gps/nbgpsfilter.c  \
#	../../../src/location/nblocationhandler.c  \
#	../../../src/location/nblocationinformation.c  \
#	../../../src/location/nblocationparameters.c  \
#	../../../src/locationservices/cellidinformation.cpp  \
#	../../../src/locationservices/cellIdlocationrequest.cpp  \
#	../../../src/locationservices/combinedlocationrequest.cpp  \
#	../../../src/locationservices/combinedrequestmanager.cpp  \
#	../../../src/locationservices/emulocationrequest.cpp  \
#	../../../src/locationservices/emulocationtracker.cpp  \
#	../../../src/locationservices/gpslocationrequest.cpp  \
#	../../../src/locationservices/gpslocationtracker.cpp  \
#	../../../src/locationservices/listenerlist.cpp  \
#	../../../src/locationservices/locationcachedb.cpp  \
#	../../../src/locationservices/locationrequest.cpp  \
#	../../../src/locationservices/locationrequestmanager.cpp  \
#	../../../src/locationservices/locationtracker.cpp  \
#	../../../src/locationservices/lsabpalgps.cpp  \
#	../../../src/locationservices/nblocationconnector.cpp  \
#	../../../src/locationservices/networklocationrequest.cpp  \
#	../../../src/locationservices/networklocationtracker.cpp  \
#	../../../src/locationservices/nmeafilereader.cpp  \
#	../../../src/locationservices/radioinformation.cpp  \
#	../../../src/locationservices/statemanager.cpp  \
#	../../../src/locationservices/trackermanager.cpp  \
#	../../../src/locationservices/wifilocationrequest.cpp  \
#	../../../src/locationservices/wifiprobes.cpp  \
#	../../../src/mobilecoupons/nbcategorylisthandler.c  \
#	../../../src/mobilecoupons/nbcategorylistinformation.c  \
#	../../../src/mobilecoupons/nbcategorylistparameters.c  \
#	../../../src/mobilecoupons/nbcouponloadimagehandler.c  \
#	../../../src/mobilecoupons/nbcouponloadimageinformation.c  \
#	../../../src/mobilecoupons/nbcouponloadimageparameters.c  \
#	../../../src/mobilecoupons/nbcouponrequesthandler.c  \
#	../../../src/mobilecoupons/nbcouponrequestinformation.c  \
#	../../../src/mobilecoupons/nbcouponrequestparameters.c  \
#	../../../src/mobilecoupons/nbmobilecouponsutils.c  \
#	../../../src/mobilecoupons/nbsearchfilter.c  \
#	../../../src/mobilecoupons/nbstorerequesthandler.c  \
#	../../../src/mobilecoupons/nbstorerequestinformation.c  \
#	../../../src/mobilecoupons/nbstorerequestparameters.c  \
#	../../../src/nav/instructset.c  \
#	../../../src/nav/navutil.c  \
#	../../../src/nav/nav_avg_speed.c  \
#	../../../src/nav/nbcamerainformation.c  \
#	../../../src/nav/nbcameraprocessor.c  \
#	../../../src/nav/nbdetourparameters.c  \
#	../../../src/nav/nbenhancedcontentprocessor.c  \
#	../../../src/nav/nbguidanceinformation.c  \
#	../../../src/nav/nbguidancemessage.c  \
#	../../../src/nav/nbguidanceprocessor.c  \
#	../../../src/nav/nbnavigation.c  \
#	../../../src/nav/nbnavigationstate.c  \
#	../../../src/nav/nbpointsofinterestinformation.c  \
#	../../../src/nav/nbpositionprocessor.c  \
#	../../../src/nav/nbpronunhandler.c  \
#	../../../src/nav/nbpronuninformation.c  \
#	../../../src/nav/nbpronunparameters.c  \
#	../../../src/nav/nbrecalclimit.c  \
#	../../../src/nav/nbroutehandler.c  \
#	../../../src/nav/nbrouteinformation.c  \
#	../../../src/nav/nbrouteparameters.c  \
#	../../../src/nav/nbtraffichandler.c  \
#	../../../src/nav/nbtrafficinformation.c  \
#	../../../src/nav/nbtrafficparameters.c  \
#	../../../src/nav/nbtrafficprocessor.c  \
#	../../../src/nav/nbvectormapprefetchprocessor.c  \
#	../../../src/nbcommon/nbcontext.c  \
#	../../../src/nbcommon/nbformat.c  \
#	../../../src/nbcommon/nbpersistentdata.c  \
#	../../../src/nbcommon/nbpointiteration.c  \
#	../../../src/nbcommon/nbsolarcalculator.c  \
#	../../../src/nbcommon/nbtristrip.c  \
#	../../../src/nbcommon/nbutility.c  \
#	../../../src/nbimage/nbimage.c  \
#	../../../src/network/nbnetwork.c  \
#	../../../src/proxpoi/nbsearchhandler.c  \
#	../../../src/proxpoi/nbsearchinformation.c  \
#	../../../src/proxpoi/nbsearchparameters.c  \
#	../../../src/publictransitmanager/nbpublictransitmanager.c  \
#	../../../src/publictransitmanager/nbpublictransitmanagerdb.c  \
#	../../../src/qalog/nbqalog.c  \
#	../../../src/qalog/nbqarecord.c  \
#	../../../src/rastermap/nbrastermaphandler.c  \
#	../../../src/rastermap/nbrastermapinformation.c  \
#	../../../src/rastermap/nbrastermapparameters.c  \
#	../../../src/rastertile/nbrastertiledatasourcehandler.c  \
#	../../../src/rastertile/nbrastertiledatasourceinformation.c  \
#	../../../src/rastertile/nbrastertiledatasourceparameters.c  \
#	../../../src/rastertile/nbrastertilemanager.c  \
#	../../../src/singlesearch/nbsinglesearchhandler.c  \
#	../../../src/singlesearch/nbsinglesearchinformation.c  \
#	../../../src/singlesearch/nbsinglesearchparameters.c  \
#	../../../src/singlesearch/nbsuggestmatch.c  \
#	../../../src/spatial/nbspatial.c  \
#	../../../src/spatial/nbvectortransform.c  \
#	../../../src/spatial/spatialvector.c  \
#	../../../src/spatial/transformutility.c  \
#	../../../src/tileservice/nbtilemanager.c  \
#	../../../src/util/format.c  \
#	../../../src/util/nbversion.c  \
#	../../../src/vectortile/nbvectortilehandler.c  \
#	../../../src/vectortile/nbvectortileinformation.c  \
#	../../../src/vectortile/nbvectortileinformationprivate.c  \
#	../../../src/vectortile/nbvectortilemanager.c  \
#	../../../src/vectortile/nbvectortileparameters.c  \
#	../../../src/wifiprobes/nbwifiprobes.c  \

LOCAL_SRC_FILES = $(SRCS)

#LOCAL_LDFLAGS := $(P4PATH)/core/releases/nbm/android/nbm_1.0.0.16_android/libs/libNBM.a
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbpal.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/abpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libabpal.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libcoreservices.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/nbgm/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbgm.so

include $(BUILD_SHARED_LIBRARY)

#########################################
#LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_STATIC_LIBRARIES := nbservices
#LOCAL_MODULE := nbservices_help
#include $(BUILD_SHARED_LIBRARY)