# Makefile for librecovery_signal

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -I./include -I../recovery_core/include
LDFLAGS = -L../recovery_core -lrecovery_core -lm

# Source files
SRCS = src/recovery_signal.c
OBJS = $(SRCS:.c=.o)

# Output libraries
STATIC_LIB = librecovery_signal.a
SHARED_LIB = librecovery_signal.so

# Targets
.PHONY: all clean static shared

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $^

$(SHARED_LIB): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(STATIC_LIB) $(SHARED_LIB)

install: all
	install -d $(DESTDIR)/usr/local/lib
	install -d $(DESTDIR)/usr/local/include/recovery
	install -m 644 $(STATIC_LIB) $(DESTDIR)/usr/local/lib/
	install -m 755 $(SHARED_LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 include/recovery_signal.h $(DESTDIR)/usr/local/include/recovery/
	ldconfig

uninstall:
	rm -f $(DESTDIR)/usr/local/lib/$(STATIC_LIB)
	rm -f $(DESTDIR)/usr/local/lib/$(SHARED_LIB)
	rm -f $(DESTDIR)/usr/local/include/recovery/recovery_signal.h
	ldconfig