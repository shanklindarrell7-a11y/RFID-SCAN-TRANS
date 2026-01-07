/**
 * Tests for Signal Exporter
 */

const SignalExporter = require('../src/signal-exporter');

describe('SignalExporter', () => {
  let exporter;
  let testSignal;

  beforeEach(() => {
    exporter = new SignalExporter();
    testSignal = {
      id: 'signal_123',
      name: 'Test Signal',
      protocol: 'EM410x',
      uid: '1234567890',
      frequency: 125000,
      modulation: 'ASK',
      bitrate: 64,
      timestamp: '2026-01-07T09:00:00.000Z',
      signalStrength: -50,
      notes: 'Test notes',
      metadata: {}
    };
  });

  test('should export to Flipper format', () => {
    const result = exporter.exportToFlipper(testSignal);
    
    expect(result).toContain('Filetype: Flipper SubGhz RAW File');
    expect(result).toContain('Frequency: 125000');
    expect(result).toContain('Protocol: EM410x');
  });

  test('should export to CSV format', () => {
    const result = exporter.exportToCSV(testSignal);
    
    expect(result).toContain('ID,Name,Protocol,UID');
    expect(result).toContain('signal_123');
    expect(result).toContain('Test Signal');
    expect(result).toContain('EM410x');
  });

  test('should export multiple signals to CSV', () => {
    const signals = [
      { ...testSignal, id: 'sig1', name: 'Signal 1' },
      { ...testSignal, id: 'sig2', name: 'Signal 2' }
    ];

    const result = exporter.exportMultipleToCSV(signals);
    const lines = result.split('\n');
    
    expect(lines.length).toBe(3); // header + 2 signals
    expect(result).toContain('Signal 1');
    expect(result).toContain('Signal 2');
  });

  test('should export to Wiegand format', () => {
    const result = exporter.exportToWiegand(testSignal);
    
    expect(result).toContain('Wiegand Format Export');
    expect(result).toContain('Facility Code:');
    expect(result).toContain('Card Number:');
  });

  test('should export to Proxmark3 format', () => {
    const result = exporter.exportToProxmark3(testSignal);
    
    expect(result).toContain('Proxmark3 Commands');
    expect(result).toContain('Read command:');
    expect(result).toContain('Clone command:');
    expect(result).toContain('Simulate command:');
  });

  test('should export to JSON format', () => {
    const result = exporter.exportToJSON(testSignal);
    const parsed = JSON.parse(result);
    
    expect(parsed.version).toBe('1.0.0');
    expect(parsed.signal.name).toBe('Test Signal');
    expect(parsed.signal.protocol).toBe('EM410x');
  });

  test('should export multiple signals to JSON', () => {
    const signals = [
      { ...testSignal, id: 'sig1' },
      { ...testSignal, id: 'sig2' }
    ];

    const result = exporter.exportMultipleToJSON(signals);
    const parsed = JSON.parse(result);
    
    expect(parsed.count).toBe(2);
    expect(parsed.signals.length).toBe(2);
  });

  test('should batch export in multiple formats', () => {
    const signals = [testSignal];
    const formats = ['flipper', 'csv', 'json'];

    const result = exporter.batchExport(signals, formats);
    
    expect(result.flipper).toBeDefined();
    expect(result.csv).toBeDefined();
    expect(result.json).toBeDefined();
  });
});
