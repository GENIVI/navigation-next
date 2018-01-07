QMAKE_TARGET  = ${PROJECT_LOC}systemtests
QMAKE         = $(QNX_HOST)/usr/bin/qmake
TARGET        = $(QMAKE_TARGET)

all: Makefile_all $(QMAKE_TARGET)

release: Makefile_release $(QMAKE_TARGET)_release

debug: Makefile_debug $(QMAKE_TARGET)_debug

release_x86: Makefile_x86 $(QMAKE_TARGET)_release_x86

release_arm: Makefile_arm $(QMAKE_TARGET)_release_arm

debug_x86: Makefile_x86 $(QMAKE_TARGET)_debug_x86

debug_arm: Makefile_arm $(QMAKE_TARGET)_debug_arm

clean:
	$(if $(wildcard ./arm/Makefile), $(MAKE) -C ./arm -f Makefile distclean, @echo " ./arm/Makefile does not exist")
	$(if $(wildcard ./x86/Makefile), $(MAKE) -C ./x86 -f Makefile distclean, @echo " ./x86/Makefile does not exist")

Makefile_all:
	$(QMAKE) -spec unsupported/blackberry-armv7le-qcc -o arm/Makefile $(QMAKE_TARGET).pro CONFIG+=device
	$(QMAKE) -spec unsupported/blackberry-x86-qcc -o x86/Makefile $(QMAKE_TARGET).pro CONFIG+=simulator

Makefile_release:
	$(QMAKE) -spec unsupported/blackberry-armv7le-qcc -o arm/Makefile $(QMAKE_TARGET).pro CONFIG+="device release" 
	$(QMAKE) -spec unsupported/blackberry-x86-qcc -o x86/Makefile $(QMAKE_TARGET).pro CONFIG+="simulator release"

Makefile_debug:
	$(QMAKE) -spec unsupported/blackberry-armv7le-qcc -o arm/Makefile $(QMAKE_TARGET).pro CONFIG+="device debug"
	$(QMAKE) -spec unsupported/blackberry-x86-qcc -o x86/Makefile $(QMAKE_TARGET).pro CONFIG+="simulator debug"

Makefile_arm:
	$(QMAKE) -spec unsupported/blackberry-armv7le-qcc -o arm/Makefile $(QMAKE_TARGET).pro CONFIG+=device

Makefile_x86:
	$(QMAKE) -spec unsupported/blackberry-x86-qcc -o x86/Makefile $(QMAKE_TARGET).pro CONFIG+=simulator


$(QMAKE_TARGET): device simulator

$(QMAKE_TARGET)_release: device_release simulator_release

$(QMAKE_TARGET)_debug: device_debug simulator_debug

$(QMAKE_TARGET)_release_x86: simulator_release

$(QMAKE_TARGET)_release_arm: device_release

$(QMAKE_TARGET)_debug_x86: simulator_debug

$(QMAKE_TARGET)_debug_arm: device_debug

device:
	$(MAKE) -C ./arm -f Makefile all

device_release:
	$(MAKE) -C ./arm -f Makefile release-all

device_debug:
	$(MAKE) -C ./arm -f Makefile debug-all

simulator:
	$(MAKE) -C ./x86 -f Makefile all

simulator_release:
	$(MAKE) -C ./x86 -f Makefile release-all

simulator_debug:
	$(MAKE) -C ./x86 -f Makefile debug-all

