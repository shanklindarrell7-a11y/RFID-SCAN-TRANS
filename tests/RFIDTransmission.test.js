/**
 * Tests for RFIDTransmission module
 */

const RFIDTransmission = require('../src/transmission/RFIDTransmission');

describe('RFIDTransmission', () => {
  let transmission;
  let sampleTag;

  beforeEach(() => {
    transmission = new RFIDTransmission();
    sampleTag = {
      id: 'tag_123',
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };
  });

  test('should clone a tag successfully', () => {
    const result = transmission.clone(sampleTag);

    expect(result.success).toBe(true);
    expect(result.clone).toBeDefined();
    expect(result.clone.sourceTag).toBe(sampleTag.id);
    expect(result.clone.clonedData).toBe(sampleTag.rawData);
  });

  test('should fail to clone invalid tag', () => {
    const result = transmission.clone({});

    expect(result.success).toBe(false);
    expect(result.error).toBeDefined();
  });

  test('should emulate a tag successfully', () => {
    const result = transmission.emulate(sampleTag);

    expect(result.success).toBe(true);
    expect(result.emulation).toBeDefined();
    expect(transmission.isEmulating()).toBe(true);
    expect(result.emulation.tagId).toBe(sampleTag.id);
  });

  test('should stop emulation', () => {
    transmission.emulate(sampleTag);
    const result = transmission.stopEmulation();

    expect(result.success).toBe(true);
    expect(transmission.isEmulating()).toBe(false);
  });

  test('should fail to stop emulation when not active', () => {
    const result = transmission.stopEmulation();

    expect(result.success).toBe(false);
    expect(result.error).toContain('No active emulation');
  });

  test('should replay a tag successfully', () => {
    const result = transmission.replay(sampleTag, 3);

    expect(result.success).toBe(true);
    expect(result.replay).toBeDefined();
    expect(result.replay.count).toBe(3);
  });

  test('should fail to replay with invalid count', () => {
    const result = transmission.replay(sampleTag, 0);

    expect(result.success).toBe(false);
    expect(result.error).toBeDefined();
  });

  test('should generate a new tag with defaults', () => {
    const result = transmission.generate({});

    expect(result.success).toBe(true);
    expect(result.tag).toBeDefined();
    expect(result.tag.protocol).toBe('EM4100');
    expect(result.tag.frequency).toBe('125kHz');
    expect(result.tag.uid).toBeDefined();
  });

  test('should generate a tag with custom parameters', () => {
    const params = {
      protocol: 'HID',
      frequency: '125kHz',
      uid: 'CUSTOM123456'
    };

    const result = transmission.generate(params);

    expect(result.success).toBe(true);
    expect(result.tag.protocol).toBe('HID');
    expect(result.tag.uid).toBe('CUSTOM123456');
  });

  test('should track current emulation', () => {
    transmission.emulate(sampleTag);
    const current = transmission.getCurrentEmulation();

    expect(current).toBeDefined();
    expect(current.tagId).toBe(sampleTag.id);
  });
});
