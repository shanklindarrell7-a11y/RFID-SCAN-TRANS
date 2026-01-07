/**
 * Professional Features System
 * Signal validation, classification, batch operations, and encryption
 */

class ProfessionalFeatures {
  constructor() {
    this.protocols = {
      'EM410x': { frequency: 125000, bits: 64, modulation: 'ASK' },
      'HID': { frequency: 125000, bits: 26, modulation: 'FSK' },
      'T55xx': { frequency: 125000, bits: 330, modulation: 'ASK/FSK/PSK' },
      'Indala': { frequency: 125000, bits: 64, modulation: 'PSK' },
      'AWID': { frequency: 125000, bits: 50, modulation: 'FSK' },
      'Mifare': { frequency: 13560000, bits: 32, modulation: 'ASK' },
      'iClass': { frequency: 13560000, bits: 64, modulation: 'ASK' }
    };
    this.encryptionKey = null;
  }

  /**
   * Validate signal integrity
   * @param {Object} signal - Signal to validate
   * @returns {Object} Validation result
   */
  validateSignal(signal) {
    const validation = {
      isValid: true,
      errors: [],
      warnings: [],
      score: 100
    };

    // Check required fields
    if (!signal.uid) {
      validation.errors.push('Missing UID');
      validation.isValid = false;
      validation.score -= 30;
    }

    if (!signal.protocol) {
      validation.errors.push('Missing protocol');
      validation.isValid = false;
      validation.score -= 20;
    }

    // Check UID format
    if (signal.uid) {
      const uidValidation = this._validateUID(signal.uid, signal.protocol);
      if (!uidValidation.valid) {
        validation.warnings.push(`UID format issue: ${uidValidation.message}`);
        validation.score -= 10;
      }
    }

    // Check frequency
    if (signal.frequency) {
      if (!this._isValidFrequency(signal.frequency)) {
        validation.warnings.push('Non-standard frequency');
        validation.score -= 5;
      }
    }

    // Check protocol compatibility
    if (signal.protocol && this.protocols[signal.protocol]) {
      const protocolInfo = this.protocols[signal.protocol];
      
      if (signal.frequency && signal.frequency !== protocolInfo.frequency) {
        validation.warnings.push(`Frequency mismatch for ${signal.protocol}`);
        validation.score -= 10;
      }
    }

    // Check signal strength
    if (signal.signalStrength !== undefined) {
      if (signal.signalStrength < -80) {
        validation.warnings.push('Very weak signal');
        validation.score -= 15;
      } else if (signal.signalStrength < -60) {
        validation.warnings.push('Weak signal');
        validation.score -= 5;
      }
    }

    validation.score = Math.max(0, validation.score);
    return validation;
  }

  /**
   * Auto-classify signal protocol
   * @param {Object} signal - Signal to classify
   * @returns {Object} Classification result
   */
  autoClassify(signal) {
    const classification = {
      detectedProtocol: null,
      confidence: 0,
      alternatives: [],
      reasoning: []
    };

    // Frequency-based classification
    if (signal.frequency) {
      if (Math.abs(signal.frequency - 125000) < 1000) {
        classification.reasoning.push('125kHz frequency detected - LF RFID');
        
        // Further classify based on UID length and pattern
        if (signal.uid) {
          const uidLength = signal.uid.replace(/\s/g, '').length;
          
          if (uidLength === 10) {
            classification.detectedProtocol = 'EM410x';
            classification.confidence = 85;
            classification.reasoning.push('10-character UID matches EM410x');
          } else if (uidLength === 16) {
            classification.detectedProtocol = 'HID';
            classification.confidence = 75;
            classification.alternatives.push('T55xx', 'AWID');
            classification.reasoning.push('16-character UID suggests HID or T55xx');
          }
        }
      } else if (Math.abs(signal.frequency - 13560000) < 10000) {
        classification.reasoning.push('13.56MHz frequency detected - HF RFID');
        
        if (signal.uid) {
          const uidLength = signal.uid.replace(/\s/g, '').length;
          
          if (uidLength === 8) {
            classification.detectedProtocol = 'Mifare';
            classification.confidence = 80;
            classification.reasoning.push('8-character UID matches Mifare Classic');
          } else if (uidLength === 14) {
            classification.detectedProtocol = 'Mifare';
            classification.confidence = 75;
            classification.alternatives.push('Mifare DESFire');
            classification.reasoning.push('14-character UID matches Mifare DESFire');
          } else if (uidLength === 16) {
            classification.detectedProtocol = 'iClass';
            classification.confidence = 70;
            classification.reasoning.push('16-character UID suggests iClass');
          }
        }
      }
    }

    // Modulation-based classification
    if (signal.modulation) {
      if (signal.modulation === 'ASK' && !classification.detectedProtocol) {
        classification.detectedProtocol = 'EM410x';
        classification.confidence = 60;
        classification.reasoning.push('ASK modulation suggests EM410x');
      }
    }

    // Pattern-based classification
    if (signal.data) {
      const patterns = this._detectPatterns(signal.data);
      if (patterns.length > 0) {
        classification.reasoning.push(`Detected patterns: ${patterns.join(', ')}`);
      }
    }

    return classification;
  }

