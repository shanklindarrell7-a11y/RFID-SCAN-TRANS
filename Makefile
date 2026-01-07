# Makefile for RFID-SCAN-TRANS
# This is a documentation Makefile for reference

.PHONY: help build flash validate clean

help:
	@echo "RFID-SCAN-TRANS Build Commands"
	@echo "================================"
	@echo ""
	@echo "Build commands (requires Flipper Zero firmware SDK):"
	@echo "  make build    - Build the application"
	@echo "  make flash    - Build and flash to connected Flipper Zero"
	@echo ""
	@echo "Testing commands:"
	@echo "  make validate - Validate export format outputs"
	@echo ""
	@echo "Utility commands:"
	@echo "  make clean    - Clean build artifacts"
	@echo ""

build:
	@echo "Building RFID Sniffer application..."
	@echo "Note: This requires the Flipper Zero firmware build environment"
	@echo "Run this from your Flipper firmware directory:"
	@echo "  ./fbt fap_rfid_sniffer"

flash: build
	@echo "Flashing to Flipper Zero..."
	@echo "Run this from your Flipper firmware directory:"
	@echo "  ./fbt launch_app APPSRC=applications_user/rfid_sniffer"

validate:
	@echo "Validating export formats..."
	@if [ -f validate_exports.py ]; then \
		python3 validate_exports.py; \
	else \
		echo "Error: validate_exports.py not found"; \
		exit 1; \
	fi

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build/ .fap/ *.fap *.elf *.bin *.hex *.o *.d
	@echo "Clean complete"

# Documentation targets
docs:
	@echo "Documentation files:"
	@echo "  README.md           - Main documentation"
	@echo "  EXPORT_FORMATS.md   - Export format specifications"
	@echo "  export_examples.c   - Code examples"

# File structure
structure:
	@echo "Project Structure:"
	@echo "=================="
	@echo ""
	@echo "Core Files:"
	@echo "  export_formats.h      - Export format interface"
	@echo "  export_formats.c      - Export format implementation"
	@echo ""
	@echo "Example Code:"
	@echo "  export_examples.h     - Example function declarations"
	@echo "  export_examples.c     - Example implementations"
	@echo ""
	@echo "Documentation:"
	@echo "  README.md            - Main documentation"
	@echo "  EXPORT_FORMATS.md    - Format specifications"
	@echo ""
	@echo "Utilities:"
	@echo "  validate_exports.py  - Format validation script"
	@echo "  Makefile            - Build documentation"
	@echo "  .gitignore          - Git ignore rules"
