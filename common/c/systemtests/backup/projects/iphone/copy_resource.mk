# root directory of the current project
PROJ_ROOT := $(realpath $(CURDIR)/../..)
CORETECH_ROOT    := $(shell echo $(PROJ_ROOT) | sed 's/\/$(PROJ_NAME).*$$//g')

OBJ_ROOT := ./build/Debug-iphonesimulator

#create build output directory if it does not exist
CHECK_OBJ_ROOT := $(shell mkdir -p $(OBJ_ROOT))

PYTHON = /usr/local/bin/python2.4

.PHONY: copy_data_files
copy_data_files:
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/directions/directions-en-us.sexp $(PROJ_ROOT)/supportfiles/navfileset/directions/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/michelle/voices.sexp $(PROJ_ROOT)/supportfiles/navfileset/michelle/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl
	$(Q)$(PYTHON) $(PROJ_ROOT)/supportfiles/sexp2tps.py $(PROJ_ROOT)/supportfiles/navfileset/routing-images/images.sexp $(PROJ_ROOT)/supportfiles/navfileset/routing-images/config.tps $(PROJ_ROOT)/tpslib/compiled/data.tpl

