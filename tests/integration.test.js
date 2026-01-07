/**
 * Integration Tests for RFID-SCAN-TRANS
 */

const RFIDScanTrans = require('../src/index');

describe('RFIDScanTrans Integration', () => {
  let app;

  beforeEach(() => {
    app = new RFIDScanTrans();
  });

  test('should capture and store signal', () => {
    const captureData = {
      name: 'Test Badge',
      uid: '1234567890',
      frequency: 125000,
      modulation: 'ASK'
    };

    const result = app.captureSignal(captureData);
    
    expect(result.success).toBe(true);
    expect(result.signalId).toBeDefined();
    expect(result.signal).toBeDefined();
    expect(result.classification).toBeDefined();
    expect(result.validation).toBeDefined();
  });

  test('should search captured signals', () => {
    app.captureSignal({ name: 'Badge 1', protocol: 'EM410x', uid: '111' });
    app.captureSignal({ name: 'Badge 2', protocol: 'HID', uid: '222' });
    app.captureSignal({ name: 'Badge 3', protocol: 'EM410x', uid: '333' });

    const results = app.searchSignals({ protocol: 'EM410x' });
    expect(results.length).toBe(2);
  });

  test('should export signal in multiple formats', () => {
    const captureData = {
      name: 'Export Test',
      protocol: 'EM410x',
      uid: '1234567890',
      frequency: 125000
    };

    const capture = app.captureSignal(captureData);
    const signalId = capture.signalId;

    const flipperExport = app.exportSignal(signalId, 'flipper');
    expect(flipperExport.success).toBe(true);
    expect(flipperExport.data).toContain('Flipper');

    const csvExport = app.exportSignal(signalId, 'csv');
    expect(csvExport.success).toBe(true);
    expect(csvExport.data).toContain('ID,Name');

    const jsonExport = app.exportSignal(signalId, 'json');
    expect(jsonExport.success).toBe(true);
    expect(jsonExport.data).toContain('"version"');
  });

  test('should batch export multiple signals', () => {
    const id1 = app.captureSignal({ name: 'S1', protocol: 'EM410x', uid: '111' }).signalId;
    const id2 = app.captureSignal({ name: 'S2', protocol: 'HID', uid: '222' }).signalId;

    const result = app.batchExport([id1, id2], ['flipper', 'csv', 'json']);
    
    expect(result.success).toBe(true);
    expect(result.count).toBe(2);
    expect(result.exports.flipper).toBeDefined();
    expect(result.exports.csv).toBeDefined();
    expect(result.exports.json).toBeDefined();
  });

  test('should clone signal', () => {
    const captureData = {
      name: 'Clone Test',
      protocol: 'T55xx',
      uid: '1234567890',
      metadata: { writable: true }
    };

    const capture = app.captureSignal(captureData);
    const result = app.cloneSignal(capture.signalId);
    
    expect(result.mode).toBe('clone');
  });

  test('should replay signal', () => {
    const captureData = {
      name: 'Replay Test',
      protocol: 'EM410x',
      uid: '1234567890'
    };

    const capture = app.captureSignal(captureData);
    const result = app.replaySignal(capture.signalId, { repeatCount: 2 });
    
    expect(result.results).toBeDefined();
    expect(result.results.length).toBe(2);
  });

  test('should generate signal variants', () => {
    const captureData = {
      name: 'Base Signal',
      protocol: 'EM410x',
      uid: 'ABCD1234'
    };

    const capture = app.captureSignal(captureData);
    const result = app.generateVariants(capture.signalId, { count: 5, mode: 'increment' });
    
    expect(result.success).toBe(true);
    expect(result.count).toBe(5);
    expect(result.variantIds.length).toBe(5);
  });

  test('should set and get power level', () => {
    const result = app.setPowerLevel(6);
    
    expect(result.success).toBe(true);
    expect(result.currentLevel).toBe(6);
    
    const info = app.getPowerLevel();
    expect(info.level).toBe(6);
  });

  test('should toggle favorites', () => {
    const capture = app.captureSignal({ name: 'Fav', protocol: 'EM410x', uid: '123' });
    
    const result = app.toggleFavorite(capture.signalId);
    expect(result.isFavorite).toBe(true);

    const favorites = app.getFavorites();
    expect(favorites.length).toBe(1);
  });

  test('should validate signal', () => {
    const capture = app.captureSignal({ 
      name: 'Valid', 
      protocol: 'EM410x', 
      uid: '1234567890' 
    });

    const validation = app.validateSignal(capture.signalId);
    expect(validation.isValid).toBeDefined();
  });

  test('should classify signal', () => {
    const capture = app.captureSignal({ 
      frequency: 125000, 
      uid: '1234567890',
      modulation: 'ASK'
    });

    const classification = app.classifySignal(capture.signalId);
    expect(classification.detectedProtocol).toBeDefined();
  });

  test('should create and restore backup', () => {
    app.captureSignal({ name: 'S1', protocol: 'EM410x', uid: '111' });
    app.captureSignal({ name: 'S2', protocol: 'HID', uid: '222' });

    const backup = app.createBackup();
    expect(backup.signals).toBeDefined();

    app.storage.clear();
    
    const restore = app.restoreBackup(backup);
    expect(restore.success).toBe(true);
    expect(restore.signalCount).toBe(2);
  });

  test('should get transmission statistics', () => {
    const capture = app.captureSignal({ name: 'Test', protocol: 'EM410x', uid: '123' });
    app.replaySignal(capture.signalId);

    const stats = app.getTransmissionStats();
    expect(stats.totalTransmissions).toBeGreaterThan(0);
  });

  test('should get application info', () => {
    const info = app.getInfo();
    
    expect(info.name).toBe('RFID-SCAN-TRANS');
    expect(info.version).toBe('1.0.0');
    expect(info.features.length).toBeGreaterThan(0);
    expect(info.stats).toBeDefined();
  });

  test('should handle non-existent signal gracefully', () => {
    const result = app.exportSignal('non_existent_id', 'flipper');
    expect(result.success).toBe(false);
    expect(result.error).toContain('not found');
  });
});
