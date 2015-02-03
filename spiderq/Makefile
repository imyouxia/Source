# This is top level makefile. The real shit is at src/Makefile

default:all

.DEFAULT:
	@cd ./modules && $(MAKE) $@
	@cd ./src && $(MAKE) $@

.PHONY:
	default
