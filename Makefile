
DESTDIR  = 
PREFIX   = /usr/local
LIBDIR   = lib

CC       = gcc
CFLAGS   = -Iinclude -Wall -std=c99 -O3

OBJS     = src/core/conf.o src/core/glue.o src/core/log.o
LIBS     = libX11.so libGL.so

-include config.make

.PHONY: all clean install
all: $(LIBS) yukon-core-lib sysconf

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(LIBS):
	$(CC) -shared -o $@.native -Wl,-soname,$@.native
	$(CC) $(CFLAGS) -fPIC -shared -o $@ src/libs/$(@:%.so=%.c) $@.native
	rm -f $@.native

yukon-core-lib: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) -lseom

sysconf:
	echo 'LDPATH="$(PREFIX)/$(LIBDIR)/yukon"' > $@

soname = `objdump -x /usr/$(LIBDIR)/$(1) | grep SONAME | awk '{ print $$2 }'`
install: $(LIBS) yukon-core-lib
	install -m 755 -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon
	install -m 755 src/scripts/yukon $(DESTDIR)$(PREFIX)/bin
	install -m 755 yukon-core-lib $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon

	$(foreach lib,$(LIBS),ln -sf /usr/$(LIBDIR)/$(lib) $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon/$(lib).native;)
	$(foreach lib,$(LIBS),install -m 755 $(lib) $(DESTDIR)$(PREFIX)/$(LIBDIR)/yukon/$(call soname,$(lib));)

clean:
	rm -f $(OBJS) $(LIBS) yukon-core-lib sysconf

mrproper: clean
	rm -f config.make
