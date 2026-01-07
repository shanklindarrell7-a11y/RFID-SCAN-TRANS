/**
 * Transmission Control System
 * Manages power levels, timing, and continuous transmission modes
 */

class TransmissionController {
  constructor() {
    this.powerLevels = 8; // 0-7
    this.currentPowerLevel = 4; // Default medium power
    this.isTransmitting = false;
    this.transmissionStats = {
      totalTransmissions: 0,
      successCount: 0,
      failureCount: 0,
      lastTransmission: null
    };
  }

  /**
   * Set transmission power level
   * @param {number} level - Power level (0-7)
   * @returns {boolean} Success status
   */
  setPowerLevel(level) {
    if (level < 0 || level >= this.powerLevels) {
      return false;
    }
    this.currentPowerLevel = level;
    return true;
  }

  /**
   * Get current power level
   * @returns {number} Current power level
   */
  getPowerLevel() {
    return this.currentPowerLevel;
  }

  /**
   * Get power level details
   * @param {number} level - Power level
   * @returns {Object} Power level information
   */
  getPowerLevelInfo(level = this.currentPowerLevel) {
    const info = {
      level: level,
      percentage: Math.round((level / (this.powerLevels - 1)) * 100),
      range: this._calculateRange(level),
      description: this._getPowerDescription(level)
    };
    return info;
  }

  /**
   * Transmit signal with configured settings
   * @param {Object} signal - Signal to transmit
   * @param {Object} options - Transmission options
   * @returns {Object} Transmission result
   */
  transmit(signal, options = {}) {
    const result = {
      success: false,
      message: '',
      timestamp: new Date().toISOString(),
      powerLevel: options.powerLevel || this.currentPowerLevel,
      repeatCount: options.repeatCount || 1,
      delay: options.delay || 0
    };

    if (this.isTransmitting) {
      result.message = 'Transmission already in progress';
      return result;
    }

    try {
      this.isTransmitting = true;
      const powerLevel = options.powerLevel !== undefined 
        ? options.powerLevel 
        : this.currentPowerLevel;

      // Validate power level
      if (powerLevel < 0 || powerLevel >= this.powerLevels) {
        throw new Error('Invalid power level');
      }

      // Execute transmission
      const transmissionResult = this._executeTransmission(signal, {
        ...options,
        powerLevel: powerLevel
      });

      result.success = transmissionResult.success;
      result.message = transmissionResult.message;
      result.transmissionId = transmissionResult.transmissionId;

      // Update stats
      this.transmissionStats.totalTransmissions++;
      if (result.success) {
        this.transmissionStats.successCount++;
      } else {
        this.transmissionStats.failureCount++;
      }
      this.transmissionStats.lastTransmission = result.timestamp;

    } catch (error) {
      result.message = `Transmission error: ${error.message}`;
      this.transmissionStats.failureCount++;
    } finally {
      this.isTransmitting = false;
    }

    return result;
  }

