# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3 -pthread -march=native -flto
LDFLAGS = -lsqlite3 -lpthread -lm

# Source files
SOURCES = rfid_sniffer.c
HEADERS = rfid_sniffer.h
OBJECTS = rfid_sniffer.o

# Example application
EXAMPLE_SRC = example.c
EXAMPLE_BIN = rfid_example

# Library
LIB_NAME = librfid_sniffer.a

# Targets
.PHONY: all clean example library test install

all: library example

# Compile library
rfid_sniffer.o: rfid_sniffer.c rfid_sniffer.h
	$(CC) $(CFLAGS) -c rfid_sniffer.c -o rfid_sniffer.o

# Create static library
library: $(OBJECTS)
	ar rcs $(LIB_NAME) $(OBJECTS)
	ranlib $(LIB_NAME)
	@echo "Static library $(LIB_NAME) created successfully"

# Build example application
example: library
	$(CC) $(CFLAGS) $(EXAMPLE_SRC) -L. -lrfid_sniffer $(LDFLAGS) -o $(EXAMPLE_BIN)
	@echo "Example application $(EXAMPLE_BIN) built successfully"

# Run tests
test: example
	@echo "Running performance tests..."
	./$(EXAMPLE_BIN)

# Install library and headers (optional)
install: library
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	cp $(LIB_NAME) /usr/local/lib/
	cp $(HEADERS) /usr/local/include/
	@echo "Library installed to /usr/local/lib"
	@echo "Headers installed to /usr/local/include"

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(LIB_NAME) $(EXAMPLE_BIN)
	rm -f test_rfid.db test_rfid.db-wal test_rfid.db-shm
	rm -f export_*.json export_*.csv export_*.bin
	@echo "Cleaned build artifacts and test files"

# Help target
help:
	@echo "RFID Sniffer Build System"
	@echo "=========================="
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build library and example application (default)"
	@echo "  library    - Build static library only"
	@echo "  example    - Build example application"
	@echo "  test       - Run performance tests"
	@echo "  install    - Install library and headers to /usr/local"
	@echo "  clean      - Remove build artifacts and test files"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - GCC compiler"
	@echo "  - SQLite3 development library"
	@echo "  - pthread library"
