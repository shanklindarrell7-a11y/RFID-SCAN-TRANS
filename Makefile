# RFID Scan Trans Makefile
# For Flipper Zero application

# Application name
APP_NAME = rfid_scan_trans

# Source files
SRC_DIR = src
INCLUDE_DIR = include
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Compiler and flags
CC = gcc
CFLAGS = -I$(INCLUDE_DIR) -Wall -Wextra -std=c11 -O2
LDFLAGS = 

# Installation directory
INSTALL_DIR = /ext/apps/Sub-GHz

# Flipper build tool (if available)
FBT = fbt

# Default target
.PHONY: all
all: check_environment build

# Check if we're in a Flipper build environment
.PHONY: check_environment
check_environment:
	@echo "Checking build environment..."
	@if command -v $(FBT) > /dev/null 2>&1; then \
		echo "Flipper Build Tool found"; \
	else \
		echo "Warning: Flipper Build Tool (fbt) not found"; \
		echo "For Flipper Zero, please use: fbt fap_$(APP_NAME)"; \
	fi

# Build using Flipper Build Tool if available, otherwise standard build
.PHONY: build
build:
	@echo "Building $(APP_NAME)..."
	@if command -v $(FBT) > /dev/null 2>&1; then \
		echo "Building with Flipper Build Tool..."; \
		$(FBT) fap_$(APP_NAME); \
	else \
		echo "Building standalone (for testing only)..."; \
		$(MAKE) standalone; \
	fi

# Standalone build (for testing without Flipper SDK)
.PHONY: standalone
standalone: $(OBJECTS)
	@echo "Linking $(APP_NAME)..."
	$(CC) $(OBJECTS) -o $(APP_NAME) $(LDFLAGS)
	@echo "Build complete (standalone mode)"

# Compile source files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Install to Flipper
.PHONY: install
install:
	@echo "Installing $(APP_NAME)..."
	@if command -v $(FBT) > /dev/null 2>&1; then \
		echo "Installing with Flipper Build Tool..."; \
		$(FBT) fap_$(APP_NAME) fap_deploy; \
	else \
		echo "Error: Cannot install without Flipper Build Tool"; \
		echo "Please install Flipper Zero SDK and run: fbt fap_$(APP_NAME) fap_deploy"; \
		exit 1; \
	fi

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(APP_NAME)
	rm -rf build dist .fbt
	@if command -v $(FBT) > /dev/null 2>&1; then \
		$(FBT) clean; \
	fi
	@echo "Clean complete"

# Format code
.PHONY: format
format:
	@echo "Formatting code..."
	@if command -v clang-format > /dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i; \
		echo "Format complete"; \
	else \
		echo "clang-format not found, skipping..."; \
	fi

# Run linter
.PHONY: lint
lint:
	@echo "Running linter..."
	@if command -v clang-tidy > /dev/null 2>&1; then \
		clang-tidy $(SOURCES) -- $(CFLAGS); \
		echo "Lint complete"; \
	else \
		echo "clang-tidy not found, skipping..."; \
	fi

# Show help
.PHONY: help
help:
	@echo "RFID Scan Trans - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build the application"
	@echo "  make install      - Install to Flipper Zero"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make format       - Format source code"
	@echo "  make lint         - Run code linter"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Dependencies:"
	@echo "  - Flipper Zero SDK (fbt)"
	@echo "  - SubGHz library"
	@echo "  - GUI components"
	@echo "  - File system access"
	@echo ""
	@echo "For Flipper Zero development:"
	@echo "  1. Clone Flipper Zero firmware repository"
	@echo "  2. Place this project in applications_user/"
	@echo "  3. Run: ./fbt fap_$(APP_NAME)"
	@echo "  4. Deploy: ./fbt fap_$(APP_NAME) fap_deploy"

.DEFAULT_GOAL := help
