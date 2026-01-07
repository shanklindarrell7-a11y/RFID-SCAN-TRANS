/**
 * RFID-SCAN-TRANS Demo Script
 * Demonstrates all major features of the application
 */

const RFIDScanTrans = require('./src/index');

console.log('=== RFID-SCAN-TRANS Demo ===\n');

const app = new RFIDScanTrans();

// Display app info
console.log('Application Info:');
console.log(JSON.stringify(app.getInfo(), null, 2));
console.log('\n' + '='.repeat(50) + '\n');

// 1. Capture signals
console.log('1. CAPTURING SIGNALS\n');

const signal1 = app.captureSignal({
  name: 'Office Badge',
  uid: '1234567890',
  frequency: 125000,
  modulation: 'ASK',
  signalStrength: -45,
  notes: 'Main entrance badge'
});

console.log(`✓ Captured: ${signal1.signal.name}`);
console.log(`  - Protocol: ${signal1.classification.detectedProtocol} (${signal1.classification.confidence}% confidence)`);
console.log(`  - Validation Score: ${signal1.validation.score}/100`);

const signal2 = app.captureSignal({
  name: 'Parking Card',
  uid: 'ABCD1234567890EF',
  frequency: 125000,
  modulation: 'FSK',
  signalStrength: -52,
  notes: 'Underground parking'
});

console.log(`✓ Captured: ${signal2.signal.name}`);
console.log(`  - Protocol: ${signal2.classification.detectedProtocol}`);

const signal3 = app.captureSignal({
  name: 'Access Keycard',
  uid: '12345678',
  frequency: 13560000,
  modulation: 'ASK',
  signalStrength: -38
});

console.log(`✓ Captured: ${signal3.signal.name}`);
console.log(`  - Protocol: ${signal3.classification.detectedProtocol}`);
console.log('\n' + '='.repeat(50) + '\n');

// 2. Search and favorites
console.log('2. SEARCH & FAVORITES\n');

const em410xSignals = app.searchSignals({ protocol: 'EM410x' });
console.log(`Found ${em410xSignals.length} EM410x signal(s)`);

app.toggleFavorite(signal1.signalId);
console.log(`✓ Marked "${signal1.signal.name}" as favorite`);

const favorites = app.getFavorites();
console.log(`Total favorites: ${favorites.length}`);
console.log('\n' + '='.repeat(50) + '\n');

// 3. Export signals
console.log('3. EXPORTING SIGNALS\n');

const flipperExport = app.exportSignal(signal1.signalId, 'flipper');
console.log(`✓ Exported to Flipper format: ${flipperExport.filename}`);
console.log('Preview:');
console.log(flipperExport.data.split('\n').slice(0, 5).join('\n'));
console.log('...\n');

const csvExport = app.exportSignal(signal1.signalId, 'csv');
console.log(`✓ Exported to CSV format`);

// Batch export
const batchResult = app.batchExport(
  [signal1.signalId, signal2.signalId],
  ['json']
);
console.log(`✓ Batch exported ${batchResult.count} signals to JSON`);
console.log('\n' + '='.repeat(50) + '\n');

// 4. Generate variants
console.log('4. GENERATING VARIANTS\n');

const variants = app.generateVariants(signal1.signalId, {
  count: 5,
  mode: 'increment'
});

console.log(`✓ Generated ${variants.count} variants from "${signal1.signal.name}"`);
console.log(`  Base UID: ${signal1.signal.uid}`);

const variantSignals = variants.variantIds.slice(0, 3).map(id => app.storage.getSignal(id));
variantSignals.forEach((v, i) => {
  console.log(`  Variant ${i + 1}: ${v.uid}`);
});
console.log('  ...');
console.log('\n' + '='.repeat(50) + '\n');

// 5. Transmission control
console.log('5. TRANSMISSION CONTROL\n');

console.log('Power level settings:');
for (let level = 0; level < 8; level += 2) {
  app.setPowerLevel(level);
  const info = app.getPowerLevel();
  console.log(`  Level ${level}: ${info.percentage}% power, ~${info.range}cm range - ${info.description}`);
}

app.setPowerLevel(5);
console.log(`\n✓ Set power level to 5 for demonstrations`);

// Replay signal
const replayResult = app.replaySignal(signal1.signalId, {
  repeatCount: 3,
  delay: 100
});

console.log(`✓ Replayed "${signal1.signal.name}" ${replayResult.results.length} times`);
console.log(`  Success rate: ${replayResult.successRate.toFixed(1)}%`);

console.log('\n' + '='.repeat(50) + '\n');

// 6. Professional features
console.log('6. PROFESSIONAL FEATURES\n');

// Validation
const validation = app.validateSignal(signal1.signalId);
console.log('Signal Validation:');
console.log(`  Valid: ${validation.isValid}`);
console.log(`  Score: ${validation.score}/100`);
console.log(`  Errors: ${validation.errors.length}`);
console.log(`  Warnings: ${validation.warnings.length}`);

// Classification
const classification = app.classifySignal(signal1.signalId);
console.log('\nAuto-Classification:');
console.log(`  Detected: ${classification.detectedProtocol}`);
console.log(`  Confidence: ${classification.confidence}%`);
console.log(`  Reasoning: ${classification.reasoning.join('; ')}`);

console.log('\n' + '='.repeat(50) + '\n');

// 7. Statistics
console.log('7. STATISTICS\n');

const stats = app.getTransmissionStats();
console.log('Transmission Statistics:');
console.log(`  Total transmissions: ${stats.totalTransmissions}`);
console.log(`  Successful: ${stats.successCount}`);
console.log(`  Failed: ${stats.failureCount}`);
console.log(`  Success rate: ${stats.successRate.toFixed(1)}%`);

const info = app.getInfo();
console.log('\nSignal Library:');
console.log(`  Total signals: ${info.stats.totalSignals}`);
console.log(`  Favorites: ${info.stats.favorites}`);

console.log('\n' + '='.repeat(50) + '\n');

// 8. Backup
console.log('8. BACKUP & RESTORE\n');

const backup = app.createBackup();
console.log(`✓ Created backup with ${backup.signals.length} signal(s)`);
console.log(`  Backup timestamp: ${backup.timestamp}`);
console.log(`  Backup version: ${backup.version}`);

console.log('\n' + '='.repeat(50) + '\n');

console.log('Demo completed successfully!');
console.log('\nAll features demonstrated:');
console.log('✓ Signal capture with auto-classification');
console.log('✓ Search and favorites');
console.log('✓ Multi-format export (Flipper, CSV, JSON, Wiegand, Proxmark3)');
console.log('✓ Signal variant generation');
console.log('✓ Transmission control with power levels');
console.log('✓ Professional validation and classification');
console.log('✓ Statistics tracking');
console.log('✓ Backup and restore');
