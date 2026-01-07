/**
 * Tests for Professional Features
 */

const ProfessionalFeatures = require('../src/professional-features');

describe('ProfessionalFeatures', () => {
  let features;

  beforeEach(() => {
    features = new ProfessionalFeatures();
  });

  describe('Signal Validation', () => {
    test('should validate valid signal', () => {
      const signal = {
        name: 'Test Signal',
        protocol: 'EM410x',
        uid: '1234567890',
        frequency: 125000
      };

      const validation = features.validateSignal(signal);
      
      expect(validation.isValid).toBe(true);
      expect(validation.errors.length).toBe(0);
    });

    test('should detect missing UID', () => {
      const signal = {
        name: 'Test Signal',
        protocol: 'EM410x'
      };

      const validation = features.validateSignal(signal);
      
      expect(validation.isValid).toBe(false);
      expect(validation.errors).toContain('Missing UID');
    });

    test('should detect missing protocol', () => {
      const signal = {
        name: 'Test Signal',
        uid: '1234567890'
      };

      const validation = features.validateSignal(signal);
      
      expect(validation.isValid).toBe(false);
      expect(validation.errors).toContain('Missing protocol');
    });

    test('should warn on weak signal strength', () => {
      const signal = {
        protocol: 'EM410x',
        uid: '1234567890',
        signalStrength: -75
      };

      const validation = features.validateSignal(signal);
      
      expect(validation.warnings.length).toBeGreaterThan(0);
      expect(validation.warnings.some(w => w.includes('signal'))).toBe(true);
    });
  });

  describe('Auto-Classification', () => {
    test('should classify EM410x by frequency and UID length', () => {
      const signal = {
        frequency: 125000,
        uid: '1234567890',
        modulation: 'ASK'
      };

      const classification = features.autoClassify(signal);
      
      expect(classification.detectedProtocol).toBe('EM410x');
      expect(classification.confidence).toBeGreaterThan(0);
    });

    test('should classify HID by frequency and UID length', () => {
      const signal = {
        frequency: 125000,
        uid: '1234567890ABCDEF'
      };

      const classification = features.autoClassify(signal);
      
      expect(classification.detectedProtocol).toBe('HID');
      expect(classification.alternatives).toBeDefined();
    });

    test('should classify Mifare by 13.56MHz frequency', () => {
      const signal = {
        frequency: 13560000,
        uid: '12345678'
      };

      const classification = features.autoClassify(signal);
      
      expect(classification.detectedProtocol).toBe('Mifare');
    });

    test('should provide reasoning for classification', () => {
      const signal = {
        frequency: 125000,
        uid: '1234567890',
        modulation: 'ASK'
      };

      const classification = features.autoClassify(signal);
      
      expect(classification.reasoning.length).toBeGreaterThan(0);
    });
  });

  describe('Batch Operations', () => {
    test('should perform batch validation', () => {
      const signals = [
        { protocol: 'EM410x', uid: '1234567890' },
        { protocol: 'HID', uid: 'ABCDEF123456' },
        { protocol: 'T55xx', uid: '9876543210' }
      ];

      const result = features.batchOperation(signals, 'validate');
      
      expect(result.total).toBe(3);
      expect(result.successful).toBeGreaterThan(0);
      expect(result.results.length).toBe(3);
    });

    test('should perform batch classification', () => {
      const signals = [
        { frequency: 125000, uid: '1234567890' },
        { frequency: 13560000, uid: '12345678' }
      ];

      const result = features.batchOperation(signals, 'classify');
      
      expect(result.total).toBe(2);
      expect(result.results.length).toBe(2);
    });
  });

  describe('Encryption', () => {
    test('should encrypt signal data', () => {
      const signal = {
        id: 'test_123',
        uid: '1234567890',
        data: 'test_data'
      };

      const encrypted = features.encryptSignal(signal, 'testkey123');
      
      expect(encrypted.encrypted).toBe(true);
      expect(encrypted.uid).not.toBe(signal.uid);
      expect(encrypted.keyHash).toBeDefined();
    });

    test('should decrypt signal data', () => {
      const signal = {
        id: 'test_123',
        uid: '1234567890',
        data: 'test_data'
      };

      const key = 'testkey123';
      const encrypted = features.encryptSignal(signal, key);
      const decrypted = features.decryptSignal(encrypted, key);
      
      expect(decrypted.encrypted).toBe(false);
    });

    test('should reject wrong decryption key', () => {
      const signal = {
        id: 'test_123',
        uid: '1234567890'
      };

      const encrypted = features.encryptSignal(signal, 'correctkey');
      
      expect(() => {
        features.decryptSignal(encrypted, 'wrongkey');
      }).toThrow('Invalid decryption key');
    });

    test('should set and use encryption key', () => {
      features.setEncryptionKey('mykey123');
      
      const signal = { uid: '1234567890' };
      const encrypted = features.encryptSignal(signal);
      
      expect(encrypted.encrypted).toBe(true);
    });
  });
});
