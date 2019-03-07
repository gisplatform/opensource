all: gio_poller poller

gio_poller: gio_poller.vala
	valac --pkg=posix --pkg=gio-unix-2.0 --vapidir=../../vapi --pkg=dualit gio_poller.vala

poller: poller.c
	gcc -Wall -g poller.c -o poller

clean:
	rm poller
	rm gio_poller

