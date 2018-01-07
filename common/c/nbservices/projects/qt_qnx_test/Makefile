QMAKE_TARGET  = ${PROJECT_LOC}unittests
QMAKE         = $(QNX_HOST)/usr/bin/qmake
TARGET        = $(QMAKE_TARGET)

all: Makefile $(QMAKE_TARGET)

release: Makefile $(QMAKE_TARGET)_release

debug: Makefile $(QMAKE_TARGET)_debug

simdebug: Makefile $(QMAKE_TARGET)_simdebug

simrelease: Makefile $(QMAKE_TARGET)_simrelease

clean:
	$(MAKE) -C ./arm -f Makefile distclean
	$(MAKE) -C ./x86 -f Makefile distclean

Makefile: FORCE
	$(QMAKE) -spec unsupported/blackberry-armv7le-g++ -o arm/Makefile $(QMAKE_TARGET).pro CONFIG+=device
	$(QMAKE) -spec unsupported/blackberry-x86-g++ -o x86/Makefile $(QMAKE_TARGET).pro CONFIG+=simulator

FORCE:

$(QMAKE_TARGET): device simulator

$(QMAKE_TARGET)_release: device_release simulator_release

$(QMAKE_TARGET)_debug: device_debug simulator_debug

$(QMAKE_TARGET)_simdebug: simulator_debug

$(QMAKE_TARGET)_simrelease: simulator_release

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

