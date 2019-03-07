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

# Экспериментальная сборка исходников под Gtk-2.
gtk2source:
	valac -D GTK2 --pkg gtk+-2.0 -C	\
		-d build/gtk2/								\
		--header=build/gtk2/gp-core.h	\
		src/gui/Dialog.vala						\
		src/gui/TreeExport.vala

pot: config
	@$(MAKE) -C build libgpcore-pot

test:
	@cd build && $(MAKE) test ARGS="-V"

.PHONY: doc
