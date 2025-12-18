# Makefile for librecovery_network

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -I./include
LDFLAGS = -lcurl -lm

# Check if libcurl is available
CURL_AVAILABLE := $(shell pkg-config --exists libcurl && echo yes || echo no)
ifeq ($(CURL_AVAILABLE),yes)
    CFLAGS += $(shell pkg-config --cflags libcurl)
    LDFLAGS = $(shell pkg-config --libs libcurl) -lm
else
    $(warning libcurl not found - network library will have limited functionality)
endif

# Source files
SRCS = src/recovery_network.c
OBJS = $(SRCS:.c=.o)

# Output libraries
STATIC_LIB = librecovery_network.a
SHARED_LIB = librecovery_network.so

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
	install -m 644 include/recovery_network.h $(DESTDIR)/usr/local/include/recovery/
	ldconfig

uninstall:
	rm -f $(DESTDIR)/usr/local/lib/$(STATIC_LIB)
	rm -f $(DESTDIR)/usr/local/lib/$(SHARED_LIB)
	rm -f $(DESTDIR)/usr/local/include/recovery/recovery_network.h
	ldconfig