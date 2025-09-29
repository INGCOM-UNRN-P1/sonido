CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -O2
INCLUDES = -Iinclude

# Directories
SRCDIR = src
INCLUDEDIR = include
TESTDIR = tests
EXAMPLEDIR = examples
OBJDIR = obj
LIBDIR = lib

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Library
LIBRARY = $(LIBDIR)/libsonido.a
SHARED_LIB = $(LIBDIR)/libsonido.so

# Test files
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(OBJDIR)/test_%.o)
TEST_EXECUTABLES = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTDIR)/%)

# Example files
EXAMPLE_SOURCES = $(wildcard $(EXAMPLEDIR)/*.c)
EXAMPLE_EXECUTABLES = $(EXAMPLE_SOURCES:$(EXAMPLEDIR)/%.c=$(EXAMPLEDIR)/%)

.PHONY: all clean test examples install lib shared

all: lib examples test

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
$(OBJDIR)/test_%.o: $(TESTDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create static library
lib: $(LIBRARY)

$(LIBRARY): $(OBJECTS) | $(LIBDIR)
	ar rcs $@ $^

# Create shared library
shared: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) | $(LIBDIR)
	$(CC) -shared -o $@ $^

# Build tests
test: $(TEST_EXECUTABLES)

$(TESTDIR)/%: $(OBJDIR)/test_%.o $(LIBRARY)
	$(CC) $(CFLAGS) $< -L$(LIBDIR) -lsonido -lm -o $@

# Build examples
examples: $(EXAMPLE_EXECUTABLES)

$(EXAMPLEDIR)/%: $(EXAMPLEDIR)/%.c $(LIBRARY)
	$(CC) $(CFLAGS) $(INCLUDES) $< -L$(LIBDIR) -lsonido -lm -o $@

# Run tests
check: test
	@echo "Running tests..."
	@for test in $(TEST_EXECUTABLES); do \
		echo "Running $$test..."; \
		$$test; \
	done

# Install library
install: $(LIBRARY)
	install -d /usr/local/lib
	install -d /usr/local/include
	install -m 644 $(LIBRARY) /usr/local/lib/
	install -m 644 $(INCLUDEDIR)/*.h /usr/local/include/

# Clean
clean:
	rm -rf $(OBJDIR) $(LIBDIR)
	rm -f $(TEST_EXECUTABLES) $(EXAMPLE_EXECUTABLES)

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build library, examples and tests"
	@echo "  lib       - Build static library"
	@echo "  shared    - Build shared library"
	@echo "  test      - Build tests"
	@echo "  examples  - Build examples"
	@echo "  check     - Run tests"
	@echo "  install   - Install library system-wide"
	@echo "  clean     - Clean build artifacts"
	@echo "  help      - Show this help message"