CC:=clang
SRCDIR:=src
BUILDDIR:=build
BIN:=bin
TARGET:=con

SOURCES:=$(shell find $(SRCDIR) -type f -name *.c)
OBJECTS:=$(SOURCES:.c=.o)
OBJECTS:=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))

DEPS:=glib-2.0
DEPS_INCLUDE:=$(shell pkg-config --cflags-only-I $(DEPS))
DEPS_LIBFLAGS:=$(shell pkg-config --libs $(DEPS))

INCLUDE := -I include $(DEPS_INCLUDE)
LIB := -ledit $(DEPS_LIBFLAGS)

CFLAGS:=-c -Wall -std=c11

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(BIN)
	$(CC) $^ -o $(BIN)/$(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<

clean:
	@echo "Cleaning..."
	rm -r $(BUILDDIR) $(BIN)

valgrind: $(TARGET)
	G_SLICE=always-malloc valgrind \
		--suppressions=glib.supp --leak-check=full $(BIN)/$(TARGET)

.PHONY: clean
