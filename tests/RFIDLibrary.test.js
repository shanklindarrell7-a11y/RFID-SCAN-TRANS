/**
 * Tests for RFIDLibrary module
 */

const RFIDLibrary = require('../src/library/RFIDLibrary');

describe('RFIDLibrary', () => {
  let library;

  beforeEach(() => {
    library = new RFIDLibrary();
  });

  test('should store a tag successfully', () => {
    const tag = {
      id: 'tag_123',
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100',
      uid: '1234567890AB'
    };

    const result = library.store(tag);

    expect(result).toBe(true);
    expect(library.size()).toBe(1);
  });

  test('should retrieve a stored tag by ID', () => {
    const tag = {
      id: 'tag_123',
      rawData: '1234567890ABCDEF',
      protocol: 'EM4100'
    };

    library.store(tag);
    const retrieved = library.retrieve('tag_123');

    expect(retrieved).toBeDefined();
    expect(retrieved.id).toBe(tag.id);
    expect(retrieved.rawData).toBe(tag.rawData);
  });

  test('should return null for non-existent tag', () => {
    const retrieved = library.retrieve('nonexistent');
    expect(retrieved).toBeNull();
  });

  test('should get all stored tags', () => {
    library.store({ id: 'tag_1', protocol: 'EM4100' });
    library.store({ id: 'tag_2', protocol: 'HID' });

    const allTags = library.getAll();

    expect(allTags.length).toBe(2);
  });

  test('should search tags by protocol', () => {
    library.store({ id: 'tag_1', protocol: 'EM4100', uid: 'UID1' });
    library.store({ id: 'tag_2', protocol: 'HID', uid: 'UID2' });
    library.store({ id: 'tag_3', protocol: 'EM4100', uid: 'UID3' });

    const results = library.search({ protocol: 'EM4100' });

    expect(results.length).toBe(2);
    expect(results.every(tag => tag.protocol === 'EM4100')).toBe(true);
  });

  test('should delete a tag', () => {
    library.store({ id: 'tag_1', protocol: 'EM4100' });
    
    const deleted = library.delete('tag_1');

    expect(deleted).toBe(true);
    expect(library.size()).toBe(0);
  });

  test('should clear all tags', () => {
    library.store({ id: 'tag_1', protocol: 'EM4100' });
    library.store({ id: 'tag_2', protocol: 'HID' });

    library.clear();

    expect(library.size()).toBe(0);
  });
});
