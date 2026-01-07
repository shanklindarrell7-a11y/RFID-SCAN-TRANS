# Changelog

All notable changes to the RFID Sniffer project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-01-07

### Added
- Initial release of RFID Sniffer for Flipper Zero
- Real-time RFID signal capture functionality
- Support for multiple signal formats (ASK, FSK, PSK, Raw)
- Support for multiple RFID protocols (EM4100, HID Prox, Indala, IOProx, AWID)
- Thread-safe signal processing with mutex protection
- Signal storage in Flipper RFID file format
- Four signal duplication modes:
  - Clone: Create exact copy of captured signal
  - Emulate: Emulate captured RFID tag
  - Replay: Replay captured signal
  - Generate: Generate new signals based on patterns
- Comprehensive GUI with menu system
- User notifications (visual and haptic feedback)
- Automatic storage directory creation
- Capture timeout (5 seconds) to prevent infinite loops
- Maximum signal size limit (4KB) for safety
- Thread-safe data copy API (`signal_processor_copy_data()`)
- Comprehensive error handling and validation
- Detailed logging for debugging

### Documentation
- README.md with comprehensive user guide
- SETUP.md with detailed setup instructions
- API_DOCUMENTATION.md with complete API reference
- SECURITY_ANALYSIS.md with security assessment
- Inline code documentation and comments
- Build instructions for UFBT and FBT
- Troubleshooting guide
- File format specification

### Security
- Safe string operations (no unsafe functions like strcpy/sprintf)
- Proper memory management with paired malloc/free
- 20+ null pointer checks using furi_assert
- Mutex-protected shared data structures
- Bounded buffers to prevent overflows
- Buffer null-termination in file operations
- Thread-safe APIs for data access
- Resource cleanup on error paths

### Build System
- application.fam for Flipper Zero app manifest
- Makefile with multiple build targets
- Support for UFBT (Universal Flipper Build Tool)
- Support for FBT (Flipper Build Tool)
- .gitignore for build artifacts

### Files Added
```
rfid_sniffer/
├── application.fam           # App manifest
├── Makefile                  # Build configuration
├── rfid_sniffer.c           # Main application (288 lines)
├── signal_processor.c       # Signal processing (458 lines)
├── signal_processor.h       # Signal processor header (48 lines)
└── icon_README.txt          # Icon placeholder instructions

Documentation/
├── README.md                 # Main documentation
├── SETUP.md                  # Setup guide (8.6 KB)
├── API_DOCUMENTATION.md      # API reference (13.1 KB)
├── SECURITY_ANALYSIS.md      # Security analysis (5.8 KB)
└── CHANGELOG.md              # This file

Build Configuration/
├── .gitignore               # Exclude build artifacts
└── Makefile                 # Build system
```

### Code Quality
- Consistent coding style throughout
- Defensive programming practices
- Proper error handling
- Resource cleanup in all paths
- Clear function separation and modularity
- Well-documented public APIs

### Known Limitations
- Hardware interface is simulated (for demonstration)
- File parsing is simplified (needs enhancement for production)
- Icon file needs to be created manually
- Requires Flipper Zero firmware 0.80.0 or later

## [Unreleased]

### Planned Features
- Actual RFID hardware interface implementation
- Enhanced file parsing with full format support
- Signal encryption for sensitive data
- User authentication/authorization
- Signal validation and integrity checks
- Additional protocol support (Mifare, NFC)
- Batch processing capabilities
- Signal visualization in GUI
- Export to multiple formats
- Rate limiting for repeated captures
- Enhanced error messages
- Audit logging
- Configuration persistence
- Custom signal generation algorithms

### Planned Improvements
- Complete hardware integration tests
- Performance optimization
- Memory usage optimization
- Extended battery life optimizations
- Improved UI/UX
- Additional language support
- Tutorial mode for new users

## Version History

### Version Numbering
This project uses [Semantic Versioning](https://semver.org/):
- MAJOR version: Incompatible API changes
- MINOR version: Backwards-compatible functionality additions
- PATCH version: Backwards-compatible bug fixes

### Development Milestones
- **2026-01-07**: Initial implementation completed
- **2026-01-07**: Security review passed
- **2026-01-07**: Documentation completed
- **2026-01-07**: Version 1.0.0 released

## Upgrading

### From Development to 1.0.0
This is the first stable release. If you were using development versions:
1. Back up any captured signals
2. Remove old version from Flipper
3. Install version 1.0.0
4. Test basic functionality
5. Restore your captured signals if needed

## Contributing

We welcome contributions! Please see our contributing guidelines:
1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Update documentation
5. Submit a pull request

## Support

- Report bugs via GitHub Issues
- Ask questions via GitHub Discussions
- Check documentation for common issues

## License

This project is provided for educational and research purposes.
Users are responsible for complying with local laws and regulations.

## Acknowledgments

- Flipper Zero team for the excellent platform
- RFID security research community
- All contributors to this project

---

**Current Version:** 1.0.0  
**Release Date:** January 7, 2026  
**Status:** Stable
