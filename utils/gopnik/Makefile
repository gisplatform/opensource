all: gopnik

gopnik: config.h  error.h  gopnik.c
	gcc -Wall -g gopnik.c `pkg-config --cflags --libs glib-2.0 gio-2.0` -o gopnik

clean:
	rm gopnik