  /**
   * Perform batch operations on multiple signals
   * @param {Array} signals - Array of signals
   * @param {string} operation - Operation to perform
   * @param {Object} options - Operation options
   * @returns {Object} Batch operation result
   */
  batchOperation(signals, operation, options = {}) {
    const results = {
      operation: operation,
      total: signals.length,
      successful: 0,
      failed: 0,
      results: []
    };

    signals.forEach(signal => {
      let result = null;

      try {
        switch (operation) {
          case 'validate':
            result = this.validateSignal(signal);
            break;
          case 'classify':
            result = this.autoClassify(signal);
            break;
          case 'encrypt':
            result = this.encryptSignal(signal, options.key);
            break;
          case 'decrypt':
            result = this.decryptSignal(signal, options.key);
            break;
          default:
            throw new Error(`Unknown operation: ${operation}`);
        }

        results.successful++;
        results.results.push({
          signalId: signal.id,
          success: true,
          result: result
        });
      } catch (error) {
        results.failed++;
        results.results.push({
          signalId: signal.id,
          success: false,
          error: error.message
        });
      }
    });

    return results;
  }

  /**
   * Encrypt signal data (basic obfuscation)
   * @param {Object} signal - Signal to encrypt
   * @param {string} key - Encryption key
   * @returns {Object} Encrypted signal
   */
  encryptSignal(signal, key = null) {
    const encryptedSignal = { ...signal };
    const effectiveKey = key || this.encryptionKey || this._generateKey();

    if (signal.uid) {
      encryptedSignal.uid = this._xorEncrypt(signal.uid, effectiveKey);
    }

    if (signal.data) {
      encryptedSignal.data = this._xorEncrypt(signal.data, effectiveKey);
    }

    encryptedSignal.encrypted = true;
    encryptedSignal.encryptionMethod = 'XOR';
    encryptedSignal.keyHash = this._hashKey(effectiveKey);

    return encryptedSignal;
  }

  /**
   * Decrypt signal data
   * @param {Object} signal - Encrypted signal
   * @param {string} key - Decryption key
   * @returns {Object} Decrypted signal
   */
  decryptSignal(signal, key = null) {
    if (!signal.encrypted) {
      return signal;
    }

    const decryptedSignal = { ...signal };
    const effectiveKey = key || this.encryptionKey;

    if (!effectiveKey) {
      throw new Error('Decryption key required');
    }

    // Verify key
    if (signal.keyHash && this._hashKey(effectiveKey) !== signal.keyHash) {
      throw new Error('Invalid decryption key');
    }

    if (signal.uid) {
      decryptedSignal.uid = this._xorEncrypt(signal.uid, effectiveKey); // XOR is symmetric
    }

    if (signal.data) {
      decryptedSignal.data = this._xorEncrypt(signal.data, effectiveKey);
    }

    decryptedSignal.encrypted = false;
    delete decryptedSignal.encryptionMethod;
    delete decryptedSignal.keyHash;

    return decryptedSignal;
  }

  /**
   * Set encryption key
   * @param {string} key - Encryption key
   */
  setEncryptionKey(key) {
    this.encryptionKey = key;
  }

  /**
   * Validate UID format
   * @private
   */
  _validateUID(uid, protocol) {
    const result = { valid: true, message: '' };

    // Remove spaces for validation
    const cleanUID = uid.replace(/\s/g, '');

    // Check if hex
    if (!/^[0-9A-Fa-f]+$/.test(cleanUID)) {
      result.valid = false;
      result.message = 'UID contains non-hex characters';
      return result;
    }

    // Check length based on protocol
    if (protocol && this.protocols[protocol]) {
      const expectedBits = this.protocols[protocol].bits;
      const expectedLength = Math.ceil(expectedBits / 4);
      
      if (cleanUID.length !== expectedLength && cleanUID.length !== expectedLength * 2) {
        result.valid = false;
        result.message = `UID length ${cleanUID.length} doesn't match expected ${expectedLength} for ${protocol}`;
      }
    }

    return result;
  }

  /**
   * Check if frequency is valid
   * @private
   */
  _isValidFrequency(frequency) {
    const standardFrequencies = [125000, 134200, 13560000];
    return standardFrequencies.some(freq => Math.abs(frequency - freq) < 10000);
  }

  /**
   * Detect patterns in data
   * @private
   */
  _detectPatterns(data) {
    const patterns = [];

    // Check for repeating bytes
    if (/(.{2})\1{3,}/.test(data)) {
      patterns.push('repeating-bytes');
    }

    // Check for sequential data
    if (/0123456789ABCDEF/.test(data.toUpperCase())) {
      patterns.push('sequential');
    }

    // Check for all zeros or all ones
    if (/^0+$/.test(data) || /^F+$/i.test(data)) {
      patterns.push('uniform');
    }

    return patterns;
  }

  /**
   * XOR encryption/decryption
   * @private
   */
  _xorEncrypt(text, key) {
    let result = '';
    for (let i = 0; i < text.length; i++) {
      const charCode = text.charCodeAt(i) ^ key.charCodeAt(i % key.length);
      result += String.fromCharCode(charCode);
    }
    return Buffer.from(result).toString('base64');
  }

  /**
   * Generate encryption key
   * @private
   */
  _generateKey() {
    return Math.random().toString(36).substring(2, 15) + 
           Math.random().toString(36).substring(2, 15);
  }

  /**
   * Hash key for verification
   * @private
   */
  _hashKey(key) {
    let hash = 0;
    for (let i = 0; i < key.length; i++) {
      hash = ((hash << 5) - hash) + key.charCodeAt(i);
      hash = hash & hash;
    }
    return hash.toString(36);
  }
}

module.exports = ProfessionalFeatures;
