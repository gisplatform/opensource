all: config
	@cd build && $(MAKE)

config:
	@install -d build
ifdef TOOLCHAIN
	@cd build && PKG_CONFIG_LIBDIR="$(TOOLCHAIN)/usr/lib/pkgconfig/" cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) -D CMAKE_TOOLCHAIN_FILE="$(TOOLCHAIN)/toolchain-file" ..
else
	@cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) ..
endif

doc:
	@cd doc && doxygen

clean:
	@echo "Cleaning build directory"
	@-$(MAKE) -C build clean
	@rm -rf build doc/documentation

distclean: clean

.PHONY: doc
