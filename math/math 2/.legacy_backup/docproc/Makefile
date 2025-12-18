# Makefile for libdocproc - Document Processing Library

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -I. $(shell pkg-config --cflags libzip libxml-2.0)
LDFLAGS = $(shell pkg-config --libs libzip libxml-2.0)
ARFLAGS = rcs

# Source files
CORE_SOURCES = core/format_detection.c core/docproc_core.c
FORMAT_SOURCES = formats/simple_formats.c formats/cli_extractors.c \
                 formats/office_xml.c formats/odf.c formats/epub.c \
                 formats/yaml.c formats/html.c formats/email.c \
                 formats/archive.c
UTIL_SOURCES = utils/zip_utils.c utils/xml_utils.c

SOURCES = $(CORE_SOURCES) $(FORMAT_SOURCES) $(UTIL_SOURCES)
OBJECTS = $(SOURCES:.c=.o)

# Library names
SHARED_LIB = libdocproc.so
STATIC_LIB = libdocproc.a

# Default target
all: $(SHARED_LIB) $(STATIC_LIB)

# Shared library
$(SHARED_LIB): $(OBJECTS)
	@echo "Creating document processing shared library: $@"
	$(CC) -shared -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "✓ Document processing shared library created"

# Static library
$(STATIC_LIB): $(OBJECTS)
	@echo "Creating document processing static library: $@"
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo "✓ Document processing static library created"

# Compile source files
%.o: %.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)
	@echo "✓ Document processing library cleaned"

# Install
install: $(SHARED_LIB) $(STATIC_LIB)
	install -d $(DESTDIR)/usr/local/lib
	install -m 755 $(SHARED_LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 $(STATIC_LIB) $(DESTDIR)/usr/local/lib/
	install -d $(DESTDIR)/usr/local/include
	install -m 644 docproc.h $(DESTDIR)/usr/local/include/
	@echo "✓ Document processing library installed"

.PHONY: all clean install