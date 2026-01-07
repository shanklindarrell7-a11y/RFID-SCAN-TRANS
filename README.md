# RFID-SCAN-TRANS

Flipper Zero app for RFID sniffing with comprehensive capture, store, analyze, and transmit capabilities.

## 🔄 Usage Workflow

### 1. **Capture** → **Store** → **Analyze** → **Transmit/Duplicate**

The complete RFID workflow follows these sequential steps:

1. **Capture**: Read RFID tags using the Flipper Zero device
2. **Store**: Save captured tags in the library for future use
3. **Analyze**: Examine tag properties, security features, and compatibility
4. **Transmit/Duplicate**: Clone, emulate, replay, or generate RFID signals

### 2. **Signal Chain**

```
RFID Tag → Capture → Library → [Clone/Emulate/Replay/Generate]
```

- **RFID Tag**: Physical RFID/NFC tag to read
- **Capture**: Module for reading and capturing tag data
- **Library**: Storage system for managing captured tags
- **Clone**: Duplicate tag to a blank card
- **Emulate**: Simulate tag using Flipper Zero
- **Replay**: Retransmit captured signals
- **Generate**: Create new tags with custom parameters

### 3. **Export Options**

```
Library → Export → [.sub / .csv / .wiegand / .pm3 / .json]
```

Export your captured tags in multiple formats:

- **`.sub`**: Flipper Zero SubGHz format
- **`.csv`**: Comma-separated values for spreadsheets
- **`.wiegand`**: Wiegand protocol format
- **`.pm3`**: Proxmark3 compatible format
- **`.json`**: JSON format for programmatic access

## 🚀 Installation

```bash
npm install
```

## 📖 Usage

### Command Line Interface

Run the demo application:

```bash
npm start
```

### Programmatic Usage

```javascript
const RFIDScanTrans = require('./src/index');

// Initialize the application
const app = new RFIDScanTrans();

// 1. CAPTURE and STORE
const result = app.captureAndStore({
  rawData: '1234567890ABCDEF',
  protocol: 'EM4100',
  frequency: '125kHz',
  uid: '1234567890AB',
  signalStrength: 85
});

console.log(`Tag captured: ${result.tag.id}`);

// 2. ANALYZE
const analysis = app.analyzeTag(result.tag.id);
console.log(`Protocol: ${analysis.analysis.analysis.protocol.name}`);
console.log(`Security: ${analysis.analysis.analysis.security.securityLevel}`);
console.log(`Cloneable: ${analysis.analysis.analysis.compatibility.canClone}`);

// 3. TRANSMIT - Clone
const cloned = app.cloneTag(result.tag.id);
console.log(cloned.message);

// 3. TRANSMIT - Emulate
const emulated = app.emulateTag(result.tag.id);
console.log(emulated.message);

// 3. TRANSMIT - Replay
const replayed = app.replayTag(result.tag.id, 3);
console.log(replayed.message);

// 3. TRANSMIT - Generate
const generated = app.generateTag({
  protocol: 'HID',
  frequency: '125kHz'
});
console.log(`Generated: ${generated.tag.uid}`);

// 4. EXPORT
const exported = app.exportTags('all', 'json');
console.log(exported.data);

// Export to other formats
app.exportTags('all', 'csv');
app.exportTags('all', 'sub');
app.exportTags('all', 'wiegand');
app.exportTags('all', 'pm3');

// Library operations
const allTags = app.getAllTags();
const stats = app.getStatistics();
const searchResults = app.searchLibrary({ protocol: 'EM4100' });
```

## 🧪 Testing

Run the test suite:

```bash
npm test
```

## 📦 Module Structure

### Core Modules

1. **RFIDCapture** (`src/capture/RFIDCapture.js`)
   - Capture RFID tag data
   - Extract UID and protocol information
   - Track captured tags

2. **RFIDLibrary** (`src/library/RFIDLibrary.js`)
   - Store and manage RFID tags
   - Search and retrieve tags
   - Library management operations

3. **RFIDAnalyzer** (`src/analysis/RFIDAnalyzer.js`)
   - Analyze tag protocols and security
   - Check compatibility for cloning/emulation
   - Generate statistics

4. **RFIDTransmission** (`src/transmission/RFIDTransmission.js`)
   - Clone tags to blank cards
   - Emulate tags using device
   - Replay captured signals
   - Generate new tags

5. **RFIDExporter** (`src/export/RFIDExporter.js`)
   - Export to multiple formats
   - Format-specific conversion
   - Batch export support

## 🔧 Supported Protocols

- **EM4100**: Low frequency, read-only, easily cloneable
- **HID**: Low frequency proximity cards
- **Mifare**: High frequency, writable, authenticated
- **NTAG**: High frequency NFC tags
- And more...

## 📝 License

MIT 
