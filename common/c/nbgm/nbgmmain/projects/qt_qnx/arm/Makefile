LIST=VARIANT
CCFLAGS+= -mthumb -mtune=cortex-a9
ifndef QRECURSE
QRECURSE=recurse.mk
ifdef QCONFIG
QRDIR=$(dir $(QCONFIG))
endif
endif
include $(QRDIR)$(QRECURSE)
