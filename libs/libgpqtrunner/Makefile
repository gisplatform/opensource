CONFIG = "Debug"



all: config
	@( cd build && $(MAKE) )

cross-mingw: config-cross-mingw
	@( cd build && $(MAKE) )

config:
	@install -d build
	@( cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) .. )

config-cross-mingw:
	@install -d build
	@cd build && PKG_CONFIG_LIBDIR=/opt/mingw/lib/pkgconfig CFLAGS=-I/opt/mingw/include cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=$(CONFIG) -D CMAKE_TOOLCHAIN_FILE=../toolchain.mingw ..

clean:
	@echo "Cleaning build directory"
	@-$(MAKE) -C build clean
	@rm -rf build doc/documentation

distclean: clean

package: config
	@( cd build && $(MAKE) package )

