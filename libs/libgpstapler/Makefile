all: config
	@cd build && $(MAKE)

config:
	@install -d build
	@cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) ..

doc:
	@cd doc && doxygen

clean:
	@echo "Cleaning build directory"
	@-$(MAKE) -C build clean
	@rm -rf build doc/documentation

distclean: clean

.PHONY: doc
