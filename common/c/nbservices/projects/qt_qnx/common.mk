# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== NAME - name of the project (default - name of project directory).
NAME=nbservices

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

# Next lines are for C++ projects only

EXTRA_SUFFIXES+=cxx cpp

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-lang-c++ -Wl,-E -M, -Wl,-z relro -Wl,--as-needed -Wl,-z,now

ifdef INSTRUMENT
LDFLAGS+= -lprofilingS
endif

VFLAG_g=-gstabs+

#===== CCFLAGS - add the flags to the C compiler command line.
CCFLAGS+=-w9 -fPIC -fstack-protector-strong -v -g -DENABLE_LOGGING_TO_OUTPUT

ifdef INSTRUMENT	
CCFLAGS+= -g -O0 -finstrument-functions
endif

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../include  \
	$(PROJECT_ROOT)/../../include/protected  \
	$(PROJECT_ROOT)/../../include/private  \
	$(PROJECT_ROOT)/../../include/qt_qnx  \
	$(PROJECT_ROOT)/../../include/generated  \
	$(PROJECT_ROOT)/../../include/generated\private  \
    $(PROJECT_ROOT)/../../src/map/tilemanager \
	$(NIMCORE_INC)/protected  \
	$(NIMCORE_INC)/private  \
	$(NIMCORE_INC)/qt_qnx  \
	$(NIMCORE_INC)  \
	$(NIMNBIPAL_INC)/qt_qnx  \
	$(NIMNBIPAL_INC)/private  \
	$(NIMNBIPAL_INC)/nbpal  \
    $(NIMNBIPAL_INC)/boost \
	$(NIMNBIPAL_INC) \
	$(NIMABPAL_INC) \
	$(NIMABPAL_INC)/qt_qnx \
	$(NIMNBUI_INC) \
	$(NIMNBUI_INC)/protected \
	$(NIMNBGMMAIN_INC) \
	$(NIMNBGMMAIN_INC)/qt_qnx \
	$(CORENBM_INC)

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../../src/analytics  \
	$(PROJECT_ROOT)/../../src/data  \
	$(PROJECT_ROOT)/../../src/enhancedcontent  \
	$(PROJECT_ROOT)/../../src/geocode  \
	$(PROJECT_ROOT)/../../src/gps  \
	$(PROJECT_ROOT)/../../src/location  \
	$(PROJECT_ROOT)/../../src/locationservices  \
	$(PROJECT_ROOT)/../../src/map/mapview \
	$(PROJECT_ROOT)/../../src/map/layermanager \
	$(PROJECT_ROOT)/../../src/map/tempmobius \
    $(PROJECT_ROOT)/../../src/map/tilemanager \
	$(PROJECT_ROOT)/../../src/mobilecoupons  \
	$(PROJECT_ROOT)/../../src/nav  \
	$(PROJECT_ROOT)/../../src/navapi  \
	$(PROJECT_ROOT)/../../src/nbcommon  \
	$(PROJECT_ROOT)/../../src/nbimage  \
	$(PROJECT_ROOT)/../../src/network  \
	$(PROJECT_ROOT)/../../src/poiimagemanager  \
	$(PROJECT_ROOT)/../../src/protocol  \
	$(PROJECT_ROOT)/../../src/protocol/metadata  \
	$(PROJECT_ROOT)/../../src/protocol/maptile  \
	$(PROJECT_ROOT)/../../src/proxpoi  \
	$(PROJECT_ROOT)/../../src/publictransitmanager  \
	$(PROJECT_ROOT)/../../src/qalog  \
	$(PROJECT_ROOT)/../../src/rastermap  \
	$(PROJECT_ROOT)/../../src/rastertile  \
	$(PROJECT_ROOT)/../../src/singlesearch  \
	$(PROJECT_ROOT)/../../src/spatial  \
	$(PROJECT_ROOT)/../../src/tileservice  \
	$(PROJECT_ROOT)/../../src/util  \
	$(PROJECT_ROOT)/../../src/vectortile  \
	$(PROJECT_ROOT)/../../src/wifiprobes

EXCLUDE_OBJS+=CoreUBaseString.o CoreAscString.o CoreUniString.o CoreUtfString.o

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

