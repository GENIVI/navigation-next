#VERSION 4.7.0

OPTIONAL=$(if $(1),$(1),$(2))

OUTPUT_KIND=$(call OPTIONAL,$(VARIANT_BUILD_TYPE),o)

CONFIG:=$(filter-out $(OUTPUT_KIND) le be spe v7,$(VARIANT_LIST))
ifeq ($(CONFIG),)
LIB_SUFFIX:=
DEBUG_SUFFIX:=_r
else
ifeq ($(CONFIG),g)
LIB_SUFFIX:=_g
else
LIB_SUFFIX:=-$(CONFIG)
endif
DEBUG_SUFFIX:=_$(CONFIG)
endif

RAW_ENDIAN_KIND:=$(lastword $(sort $(filter le be,$(VARIANT_LIST))))
ENDIAN_KIND:=$(call OPTIONAL,$(RAW_ENDIAN_KIND),le)

ifeq ($(CPU),arm)
CPU_VARIANT:=$(filter v7,$(VARIANT_LIST))
else ifeq ($(CPU),ppc)
CPU_VARIANT:=$(filter spe,$(VARIANT_LIST))
endif
RAW_ENDIAN_KIND:=$(if $(CPUVARIANT),$(RAW_ENDIAN_KIND)-$(CPU_VARIANT),$(RAW_ENDIAN_KIND))
CPU_VARIANT:=$(if $(CPU_VARIANT),$(ENDIAN_KIND)_$(CPU_VARIANT),$(ENDIAN_KIND))

VARFAMILY_VARIANTS=\
			$(1)_$(OS)_$(CPU)_$(CPU_VARIANT)_$(OUTPUT_KIND)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(OS)_$(CPU)_$(CPU_VARIANT)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(OS)_$(CPU)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(OS)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(CPU)_$(CPU_VARIANT)_$(OUTPUT_KIND)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(CPU)_$(CPU_VARIANT)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(CPU)$(2)$(DEBUG_SUFFIX) \
			$(1)$(2)$(DEBUG_SUFFIX) \
			$(1)_$(OS)_$(CPU)_$(CPU_VARIANT)_$(OUTPUT_KIND)$(2) \
			$(1)_$(OS)_$(CPU)_$(CPU_VARIANT)$(2) \
			$(1)_$(OS)_$(CPU)$(2) \
			$(1)_$(OS)$(2) \
			$(1)_$(CPU)_$(CPU_VARIANT)_$(OUTPUT_KIND)$(2) \
			$(1)_$(CPU)_$(CPU_VARIANT)$(2) \
			$(1)_$(CPU)
VARFAMILY_ALL=$(call VARFAMILY_VARIANTS,$(1),$(2)) $(1)$(2)

SELECT_EXPRESSION = $(firstword $(foreach a,$(call VARFAMILY_ALL,$(1),), $(if $($(a)),$(a),)))
MERGE_EXPRESSION=$(strip $(foreach a,$(call VARFAMILY_ALL,$(2),$(3)), $(call $(1),$($(a)))))
MERGE_EXPRESSION_V=$(strip $(foreach a,$(call VARFAMILY_VARIANTS,$(2),$(3)), $(call $(1),$($(a)))))
IDENTITY=$(1)

STARTS_WITH=$(if $(filter-out $(patsubst $(1)%,%,$(2)),$(2)),$(1),)
ENDS_WITH=$(if $(filter-out $(patsubst %$(1),%,$(2)),$(2)),$(1),)
dquote:="
UNQUOTE=$(if $(and $(call STARTS_WITH,$(dquote),$(1)),$(call ENDS_WITH,$(dquote),$(1))),\
		$(patsubst $(dquote)%$(dquote),%,$(1)),\
		$(1))

