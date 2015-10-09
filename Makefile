CC:=clang
SRCDIR:=src
BUILDDIR:=build
BIN:=bin
TARGET:=con

SOURCES:=$(shell find $(SRCDIR) -type f -name *.c)
OBJECTS:=$(SOURCES:.c=.o)
OBJECTS:=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
LIB := -ledit
INCLUDE:=-I include

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

.PHONY: clean
