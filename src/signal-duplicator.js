/**
 * Duplication Modes System
 * Clone, Emulate, Replay, and Generate signal variants
 */

class SignalDuplicator {
  constructor() {
    this.modes = ['clone', 'emulate', 'replay', 'generate'];
    this.isTransmitting = false;
  }

  /**
   * Clone mode: Write to physical writable RFID cards
   * @param {Object} signal - Signal to clone
   * @param {Object} options - Clone options
   * @returns {Object} Clone result
   */
  clone(signal, options = {}) {
    const result = {
      mode: 'clone',
      success: false,
      message: '',
      signal: signal
    };

    try {
      // Validate signal for cloning
      if (!this._validateSignalForClone(signal)) {
        result.message = 'Signal validation failed - incompatible format';
        return result;
      }

      // Simulate writing to card
      result.success = true;
      result.message = `Successfully cloned ${signal.protocol} signal to card`;
      result.writtenData = {
        uid: signal.uid,
        blocks: this._prepareBlocks(signal),
        timestamp: new Date().toISOString()
      };
    } catch (error) {
      result.message = `Clone failed: ${error.message}`;
    }

    return result;
  }

  /**
   * Emulate mode: Continuous broadcast (simulates card presence)
   * @param {Object} signal - Signal to emulate
   * @param {Object} options - Emulation options
   * @returns {Object} Emulation controller
   */
  emulate(signal, options = {}) {
    const controller = {
      mode: 'emulate',
      signal: signal,
      isActive: false,
      startTime: null,
      transmissionCount: 0
    };

    controller.start = () => {
      if (this.isTransmitting) {
        return { success: false, message: 'Another transmission is active' };
      }

      this.isTransmitting = true;
      controller.isActive = true;
      controller.startTime = new Date();
      
      // Simulate continuous broadcast
      controller.intervalId = setInterval(() => {
        controller.transmissionCount++;
        this._transmit(signal, options);
      }, options.interval || 1000);

      return { 
        success: true, 
        message: `Emulation started for ${signal.protocol}`,
        controller: controller
      };
    };

    controller.stop = () => {
      if (controller.intervalId) {
        clearInterval(controller.intervalId);
        controller.intervalId = null;
      }
      controller.isActive = false;
      this.isTransmitting = false;

      const duration = controller.startTime 
        ? (new Date() - controller.startTime) / 1000 
        : 0;

      return {
        success: true,
        message: 'Emulation stopped',
        stats: {
          duration: duration,
          transmissions: controller.transmissionCount
        }
      };
    };

    controller.getStatus = () => {
      return {
        isActive: controller.isActive,
        transmissionCount: controller.transmissionCount,
        uptime: controller.startTime 
          ? (new Date() - controller.startTime) / 1000 
          : 0
      };
    };

    return controller;
  }

  /**
   * Replay mode: One-time transmission of captured signal
   * @param {Object} signal - Signal to replay
   * @param {Object} options - Replay options
   * @returns {Object} Replay result
   */
  replay(signal, options = {}) {
    const result = {
      mode: 'replay',
      success: false,
      message: '',
      signal: signal
    };

    try {
      if (this.isTransmitting) {
        result.message = 'Another transmission is active';
        return result;
      }

      const repeatCount = options.repeatCount || 1;
      const delay = options.delay || 0;

      this.isTransmitting = true;

      // Simulate replay transmission
      for (let i = 0; i < repeatCount; i++) {
        this._transmit(signal, options);
        if (i < repeatCount - 1 && delay > 0) {
          // Simulate delay
        }
      }

      this.isTransmitting = false;

      result.success = true;
      result.message = `Replayed ${signal.protocol} signal ${repeatCount} time(s)`;
      result.transmissions = repeatCount;
    } catch (error) {
      this.isTransmitting = false;
      result.message = `Replay failed: ${error.message}`;
    }

    return result;
  }

  /**
   * Generate mode: Create variants (incrementing IDs, etc.)
   * @param {Object} signal - Base signal
   * @param {Object} options - Generation options
   * @returns {Array} Generated signal variants
   */
  generate(signal, options = {}) {
    const variants = [];
    const count = options.count || 10;
    const mode = options.mode || 'increment'; // increment, decrement, random

    for (let i = 0; i < count; i++) {
      const variant = { ...signal };
      variant.id = `${signal.id}_variant_${i}`;
      variant.name = `${signal.name}_${i + 1}`;

      switch (mode) {
        case 'increment':
          variant.uid = this._incrementUID(signal.uid, i + 1);
          break;
        case 'decrement':
          variant.uid = this._decrementUID(signal.uid, i + 1);
          break;
        case 'random':
          variant.uid = this._randomizeUID(signal.uid);
          break;
      }

      variant.metadata = {
        ...variant.metadata,
        generatedFrom: signal.id,
        generationMode: mode,
        generationIndex: i
      };

      variants.push(variant);
    }

    return variants;
  }

  /**
   * Stop any active transmission
   * @returns {boolean} Success status
   */
  stopTransmission() {
    this.isTransmitting = false;
    return true;
  }

  /**
   * Validate signal for cloning
   * @private
   */
  _validateSignalForClone(signal) {
    if (!signal.uid || !signal.protocol) {
      return false;
    }

    // Check for writable protocols
    const writableProtocols = ['T55xx', 'EM4305', 'Hitag', 'Writable'];
    const isWritable = writableProtocols.some(p => 
      signal.protocol.toLowerCase().includes(p.toLowerCase())
    );

    return isWritable || signal.metadata?.writable === true;
  }

  /**
   * Prepare data blocks for writing
   * @private
   */
  _prepareBlocks(signal) {
    const blocks = [];
    const uid = signal.uid || '';
    
    // Split UID into blocks (simplified)
    for (let i = 0; i < uid.length; i += 8) {
      blocks.push(uid.substring(i, i + 8));
    }

    return blocks;
  }

  /**
   * Transmit signal
   * @private
   */
  _transmit(signal, options) {
    // Simulated transmission
    // In real implementation, this would interface with hardware
    const powerLevel = options.powerLevel || 4;
    return true;
  }

  /**
   * Increment UID
   * @private
   */
  _incrementUID(uid, increment) {
    if (!uid) return uid;
    
    try {
      const num = parseInt(uid.replace(/\s/g, ''), 16);
      const newNum = num + increment;
      return newNum.toString(16).toUpperCase().padStart(uid.replace(/\s/g, '').length, '0');
    } catch {
      return uid;
    }
  }

  /**
   * Decrement UID
   * @private
   */
  _decrementUID(uid, decrement) {
    if (!uid) return uid;
    
    try {
      const num = parseInt(uid.replace(/\s/g, ''), 16);
      const newNum = Math.max(0, num - decrement);
      return newNum.toString(16).toUpperCase().padStart(uid.replace(/\s/g, '').length, '0');
    } catch {
      return uid;
    }
  }

  /**
   * Randomize UID
   * @private
   */
  _randomizeUID(uid) {
    if (!uid) return uid;
    
    const length = uid.replace(/\s/g, '').length;
    let randomUID = '';
    
    for (let i = 0; i < length; i++) {
      randomUID += Math.floor(Math.random() * 16).toString(16).toUpperCase();
    }
    
    return randomUID;
  }
}

module.exports = SignalDuplicator;