# Macros that search for the folder of matching variant to add to the library path
LIBPATTERN=$(pref)$(if $(RAW_ENDIAN_KIND),$(sep)$(RAW_ENDIAN_KIND))$(if $(CONFIG),-$(CONFIG))/*.$(pref)
LIBMATCH = $(dir $(sort $(wildcard $(foreach sep,. -,\
	$(foreach pref,$(2),\
		$(foreach path,$(1)/$(OS)/$(CPU) $(1)/$(CPU) $(1),$(path)/$(LIBPATTERN)))))))
LIBMATCH-opt=$(call OPTIONAL,$(call LIBMATCH,$(1),$(2)),$(1))
MATCHING_LIB_FOLDER=$(call MATCHING_LIB_FOLDER-impl,$(call UNQUOTE,$(1)))
MATCHING_LIB_FOLDER-impl=\
	$(if $(call ENDS_WITH,@dynamic,$(1)),\
			$(call LIBMATCH-opt,$(call UNQUOTE,$(patsubst %@dynamic,%,$(1))),so),\
		$(if $(call ENDS_WITH,@static,$(1)),\
				$(call LIBMATCH-opt,$(call UNQUOTE,$(patsubst %@static,%,$(1))),a),\
			$(call LIBMATCH-opt,$(call UNQUOTE,$(1)),so a)))

GCC_VERSION:=$($(call SELECT_EXPRESSION,GCC_VERSION))
DEFCOMPILER_TYPE:= $($(call SELECT_EXPRESSION, DEFCOMPILER_TYPE))

EXTRA_LIBVPATH:= $(call MERGE_EXPRESSION,MATCHING_LIB_FOLDER,EXTRA_LIBVPATH)
extra_incvpath_tmp:=$(call MERGE_EXPRESSION,IDENTITY,EXTRA_INCVPATH,)
EXTRA_INCVPATH = $(call MERGE_EXPRESSION,IDENTITY,EXTRA_INCVPATH,_@$(basename $@)) \
	$(extra_incvpath_tmp)
LATE_SRCVPATH := $(call MERGE_EXPRESSION,IDENTITY,EXTRA_SRCVPATH)
EXTRA_OBJS := $($(call SELECT_EXPRESSION,EXTRA_OBJS))

CCFLAGS += $(call MERGE_EXPRESSION_V,IDENTITY,CCFLAGS,) $(call MERGE_EXPRESSION_V,IDENTITY,CCFLAGS,_@$(basename $@))
ASFLAGS += $(call MERGE_EXPRESSION_V,IDENTITY,ASFLAGS,) $(call MERGE_EXPRESSION_V,IDENTITY,ASFLAGS,_@$(basename $@))
ARFLAGS += $(call MERGE_EXPRESSION_V,IDENTITY,ARFLAGS,)
LDFLAGS += $(call MERGE_EXPRESSION_V,IDENTITY,LDFLAGS,)

EXTRA_CLEAN := $(call MERGE_EXPRESSION_V,IDENTITY,EXTRA_CLEAN,)

# a macro that we will evaluate to define pre/post macros for libraries that request static or dynamic link
define LIB_HANDLER
LIBPREF_$(subst @static,,$(subst @dynamic,,$(1)))+=$(if $(call ENDS_WITH,@static,$(1)),-Bstatic,$(if $(call ENDS_WITH,@dynamic,$(1)),-Bdynamic,))
LIBPOST_$(subst @static,,$(subst @dynamic,,$(1)))+=$(if $(call ENDS_WITH,@static,$(1)),-Bdynamic,$(if $(call ENDS_WITH,@dynamic,$(1)),-Bstatic,))
endef

LIBS-raw:=$(patsubst %S_g, %_gS, $(foreach lib, $(call MERGE_EXPRESSION,IDENTITY,LIBS),$(subst *,$(LIB_SUFFIX),$(lib))))
$(foreach lib,$(LIBS-raw),$(eval $(call LIB_HANDLER,$(lib))))

LIBS:= $(LIBSOPT) $(patsubst %@static,%,$(patsubst %@dynamic,%,$(LIBS-raw)))

ifdef LIBNAMES 
LIBNAMES:=
endif 
ifneq ($(LIBS),)
EXTRA_DEPS += $(wildcard $(foreach a,$(EXTRA_LIBVPATH),$(a)/*.a))
endif

USEFILE:=$($(call SELECT_EXPRESSION,USEFILE))

BUILDNAME:=$($(call SELECT_EXPRESSION,BUILDNAME))$(if $(suffix $(BUILDNAME)),,$(IMAGE_SUFF_$(BUILD_TYPE)))
BUILDNAME_SAR:= $(patsubst %$(IMAGE_SUFF_$(BUILD_TYPE)),%S.a,$(BUILDNAME))

PRE_BUILD:=$($(call SELECT_EXPRESSION,PRE_BUILD))
POST_BUILD:=$($(call SELECT_EXPRESSION,POST_BUILD))

# Define these because CDT insists on trying to call them
main-build: all ;
pre-build: ;
post-build: ;
.PHONY: pre-build main-build post-build
