/**
 * Integration tests for RFIDScanTrans main application
 */

const RFIDScanTrans = require('../src/index');

describe('RFIDScanTrans Integration', () => {
  let app;

  beforeEach(() => {
    app = new RFIDScanTrans();
  });

  test('should complete full workflow: capture, store, analyze', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      frequency: '125kHz',
      uid: '1234567890AB'
    };

    // Capture and store
    const captured = app.captureAndStore(tagData);
    expect(captured.success).toBe(true);
    expect(captured.tag.id).toBeDefined();

    // Analyze
    const analysis = app.analyzeTag(captured.tag.id);
    expect(analysis.success).toBe(true);
    expect(analysis.analysis).toBeDefined();
  });

  test('should clone a stored tag', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };

    const captured = app.captureAndStore(tagData);
    const cloned = app.cloneTag(captured.tag.id);

    expect(cloned.success).toBe(true);
    expect(cloned.clone.sourceTag).toBe(captured.tag.id);
  });

  test('should emulate a stored tag', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };

    const captured = app.captureAndStore(tagData);
    const emulated = app.emulateTag(captured.tag.id);

    expect(emulated.success).toBe(true);
    expect(emulated.emulation.tagId).toBe(captured.tag.id);
  });

  test('should replay a stored tag', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };

    const captured = app.captureAndStore(tagData);
    const replayed = app.replayTag(captured.tag.id, 2);

    expect(replayed.success).toBe(true);
    expect(replayed.replay.count).toBe(2);
  });

  test('should generate and optionally store a new tag', () => {
    const generated = app.generateTag({ protocol: 'HID' }, true);

    expect(generated.success).toBe(true);
    expect(generated.tag.protocol).toBe('HID');

    const allTags = app.getAllTags();
    expect(allTags.length).toBe(1);
  });

  test('should export tags in multiple formats', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };

    app.captureAndStore(tagData);

    const jsonExport = app.exportTags('all', 'json');
    expect(jsonExport.success).toBe(true);

    const csvExport = app.exportTags('all', 'csv');
    expect(csvExport.success).toBe(true);

    const subExport = app.exportTags('all', 'sub');
    expect(subExport.success).toBe(true);
  });

  test('should search library', () => {
    app.captureAndStore({ rawData: 'TAG1', protocol: 'EM4100', uid: 'UID1' });
    app.captureAndStore({ rawData: 'TAG2', protocol: 'HID', uid: 'UID2' });

    const results = app.searchLibrary({ protocol: 'EM4100' });

    expect(results.length).toBe(1);
    expect(results[0].protocol).toBe('EM4100');
  });

  test('should generate statistics', () => {
    app.captureAndStore({ rawData: 'TAG1', protocol: 'EM4100', uid: 'UID1', frequency: '125kHz' });
    app.captureAndStore({ rawData: 'TAG2', protocol: 'EM4100', uid: 'UID2', frequency: '125kHz' });
    app.captureAndStore({ rawData: 'TAG3', protocol: 'HID', uid: 'UID3', frequency: '125kHz' });

    const stats = app.getStatistics();

    expect(stats.totalTags).toBe(3);
    expect(stats.protocols['EM4100']).toBe(2);
    expect(stats.protocols['HID']).toBe(1);
  });

  test('should clear all data', () => {
    app.captureAndStore({ rawData: 'TAG1', protocol: 'EM4100', uid: 'UID1' });
    app.clearAll();

    const allTags = app.getAllTags();
    expect(allTags.length).toBe(0);
  });
});
