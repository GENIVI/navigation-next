LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_MODULE := abservices



# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../..
$(warning Value of PROJ_ROOT is $(PROJ_ROOT))
P4PATH :=    $(LOCAL_PATH)/../../../../../../../..
$(warning Value of P4PATH is $(P4PATH))
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/private
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/protected
INCLUDE_DIRS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbservices/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbservices/$(BRANCH)/include/protected
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbservices/$(BRANCH)/include/android_ndk



LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))

	
SRCS := ../../../src/data/data_add_feature.c  \
	../../../src/data/data_asr_field_data.c  \
	../../../src/data/data_asr_recentloc.c  \
	../../../src/data/data_asr_reco_query.c  \
	../../../src/data/data_asr_reco_reply.c  \
	../../../src/data/data_asr_stats_query.c  \
	../../../src/data/data_asr_stats_reply.c  \
	../../../src/data/data_asr_timing.c  \
	../../../src/data/data_asr_utterance.c  \
	../../../src/data/data_auth_query.c  \
	../../../src/data/data_auth_reply.c  \
	../../../src/data/data_bind_license.c  \
	../../../src/data/data_bundle.c  \
	../../../src/data/data_bundle_option.c  \
	../../../src/data/data_client_stored_message.c  \
	../../../src/data/data_datastore.c  \
	../../../src/data/data_datastore_query.c  \
	../../../src/data/data_datastore_reply.c  \
	../../../src/data/data_error_code.c  \
	../../../src/data/data_error_msg.c  \
	../../../src/data/data_ers_contact.c  \
	../../../src/data/data_ers_query.c  \
	../../../src/data/data_ers_reply.c  \
	../../../src/data/data_feature.c  \
	../../../src/data/data_feature_list.c  \
	../../../src/data/data_fileset.c  \
	../../../src/data/data_fileset_query.c  \
	../../../src/data/data_fileset_reply.c  \
	../../../src/data/data_fileset_status_query.c  \
	../../../src/data/data_fileset_status_reply.c  \
	../../../src/data/data_license.c  \
	../../../src/data/data_license_query.c  \
	../../../src/data/data_license_reply.c  \
	../../../src/data/data_license_request.c  \
	../../../src/data/data_license_response.c  \
	../../../src/data/data_maptile_access_token.c  \
	../../../src/data/data_message.c  \
	../../../src/data/data_message_confirm.c  \
	../../../src/data/data_message_confirm_query.c  \
	../../../src/data/data_message_id.c  \
	../../../src/data/data_message_nag.c  \
	../../../src/data/data_message_reply.c  \
	../../../src/data/data_message_text.c  \
	../../../src/data/data_msg_content.c  \
	../../../src/data/data_msg_recipient.c  \
	../../../src/data/data_place_message.c  \
	../../../src/data/data_place_message_flags.c  \
	../../../src/data/data_place_msg_banner.c  \
	../../../src/data/data_place_msg_content.c  \
	../../../src/data/data_place_msg_error.c  \
	../../../src/data/data_place_msg_id.c  \
	../../../src/data/data_place_msg_recipient.c  \
	../../../src/data/data_place_msg_recipient_result.c  \
	../../../src/data/data_price_list.c  \
	../../../src/data/data_price_option.c  \
	../../../src/data/data_profile_query.c  \
	../../../src/data/data_profile_reply.c  \
	../../../src/data/data_promo_code.c  \
	../../../src/data/data_purchase_option.c  \
	../../../src/data/data_recommended_option.c  \
	../../../src/data/data_remove_feature.c  \
	../../../src/data/data_request.c  \
	../../../src/data/data_response.c  \
	../../../src/data/data_save_optin_response.c  \
	../../../src/data/data_send_message_query.c  \
	../../../src/data/data_send_message_reply.c  \
	../../../src/data/data_send_msg_result.c  \
	../../../src/data/data_send_place_message_query.c  \
	../../../src/data/data_send_place_message_reply.c  \
	../../../src/data/data_share_content.c  \
	../../../src/data/data_subscribed_message.c  \
	../../../src/data/data_sync_add_item.c  \
	../../../src/data/data_sync_assign_item_id.c  \
	../../../src/data/data_sync_delete_item.c  \
	../../../src/data/data_sync_error.c  \
	../../../src/data/data_sync_modify_item.c  \
	../../../src/data/data_sync_places_db_query.c  \
	../../../src/data/data_sync_places_db_reply.c  \
	../../../src/data/data_sync_places_db_status_query.c  \
	../../../src/data/data_sync_places_db_status_reply.c  \
	../../../src/data/data_sync_places_query.c  \
	../../../src/data/data_sync_places_reply.c  \
	../../../src/data/data_sync_places_status_query.c  \
	../../../src/data/data_sync_places_status_reply.c  \
	../../../src/data/data_text_msg_content.c  \
	../../../src/data/data_want_license_message.c  \
	../../../src/datastore/abdatastorehandler.c  \
	../../../src/datastore/abdatastoreinformation.c  \
	../../../src/datastore/abdatastoreparameters.c  \
	../../../src/ers/abershandler.c  \
	../../../src/ers/abersinformation.c  \
	../../../src/ers/abersparameters.c  \
	../../../src/fileset/abfilesethandler.c  \
	../../../src/fileset/abfilesetinformation.c  \
	../../../src/fileset/abfilesetparameters.c  \
	../../../src/fileset/abfilesetstatushandler.c  \
	../../../src/fileset/abfilesetstatusinformation.c  \
	../../../src/fileset/abfilesetstatusparameters.c  \
	../../../src/license/ablicensehandler.c  \
	../../../src/license/ablicenseinformation.c  \
	../../../src/license/ablicenseparameters.c  \
	../../../src/motd/abservermessagehandler.c  \
	../../../src/motd/abservermessageinformation.c  \
	../../../src/motd/abservermessageparameters.c  \
	../../../src/msg/absharehandler.c  \
	../../../src/msg/abshareinformation.c  \
	../../../src/msg/abshareparameters.c  \
	../../../src/network/abnetworkconfiguration.c  \
	../../../src/profile/abprofilehandler.c  \
	../../../src/profile/abprofileinformation.c  \
	../../../src/profile/abprofileparameters.c  \
	../../../src/qalog/abqalog.c  \
	../../../src/qalog/abqaloghandler.c  \
	../../../src/qalog/abqaloginformation.c  \
	../../../src/qalog/abqalogparameters.c  \
	../../../src/sms/absms.c  \
	../../../src/speech/abspeechrecognitionhandler.c  \
	../../../src/speech/abspeechrecognitioninformation.c  \
	../../../src/speech/abspeechrecognitionparameters.c  \
	../../../src/speech/abspeechstatisticshandler.c  \
	../../../src/speech/abspeechstatisticsparameters.c  \
	../../../src/subscription/abauthenticationhandler.c  \
	../../../src/subscription/abauthenticationinformation.c  \
	../../../src/subscription/abauthenticationparameters.c  \
	../../../src/sync/absynchronizationhandler.c  \
	../../../src/sync/absynchronizationinformation.c  \
	../../../src/sync/absynchronizationparameters.c  \
	../../../src/sync/absynchronizationstatushandler.c  \
	../../../src/sync/absynchronizationstatusinformation.c  \
	../../../src/sync/absynchronizationstatusparameters.c  \
	../../../src/util/abutil.c  \
	../../../src/util/abversion.c  \
	../../../src/version/abserverversionhandler.c  \
	../../../src/version/abserverversioninformation.c  \
	../../../src/version/abserverversionparameters.c  \
      
		
LOCAL_SRC_FILES = $(SRCS)

LOCAL_LDFLAGS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbpal.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/abpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libabpal.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/coreservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libcoreservices.so
LOCAL_LDFLAGS += $(P4PATH)/client/c/core/nbservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbservices.so

include $(BUILD_SHARED_LIBRARY)
