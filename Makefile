
DESTDIR  = 
PREFIX   = /usr/local
LIBDIR   = lib

CC       = gcc
CFLAGS   = -Iinclude -std=c99 -O3

OBJS     = src/config.o src/core.o src/hooks.o src/log.o
NATIVE   = libX11.so libGL.so

.PHONY: all clean install
all: yukon-core

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(NATIVE):
	$(CC) -shared -o $@ -Wl,-soname,$@.native

yukon-core: $(OBJS) $(NATIVE)
	$(CC) -shared -o $@ $(OBJS) -ldl -lseom -L. $(NATIVE)

soname = $$(objdump -x /usr/$(LIBDIR)/$(1) | grep SONAME | awk '{ print $$2 }')
install: yukon-core
	install -m 755 -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon
	install -m 755 tools/yukon $(DESTDIR)$(PREFIX)/bin
	install -m 755 yukon-core $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon

	$(foreach lib,$(NATIVE),ln -sf /usr/$(LIBDIR)/$(lib) $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon/$(lib).native;)
	$(foreach lib,$(NATIVE),ln -sf yukon-core $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon/$(call soname,$(lib));)

clean:
	rm -f $(OBJS) $(NATIVE) yukon-core
