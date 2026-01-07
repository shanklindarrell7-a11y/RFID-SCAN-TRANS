# Contributing to RFID-SCAN-TRANS

Thank you for your interest in contributing to RFID-SCAN-TRANS! This document provides guidelines and instructions for contributing.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR-USERNAME/RFID-SCAN-TRANS.git
   cd RFID-SCAN-TRANS
   ```
3. **Set up development environment**:
   - Install Flipper Zero SDK
   - Install required build tools (gcc, make)

## Development Workflow

### Creating a Feature Branch

Always create a new branch for your work:

```bash
git checkout -b feature/your-feature-name
```

Branch naming conventions:
- `feature/` - New features
- `bugfix/` - Bug fixes
- `docs/` - Documentation updates
- `refactor/` - Code refactoring

### Making Changes

1. **Write clean code**:
   - Follow the existing code style
   - Use meaningful variable and function names
   - Add comments for complex logic

2. **Test your changes**:
   - Build the application: `make`
   - Test on actual hardware if possible
   - Verify no existing functionality breaks

3. **Add tests** for new features:
   - Add unit tests if applicable
   - Document test cases

### Code Style Guidelines

- **Indentation**: 4 spaces (no tabs)
- **Line length**: Maximum 100 characters
- **Braces**: K&R style
- **Naming**:
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`
  - Types: `PascalCase`

Example:
```c
static bool process_signal_data(const uint8_t* data, size_t size) {
    if(!data || size == 0) {
        return false;
    }
    
    // Process data
    for(size_t i = 0; i < size; i++) {
        // ...
    }
    
    return true;
}
```

### Adding New Protocols

To add support for a new RFID protocol:

1. **Add protocol type** to `rfid_scan_trans.h`:
   ```c
   typedef enum {
       PROTOCOL_UNKNOWN,
       PROTOCOL_EM4100,
       PROTOCOL_HID,
       PROTOCOL_INDALA,
       PROTOCOL_YOUR_PROTOCOL,  // Add here
       PROTOCOL_RAW,
   } ProtocolType;
   ```

2. **Implement detector** in `signal_processor.c`:
   ```c
   static bool detect_your_protocol(const uint8_t* samples, size_t sample_count) {
       // Implement detection logic
       // Return true if samples match your protocol
       return false;
   }
   ```

3. **Implement decoder** in `signal_processor.c`:
   ```c
   static bool decode_your_protocol(const uint8_t* samples, size_t sample_count, SignalData* signal) {
       if(!samples || !signal) return false;
       
       signal->type = PROTOCOL_YOUR_PROTOCOL;
       strncpy(signal->name, "Your Protocol", sizeof(signal->name) - 1);
       
       // Decode samples into signal->data
       // Set signal->data_size and signal->frequency
       
       return true;
   }
   ```

4. **Register handler** in `protocol_handlers` array:
   ```c
   static const ProtocolHandler protocol_handlers[] = {
       // ... existing handlers ...
       {
           .name = "Your Protocol",
           .detector = detect_your_protocol,
           .decoder = decode_your_protocol,
       },
   };
   ```

5. **Test thoroughly**:
   - Test detection with known samples
   - Test decoding accuracy
   - Verify export works correctly

### Commit Guidelines

Write clear, descriptive commit messages:

```
Short (50 chars or less) summary

More detailed explanatory text, if necessary. Wrap it to about 72
characters. The blank line separating the summary from the body is
critical.

- Bullet points are okay
- Use imperative mood: "Add feature" not "Added feature"
- Reference issues: "Fixes #123"
```

Examples:
- `Add support for AWID protocol detection`
- `Fix buffer overflow in signal_processor`
- `Update documentation for protocol extension`

### Submitting a Pull Request

1. **Ensure your code builds**:
   ```bash
   make clean
   make
   ```

2. **Update documentation** if needed:
   - Update README.md
   - Add comments to code
   - Update CONTRIBUTING.md if adding new processes

3. **Commit and push** your changes:
   ```bash
   git add .
   git commit -m "Your descriptive commit message"
   git push origin feature/your-feature-name
   ```

4. **Create Pull Request**:
   - Go to the repository on GitHub
   - Click "New Pull Request"
   - Select your feature branch
   - Fill in the PR template with:
     - Description of changes
     - Testing performed
     - Screenshots (if applicable)
     - Related issues

5. **Respond to feedback**:
   - Address review comments
   - Push additional commits as needed
   - Keep discussion professional and constructive

## Testing

### Manual Testing

1. Build the application
2. Deploy to Flipper Zero
3. Test all affected functionality
4. Verify existing features still work

### Test Cases to Consider

- Protocol detection with valid signals
- Protocol detection with invalid signals
- Library add/remove operations
- Export to all formats
- Edge cases (empty library, full library, etc.)

## Documentation

When adding features, update:

- Code comments
- README.md
- This CONTRIBUTING.md
- Inline documentation in headers

## Questions?

If you have questions:

1. Check existing issues
2. Review documentation
3. Open a new issue with the `question` label

## Code of Conduct

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Help others learn and grow

## Recognition

Contributors will be acknowledged in:
- README.md contributors section
- Release notes
- Commit history

Thank you for contributing to RFID-SCAN-TRANS!
