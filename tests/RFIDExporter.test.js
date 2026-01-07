/**
 * Tests for RFIDExporter module
 */

const RFIDExporter = require('../src/export/RFIDExporter');

describe('RFIDExporter', () => {
  let exporter;
  let sampleTag;

  beforeEach(() => {
    exporter = new RFIDExporter();
    sampleTag = {
      id: 'tag_123',
      timestamp: '2026-01-07T00:00:00.000Z',
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      frequency: '125kHz',
      uid: '1234567890AB',
      metadata: {
        signalStrength: 85
      }
    };
  });

  test('should export to JSON format', () => {
    const result = exporter.export(sampleTag, 'json');

    expect(result.success).toBe(true);
    expect(result.format).toBe('json');
    expect(result.data).toBeDefined();

    const parsed = JSON.parse(result.data);
    expect(parsed.tags).toHaveLength(1);
    expect(parsed.tags[0].id).toBe(sampleTag.id);
  });

  test('should export to CSV format', () => {
    const result = exporter.export(sampleTag, 'csv');

    expect(result.success).toBe(true);
    expect(result.format).toBe('csv');
    expect(result.data).toContain('ID,Timestamp,Protocol');
    expect(result.data).toContain(sampleTag.id);
  });

  test('should export to SubGHz format', () => {
    const result = exporter.export(sampleTag, 'sub');

    expect(result.success).toBe(true);
    expect(result.data).toContain('Flipper SubGHz');
    expect(result.data).toContain(sampleTag.protocol);
    expect(result.data).toContain(sampleTag.uid);
  });

  test('should export to Wiegand format', () => {
    const result = exporter.export(sampleTag, 'wiegand');

    expect(result.success).toBe(true);
    expect(result.data).toContain('Wiegand Format');
    expect(result.data).toContain('FacilityCode');
    expect(result.data).toContain('CardNumber');
  });

  test('should export to Proxmark3 format', () => {
    const result = exporter.export(sampleTag, 'pm3');

    expect(result.success).toBe(true);
    expect(result.data).toContain('Proxmark3');
    expect(result.data).toContain('Protocol=');
    expect(result.data).toContain(sampleTag.protocol);
  });

  test('should export multiple tags', () => {
    const tag2 = { ...sampleTag, id: 'tag_456', uid: 'ABCDEF123456' };
    const result = exporter.export([sampleTag, tag2], 'json');

    expect(result.success).toBe(true);
    expect(result.tagCount).toBe(2);

    const parsed = JSON.parse(result.data);
    expect(parsed.tags).toHaveLength(2);
  });

  test('should return error for unsupported format', () => {
    const result = exporter.export(sampleTag, 'unsupported');

    expect(result.success).toBe(false);
    expect(result.error).toContain('Unsupported format');
  });

  test('should return error for empty tag array', () => {
    const result = exporter.export([], 'json');

    expect(result.success).toBe(false);
    expect(result.error).toContain('No tags to export');
  });

  test('should return list of supported formats', () => {
    const formats = exporter.getSupportedFormats();

    expect(formats).toContain('sub');
    expect(formats).toContain('csv');
    expect(formats).toContain('wiegand');
    expect(formats).toContain('pm3');
    expect(formats).toContain('json');
  });
});
