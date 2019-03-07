all: config
	@cd build && $(MAKE)

config:
	@install -d build
	@cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) ..

clean:
	@echo "Cleaning build directory"
	@-$(MAKE) -C build clean
	@rm -rf build

distclean: clean

test:
	@cd build && $(MAKE) test ARGS="-V"

