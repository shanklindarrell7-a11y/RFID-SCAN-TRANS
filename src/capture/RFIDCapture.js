/**
 * RFIDCapture - Module for capturing RFID tag data
 * Simulates reading RFID tags from Flipper Zero device
 */
class RFIDCapture {
  constructor() {
    this.capturedTags = [];
  }

  /**
   * Capture an RFID tag
   * @param {Object} tagData - Raw tag data
   * @returns {Object} Captured tag with metadata
   */
  capture(tagData) {
    const capturedTag = {
      id: this.generateTagId(),
      timestamp: new Date().toISOString(),
      rawData: tagData.rawData || '',
      protocol: tagData.protocol || 'EM4100',
      frequency: tagData.frequency || '125kHz',
      uid: tagData.uid || this.extractUID(tagData.rawData),
      metadata: {
        signalStrength: tagData.signalStrength || 0,
        readCount: 1
      }
    };

    this.capturedTags.push(capturedTag);
    return capturedTag;
  }

  /**
   * Scan for RFID tags (simulation)
   * @returns {Object|null} Detected tag or null
   */
  scan() {
    // Simulation: In real implementation, this would interface with Flipper Zero hardware
    console.log('Scanning for RFID tags...');
    return null;
  }

  /**
   * Extract UID from raw data
   * @param {string} rawData - Raw tag data
   * @returns {string} Extracted UID
   */
  extractUID(rawData) {
    if (!rawData) return '';
    // Simple extraction logic - in real implementation would parse based on protocol
    return rawData.substring(0, 16);
  }

  /**
   * Generate unique tag ID
   * @returns {string} Unique identifier
   */
  generateTagId() {
    return `tag_${Date.now()}_${Math.random().toString(36).substring(2, 11)}`;
  }

  /**
   * Get all captured tags
   * @returns {Array} List of captured tags
   */
  getCapturedTags() {
    return [...this.capturedTags];
  }

  /**
   * Clear captured tags
   */
  clear() {
    this.capturedTags = [];
  }
}

module.exports = RFIDCapture;