  /**
   * Start continuous transmission mode
   * @param {Object} signal - Signal to transmit
   * @param {Object} options - Continuous mode options
   * @returns {Object} Continuous transmission controller
   */
  startContinuous(signal, options = {}) {
    const controller = {
      signal: signal,
      isActive: false,
      startTime: null,
      transmissionCount: 0,
      powerLevel: options.powerLevel || this.currentPowerLevel,
      interval: options.interval || 1000,
      monitoring: {
        enabled: options.monitoring !== false,
        feedback: []
      }
    };

    controller.start = () => {
      if (this.isTransmitting) {
        return { success: false, message: 'Another transmission is active' };
      }

      controller.isActive = true;
      controller.startTime = new Date();
      
      // Start continuous transmission
      controller.intervalId = setInterval(() => {
        const result = this._executeTransmission(signal, {
          powerLevel: controller.powerLevel,
          continuous: true
        });

        controller.transmissionCount++;

        if (controller.monitoring.enabled) {
          controller.monitoring.feedback.push({
            timestamp: new Date().toISOString(),
            success: result.success,
            powerLevel: controller.powerLevel
          });

          // Keep only last 100 entries
          if (controller.monitoring.feedback.length > 100) {
            controller.monitoring.feedback.shift();
          }
        }
      }, controller.interval);

      return {
        success: true,
        message: 'Continuous transmission started',
        controller: controller
      };
    };

    controller.stop = () => {
      if (controller.intervalId) {
        clearInterval(controller.intervalId);
        controller.intervalId = null;
      }
      controller.isActive = false;

      const duration = controller.startTime 
        ? (new Date() - controller.startTime) / 1000 
        : 0;

      return {
        success: true,
        message: 'Continuous transmission stopped',
        stats: {
          duration: duration,
          transmissionCount: controller.transmissionCount,
          averageRate: controller.transmissionCount / duration
        }
      };
    };

    controller.updatePowerLevel = (level) => {
      if (level >= 0 && level < this.powerLevels) {
        controller.powerLevel = level;
        return true;
      }
      return false;
    };

    controller.getMonitoring = () => {
      return {
        isActive: controller.isActive,
        transmissionCount: controller.transmissionCount,
        uptime: controller.startTime 
          ? (new Date() - controller.startTime) / 1000 
          : 0,
        powerLevel: controller.powerLevel,
        recentFeedback: controller.monitoring.feedback.slice(-10)
      };
    };

    return controller;
  }

  /**
   * Transmit with repeat control
   * @param {Object} signal - Signal to transmit
   * @param {number} repeatCount - Number of repeats
   * @param {number} delay - Delay between repeats (ms)
   * @returns {Object} Result with all transmissions
   */
  transmitWithRepeat(signal, repeatCount, delay = 0) {
    const results = [];

    for (let i = 0; i < repeatCount; i++) {
      const result = this.transmit(signal, {
        powerLevel: this.currentPowerLevel,
        repeatIndex: i,
        totalRepeats: repeatCount
      });

      results.push(result);

      // Simulate delay between transmissions
      if (i < repeatCount - 1 && delay > 0) {
        // In real implementation, would use actual delay
      }
    }

    return {
      success: results.every(r => r.success),
      message: `Transmitted ${repeatCount} times`,
      results: results,
      successRate: (results.filter(r => r.success).length / repeatCount) * 100
    };
  }

  /**
   * Get transmission statistics
   * @returns {Object} Transmission statistics
   */
  getStats() {
    return {
      ...this.transmissionStats,
      successRate: this.transmissionStats.totalTransmissions > 0
        ? (this.transmissionStats.successCount / this.transmissionStats.totalTransmissions) * 100
        : 0
    };
  }

  /**
   * Reset transmission statistics
   */
  resetStats() {
    this.transmissionStats = {
      totalTransmissions: 0,
      successCount: 0,
      failureCount: 0,
      lastTransmission: null
    };
  }

  /**
   * Execute single transmission
   * @private
   */
  _executeTransmission(signal, options) {
    const transmissionId = `tx_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    // Simulate transmission based on power level and signal
    const success = this._simulateTransmission(signal, options);

    return {
      success: success,
      message: success ? 'Transmission successful' : 'Transmission failed',
      transmissionId: transmissionId
    };
  }

  /**
   * Simulate transmission (hardware interface would go here)
   * @private
   */
  _simulateTransmission(signal, options) {
    // Simulated success based on power level
    // Higher power = higher success rate
    const successProbability = 0.7 + (options.powerLevel / this.powerLevels) * 0.3;
    return Math.random() < successProbability;
  }

  /**
   * Calculate transmission range based on power level
   * @private
   */
  _calculateRange(level) {
    // Simulated range calculation
    const baseRange = 5; // cm
    const maxRange = 50; // cm
    return Math.round(baseRange + (level / (this.powerLevels - 1)) * (maxRange - baseRange));
  }

  /**
   * Get power level description
   * @private
   */
  _getPowerDescription(level) {
    const descriptions = [
      'Minimal (very short range)',
      'Very Low (short range)',
      'Low (reduced range)',
      'Below Medium',
      'Medium (standard range)',
      'Above Medium',
      'High (extended range)',
      'Maximum (full power)'
    ];
    return descriptions[level] || 'Unknown';
  }
}

module.exports = TransmissionController;
