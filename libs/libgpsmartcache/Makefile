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

pot: config
	@$(MAKE) -C build libgpsmartcache-pot

test:
	@cd build && $(MAKE) test ARGS="-V"

.PHONY: doc
