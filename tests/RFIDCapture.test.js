/**
 * Tests for RFIDCapture module
 */

const RFIDCapture = require('../src/capture/RFIDCapture');

describe('RFIDCapture', () => {
  let capture;

  beforeEach(() => {
    capture = new RFIDCapture();
  });

  test('should capture RFID tag with valid data', () => {
    const tagData = {
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      frequency: '125kHz',
      uid: '1234567890AB'
    };

    const result = capture.capture(tagData);

    expect(result).toBeDefined();
    expect(result.id).toBeDefined();
    expect(result.rawData).toBe(tagData.rawData);
    expect(result.protocol).toBe(tagData.protocol);
    expect(result.uid).toBe(tagData.uid);
    expect(result.timestamp).toBeDefined();
  });

  test('should use default values when tag data is incomplete', () => {
    const tagData = {
      rawData: '1234567890ABCDEF'
    };

    const result = capture.capture(tagData);

    expect(result.protocol).toBe('EM4100');
    expect(result.frequency).toBe('125kHz');
  });

  test('should extract UID from raw data if not provided', () => {
    const tagData = {
      rawData: '1234567890ABCDEF'
    };

    const result = capture.capture(tagData);

    expect(result.uid).toBe('1234567890ABCDEF');
  });

  test('should track captured tags', () => {
    const tagData1 = { rawData: 'TAG1', uid: 'UID1' };
    const tagData2 = { rawData: 'TAG2', uid: 'UID2' };

    capture.capture(tagData1);
    capture.capture(tagData2);

    const tags = capture.getCapturedTags();
    expect(tags.length).toBe(2);
  });

  test('should clear captured tags', () => {
    capture.capture({ rawData: 'TAG1' });
    capture.capture({ rawData: 'TAG2' });

    capture.clear();

    expect(capture.getCapturedTags().length).toBe(0);
  });
});
