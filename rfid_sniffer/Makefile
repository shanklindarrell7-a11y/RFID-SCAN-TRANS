# RFID Sniffer Makefile
# For Flipper Zero Application

# Application name
APP_NAME = rfid_sniffer

# Source files
SOURCES = rfid_sniffer.c signal_processor.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Compiler and flags
CC = arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -O2 -g -std=gnu11
LDFLAGS = -lm

# Flipper Zero SDK path (customize as needed)
SDK_PATH ?= /path/to/flipperzero-firmware

# Include directories
INCLUDES = -I. -I$(SDK_PATH)/furi -I$(SDK_PATH)/lib

# Build target
all: $(APP_NAME).fap

# Compile C files to object files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link object files to create FAP
$(APP_NAME).fap: $(OBJECTS)
	@echo "Linking $(APP_NAME).fap..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(APP_NAME).fap

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(APP_NAME).fap

# Install to Flipper (requires mounted SD card or qFlipper)
install: $(APP_NAME).fap
	@echo "Installing to Flipper..."
	@if [ -d "/media/sdcard" ]; then \
		cp $(APP_NAME).fap /media/sdcard/apps/RFID/; \
		echo "Installed to SD card"; \
	else \
		echo "SD card not found. Please copy manually."; \
	fi

# Build using Flipper Build Tool (FBT) - recommended method
fbt-build:
	@echo "Building with Flipper Build Tool..."
	@cd $(SDK_PATH) && ./fbt fap_rfid_sniffer

# Build using UFBT (Universal Flipper Build Tool) - recommended for external apps
ufbt-build:
	@echo "Building with UFBT..."
	ufbt build

ufbt-launch:
	@echo "Launching on Flipper..."
	ufbt launch

# Help target
help:
	@echo "RFID Sniffer Makefile"
	@echo "====================="
	@echo "Targets:"
	@echo "  all          - Build the application (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install to Flipper SD card"
	@echo "  fbt-build    - Build using Flipper Build Tool"
	@echo "  ufbt-build   - Build using UFBT (recommended)"
	@echo "  ufbt-launch  - Build and launch on Flipper"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Note: For building FAP files, use FBT or UFBT:"
	@echo "  - FBT: Place in firmware apps/ directory and run './fbt fap_rfid_sniffer'"
	@echo "  - UFBT: Run 'ufbt build' in this directory"

.PHONY: all clean install fbt-build ufbt-build ufbt-launch help
