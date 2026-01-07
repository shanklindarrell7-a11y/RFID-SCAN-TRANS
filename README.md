# RFID-SCAN-TRANS

Flipper Zero app for RFID sniffing with advanced signal storage and transmission capabilities.

## Overview

RFID-SCAN-TRANS is a comprehensive RFID signal management system that provides professional-grade features for capturing, storing, analyzing, and transmitting RFID signals. Designed for security researchers, penetration testers, and RFID enthusiasts.

## Key Features

### 1. Signal Storage System
- **Signal Library**: Comprehensive database of captured signals with rich metadata
- **Auto-save**: Automatically save captures to SD card
- **Backup System**: Automatic backups on exit with restore capability
- **Search & Filter**: Find signals by name, protocol, or UID
- **Favorites**: Mark important signals for quick access

### 2. Multi-Format Export
- **Flipper Format**: `.sub` files compatible with Flipper Zero apps
- **CSV Database**: Spreadsheet-friendly format for analysis
- **Wiegand Format**: Industry-standard Wiegand output
- **Proxmark3 Commands**: Ready-to-use Proxmark3 scripts
- **JSON Metadata**: Structured data for integration

### 3. Duplication Modes
- **Clone Mode**: Write to physical writable RFID cards (T55xx, EM4305, etc.)
- **Emulate Mode**: Continuous broadcast simulating card presence
- **Replay Mode**: One-time transmission of captured signal
- **Generate Mode**: Create variants with incrementing/decrementing IDs

### 4. Transmission Control
- **Power Levels**: 8 configurable levels (0-7) for range control
- **Repeat Control**: Configurable repeat counts for reliability
- **Timing Control**: Precise delay between transmissions
- **Continuous Mode**: Infinite broadcast for emulation scenarios
- **Real-time Monitoring**: Visual feedback during transmission

### 5. Professional Features
- **Signal Validation**: Check signal integrity and quality
- **Auto-classification**: Intelligent protocol detection
- **Batch Operations**: Export/validate multiple signals at once
- **Encryption Support**: Basic signal obfuscation for security
- **Metadata Management**: Notes, timestamps, signal strength tracking

## Supported Protocols

### Low Frequency (125kHz)
- EM410x / EM4100
- HID Prox
- T55xx (read/write)
- Indala
- AWID

### High Frequency (13.56MHz)
- Mifare Classic
- Mifare DESFire
- iClass

## Installation

```bash
npm install
```

## Quick Start

Run the demo to see all features in action:

```bash
npm run demo
```

## Usage

### Basic Usage

```javascript
const RFIDScanTrans = require('./src/index');

const app = new RFIDScanTrans();

// Capture a signal
const capture = app.captureSignal({
  name: 'Office Badge',
  uid: '1234567890',
  frequency: 125000,
  modulation: 'ASK'
});

console.log('Captured signal:', capture.signalId);
console.log('Detected protocol:', capture.classification.detectedProtocol);
```

### Signal Storage

```javascript
// Search for signals
const results = app.searchSignals({ protocol: 'EM410x' });

// Mark as favorite
app.toggleFavorite(signalId);

// Get favorites
const favorites = app.getFavorites();
```

### Export Signals

```javascript
// Export to Flipper format
const flipperExport = app.exportSignal(signalId, 'flipper');

// Batch export multiple formats
const batchExport = app.batchExport(
  [signalId1, signalId2], 
  ['flipper', 'csv', 'json']
);
```

### Duplication Modes

```javascript
// Clone to writable card
app.cloneSignal(signalId);

// Emulate continuously
const controller = app.emulateSignal(signalId);
controller.start();
// ... later
controller.stop();

// Replay signal
app.replaySignal(signalId, { 
  repeatCount: 3, 
  delay: 1000 
});

// Generate variants
app.generateVariants(signalId, { 
  count: 10, 
  mode: 'increment' 
});
```

### Transmission Control

```javascript
// Set power level (0-7)
app.setPowerLevel(6);

// Get power level info
const info = app.getPowerLevel();
console.log(`Power: ${info.percentage}%, Range: ${info.range}cm`);

// Get statistics
const stats = app.getTransmissionStats();
console.log(`Success rate: ${stats.successRate}%`);
```

### Professional Features

```javascript
// Validate signal
const validation = app.validateSignal(signalId);
console.log('Valid:', validation.isValid);
console.log('Score:', validation.score);

// Auto-classify
const classification = app.classifySignal(signalId);
console.log('Protocol:', classification.detectedProtocol);
console.log('Confidence:', classification.confidence);

// Backup and restore
const backup = app.createBackup();
// ... later
app.restoreBackup(backup);
```

## Testing

Run the test suite:

```bash
npm test
```

## API Reference

### RFIDScanTrans

Main application class that integrates all features.

#### Methods

- `captureSignal(data)` - Capture and store a new signal
- `searchSignals(criteria)` - Search for signals
- `exportSignal(id, format)` - Export signal in specified format
- `batchExport(ids, formats)` - Export multiple signals
- `cloneSignal(id, options)` - Clone to writable card
- `emulateSignal(id, options)` - Start continuous emulation
- `replaySignal(id, options)` - Replay signal once or multiple times
- `generateVariants(id, options)` - Generate signal variants
- `setPowerLevel(level)` - Set transmission power (0-7)
- `getPowerLevel()` - Get current power level info
- `toggleFavorite(id)` - Toggle favorite status
- `getFavorites()` - Get all favorite signals
- `validateSignal(id)` - Validate signal integrity
- `classifySignal(id)` - Auto-classify protocol
- `createBackup()` - Create backup of all signals
- `restoreBackup(data)` - Restore from backup
- `getTransmissionStats()` - Get transmission statistics
- `getInfo()` - Get application information

## Security Considerations

This tool is intended for educational and authorized security testing purposes only. Always ensure you have explicit permission before capturing, cloning, or transmitting RFID signals.

### Legal Notice

- Only use on systems you own or have explicit authorization to test
- Unauthorized access to RFID systems may be illegal in your jurisdiction
- Be aware of local regulations regarding RF transmission

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT

## Acknowledgments

Built for the Flipper Zero community and RFID security researchers. 
