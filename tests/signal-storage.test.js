/**
 * Tests for Signal Storage System
 */

const SignalStorage = require('../src/signal-storage');

describe('SignalStorage', () => {
  let storage;

  beforeEach(() => {
    storage = new SignalStorage();
  });

  test('should add a signal and return ID', () => {
    const signal = {
      name: 'Test Signal',
      protocol: 'EM410x',
      uid: '1234567890',
      frequency: 125000
    };

    const id = storage.addSignal(signal);
    expect(id).toBeTruthy();
    expect(typeof id).toBe('string');
  });

  test('should retrieve a signal by ID', () => {
    const signal = {
      name: 'Test Signal',
      protocol: 'EM410x',
      uid: '1234567890'
    };

    const id = storage.addSignal(signal);
    const retrieved = storage.getSignal(id);

    expect(retrieved).toBeTruthy();
    expect(retrieved.name).toBe('Test Signal');
    expect(retrieved.protocol).toBe('EM410x');
    expect(retrieved.uid).toBe('1234567890');
  });

  test('should search signals by name', () => {
    storage.addSignal({ name: 'Signal One', protocol: 'EM410x' });
    storage.addSignal({ name: 'Signal Two', protocol: 'HID' });
    storage.addSignal({ name: 'Another One', protocol: 'EM410x' });

    const results = storage.searchSignals({ name: 'One' });
    expect(results.length).toBe(2);
  });

  test('should filter signals by protocol', () => {
    storage.addSignal({ name: 'Signal 1', protocol: 'EM410x' });
    storage.addSignal({ name: 'Signal 2', protocol: 'HID' });
    storage.addSignal({ name: 'Signal 3', protocol: 'EM410x' });

    const results = storage.filterByProtocol('EM410x');
    expect(results.length).toBe(2);
  });

  test('should toggle favorite status', () => {
    const id = storage.addSignal({ name: 'Test', protocol: 'EM410x' });
    
    expect(storage.toggleFavorite(id)).toBe(true);
    expect(storage.toggleFavorite(id)).toBe(false);
  });

  test('should get favorite signals', () => {
    const id1 = storage.addSignal({ name: 'Fav 1', protocol: 'EM410x' });
    const id2 = storage.addSignal({ name: 'Fav 2', protocol: 'HID' });
    storage.addSignal({ name: 'Not Fav', protocol: 'EM410x' });

    storage.toggleFavorite(id1);
    storage.toggleFavorite(id2);

    const favorites = storage.getFavorites();
    expect(favorites.length).toBe(2);
  });

  test('should update signal metadata', () => {
    const id = storage.addSignal({ name: 'Test', protocol: 'EM410x' });
    
    storage.updateSignal(id, { notes: 'Updated notes' });
    const signal = storage.getSignal(id);
    
    expect(signal.notes).toBe('Updated notes');
  });

  test('should delete a signal', () => {
    const id = storage.addSignal({ name: 'Test', protocol: 'EM410x' });
    
    expect(storage.deleteSignal(id)).toBe(true);
    expect(storage.getSignal(id)).toBeNull();
  });

  test('should count signals', () => {
    expect(storage.count()).toBe(0);
    
    storage.addSignal({ name: 'Signal 1', protocol: 'EM410x' });
    storage.addSignal({ name: 'Signal 2', protocol: 'HID' });
    
    expect(storage.count()).toBe(2);
  });

  test('should create and restore backup', () => {
    storage.addSignal({ name: 'Signal 1', protocol: 'EM410x' });
    storage.addSignal({ name: 'Signal 2', protocol: 'HID' });

    const backup = storage.backup();
    expect(backup.signals.length).toBe(2);

    storage.clear();
    expect(storage.count()).toBe(0);

    storage.restore(backup);
    expect(storage.count()).toBe(2);
  });

  test('should clear all signals', () => {
    storage.addSignal({ name: 'Signal 1', protocol: 'EM410x' });
    storage.addSignal({ name: 'Signal 2', protocol: 'HID' });

    storage.clear();
    expect(storage.count()).toBe(0);
  });
});
