PROJ_NAME := abpal

PLATFORM ?= linux

# include platform specific makefile
include $(PLATFORM).mk

V ?= 0
ifeq ($(V), 0)
    Q = @
else
    Q =
endif

ifneq ($(SRC_DIRS),)
SRCS += $(foreach s,$(SRC_DIRS),$(wildcard $(s)/*.c))
SRCS += $(foreach s,$(SRC_DIRS),$(wildcard $(s)/*.cpp))
VPATH += $(SRC_DIRS)
CFLAGS += $(foreach d,$(SRC_DIRS),-I$(d))
endif

ifneq ($(INCLUDE_DIRS),)
CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))
endif

ifneq ($(EXCLUDE_SRCS),)
SRCS := $(filter-out $(EXCLUDE_SRCS),$(SRCS))
endif

OBJS := $(notdir $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRCS))))
OBJS := $(foreach o,$(OBJS),$(OBJ_ROOT)/$(o))

ifneq ($(VERSION_FILE),)
LIB_VERSION := $(shell cat $(PROJ_ROOT)/resources/$(PLATFORM)/version.txt)
DEPENDS_TARGETS += $(VERSION_FILE)
endif

DEPENDS_DIR_MAKE := $(foreach d,$(DEPENDS_DIR),$(d)/.make)
DEPENDS_DIR_CLEAN := $(foreach d,$(DEPENDS_DIR),$(d)/.clean)
DEPENDS_DIR_DISTCLEAN := $(foreach d,$(DEPENDS_DIR),$(d)/.distclean)

DEPENDS_TARGET_CLEAN := $(foreach t,$(DEPENDS_TARGETS),$(t)_clean) 
DEPENDS_TARGET_DISTCLEAN := $(foreach t,$(DEPENDS_TARGETS),$(t)_distclean) 

EXTRA_TARGETS_DIR_MAKE := $(foreach d,$(EXTRA_TARGETS_DIR),$(d)/.make)
EXTRA_TARGETS_DIR_CLEAN := $(foreach d,$(EXTRA_TARGETS_DIR),$(d)/.clean)
EXTRA_TARGETS_DIR_DISTCLEAN := $(foreach d,$(EXTRA_TARGETS_DIR),$(d)/.distclean)

DEFAULT_TARGET :=

ifneq ($(APP_NAME),)
DEFAULT_TARGETS += $(OBJ_ROOT)/$(APP_NAME)
endif

ifneq ($(SHARED_LIB_NAME),)
ifneq ($(LIB_VERSION),)
MAJOR_VERSION := $(shell echo $(VERSION) | awk -F. '{print $$1}')
MINOR_VERSION := $(shell echo $(VERSION) | awk -F. '{print $$2}')
SUB_VERSION := $(shell echo $(VERSION) | awk -F. '{print $$3}')
BUILD_NUMBER := $(shell echo $(VERSION) | awk -F. '{print $$4}')

#compose lib file name with major, minor version
SHARED_LIB_FILE_NAME := $(OBJ_ROOT)/$(SHARED_LIB_NAME).$(LIB_VERSION)
else
SHARED_LIB_FILE_NAME := $(OBJ_ROOT)/$(SHARED_LIB_NAME).0
endif

DEFAULT_TARGETS += $(OBJ_ROOT)/$(SHARED_LIB_NAME)
endif

ifneq ($(STATIC_LIB_NAME),)
DEFAULT_TARGETS += $(OBJ_ROOT)/$(STATIC_LIB_NAME)
endif

ifneq ($(PARTIAL_LIB_NAME),)
DEFAULT_TARGETS += $(OBJ_ROOT)/$(PARTIAL_LIB_NAME)
endif

.PHONY: all
all: $(DEFAULT_TARGETS) $(EXTRA_TARGETS_DIR_MAKE) $(EXTRA_TARGETS)

ifneq ($(DEPENDS_DIR),)
$(DEPENDS_DIR_MAKE): FORCE
	$Q$(MAKE) -C $(dir $@)

$(DEPENDS_DIR_CLEAN): FORCE
	$Q$(MAKE) -C $(dir $@) clean

$(DEPENDS_DIR_DISTCLEAN): FORCE
	$Q$(MAKE) -C $(dir $@) distclean
endif

ifneq ($(EXTRA_TARGETS_DIR),)
$(EXTRA_TARGETS_DIR_MAKE): FORCE
	$Q$(MAKE) -C $(dir $@)

$(EXTRA_TARGETS_DIR_CLEAN): FORCE
	$Q$(MAKE) -C $(dir $@) clean

$(EXTRA_TARGETS_DIR_DISTCLEAN): FORCE
	$Q$(MAKE) -C $(dir $@) distclean
endif

ifneq ($(APP_NAME),)
$(OBJ_ROOT)/$(APP_NAME): $(DEPENDS_DIR_MAKE) $(DEPENDS_TARGETS) $(OBJS)
	@echo Creating $(notdir $@)
	$Q$(CC) -o $@ $(OBJS) $(EXTRA_OBJS) $(LDFLAGS)
endif

ifneq ($(STATIC_LIB_NAME),)
$(OBJ_ROOT)/$(STATIC_LIB_NAME): $(DEPENDS_DIR_MAKE) $(DEPENDS_TARGETS) $(OBJS)
	@echo Creating $(notdir $@)
	$Q$(AR) cru $@ $(OBJS) $(EXTRA_OBJS)
endif

ifneq ($(SHARED_LIB_NAME),)
$(OBJ_ROOT)/$(SHARED_LIB_NAME): $(DEPENDS_DIR_MAKE) $(DEPENDS_TARGETS) $(OBJS)
	@echo Creating $(notdir $@)
	$Q$(CC) -shared -Wl,-soname,$(notdir $@) -o $(SHARED_LIB_FILE_NAME) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS)
	$(Q)ln -sf $(SHARED_LIB_FILE_NAME) $@
endif

ifneq ($(PARTIAL_LIB_NAME),)
$(OBJ_ROOT)/$(PARTIAL_LIB_NAME): $(DEPENDS_DIR_MAKE) $(DEPENDS_TARGETS) $(OBJS)
	@echo Creating $(notdir $@)
	$Q$(LD) --partial -o $@ $(OBJS)	
endif

ifneq ($(VERSION_FILE),)
$(VERSION_FILE) : $(PROJ_ROOT)/resources/$(PLATFORM)/version.txt $(PROJ_ROOT)/resources/version.gen.txt
	@echo Generating $(notdir $@)
	$(Q)$(PYTHON) $(PROJ_ROOT)/build/version.py $(PROJ_ROOT) $(PLATFORM) 

.PHONY: $(VERSION_FILE)_clean
$(VERSION_FILE)_clean:
	$(Q)$(RM) $(VERSION_FILE)

.PHONY: $(VERSION_FILE)_distclean
$(VERSION_FILE)_distclean:
	$(Q)$(RM) $(VERSION_FILE)
endif

$(OBJ_ROOT)/%.o : %.c
	@echo Compiling $(notdir $<)
	$Q$(CC) $(CFLAGS) -c -o $@ $<
ifneq ($(GENERATE_DEPENDENCY),)
	$Q$(CC) $(CFLAGS) -MM -MP -MT $@ -o $(OBJ_ROOT)/$*.d $<
endif

$(OBJ_ROOT)/%.o : %.cpp
	@echo Compiling $(notdir $<)
	$Q$(CXX) $(CFLAGS) -c -o $@ $<
ifneq ($(GENERATE_DEPENDENCY),)
	$Q$(CXX) $(CFLAGS) -MM -MP -MT $@ -o $(OBJ_ROOT)/$*.d $<
endif

.PHONY: clean
clean: $(DEPENDS_DIR_CLEAN) $(DEPENDS_TARGET_CLEAN) $(EXTRA_TARGETS_DIR_CLEAN)
	$(Q)cd $(OBJ_ROOT) && $(RM) *.o *.d *.a lib*.so* $(APP_NAME) $(CLEAN_FILES)

.PHONY: distclean
distclean: $(DEPENDS_DIR_DISTCLEAN) $(DEPENDS_TARGET_DISTCLEAN) $(EXTRA_TARGETS_DIR_DISTCLEAN)
	$(Q)$(RM) $(OBJ_ROOT)

.PHONY: FORCE
FORCE:

-include $(OBJS:.o=.d)

