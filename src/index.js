/**
 * RFID-SCAN-TRANS Main Application
 * Flipper Zero app for RFID sniffing with signal storage and transmission
 */

const SignalStorage = require('./signal-storage');
const SignalExporter = require('./signal-exporter');
const SignalDuplicator = require('./signal-duplicator');
const TransmissionController = require('./transmission-controller');
const ProfessionalFeatures = require('./professional-features');

class RFIDScanTrans {
  constructor() {
    this.storage = new SignalStorage();
    this.exporter = new SignalExporter();
    this.duplicator = new SignalDuplicator();
    this.transmissionController = new TransmissionController();
    this.professionalFeatures = new ProfessionalFeatures();
    this.version = '1.0.0';
  }

  /**
   * Capture a new RFID signal
   * @param {Object} captureData - Raw capture data
   * @returns {Object} Capture result with signal ID
   */
  captureSignal(captureData) {
    // Auto-classify the signal
    const classification = this.professionalFeatures.autoClassify(captureData);
    
    // Enhance signal data with classification
    const enhancedSignal = {
      ...captureData,
      protocol: captureData.protocol || classification.detectedProtocol || 'Unknown',
      metadata: {
        ...captureData.metadata,
        classification: classification,
        captureVersion: this.version
      }
    };

    // Validate the signal
    const validation = this.professionalFeatures.validateSignal(enhancedSignal);
    enhancedSignal.validation = validation;

    // Store the signal
    const signalId = this.storage.addSignal(enhancedSignal);

    return {
      success: true,
      signalId: signalId,
      signal: this.storage.getSignal(signalId),
      classification: classification,
      validation: validation
    };
  }

  /**
   * Search for signals
   * @param {Object} criteria - Search criteria
   * @returns {Array} Matching signals
   */
  searchSignals(criteria) {
    return this.storage.searchSignals(criteria);
  }

  /**
   * Export signal in specified format
   * @param {string} signalId - Signal ID
   * @param {string} format - Export format
   * @returns {Object} Export result
   */
  exportSignal(signalId, format) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    let exportData = null;

    switch (format.toLowerCase()) {
      case 'flipper':
        exportData = this.exporter.exportToFlipper(signal);
        break;
      case 'csv':
        exportData = this.exporter.exportToCSV(signal);
        break;
      case 'wiegand':
        exportData = this.exporter.exportToWiegand(signal);
        break;
      case 'proxmark3':
        exportData = this.exporter.exportToProxmark3(signal);
        break;
      case 'json':
        exportData = this.exporter.exportToJSON(signal);
        break;
      default:
        return { success: false, error: 'Unknown format' };
    }

    return {
      success: true,
      format: format,
      data: exportData,
      filename: `${signal.name}.${this._getExtension(format)}`
    };
  }

  /**
   * Export multiple signals
   * @param {Array} signalIds - Array of signal IDs
   * @param {Array} formats - Array of formats
   * @returns {Object} Batch export result
   */
  batchExport(signalIds, formats) {
    const signals = signalIds.map(id => this.storage.getSignal(id)).filter(s => s);
    
    if (signals.length === 0) {
      return { success: false, error: 'No valid signals found' };
    }

    const exports = this.exporter.batchExport(signals, formats);

    return {
      success: true,
      count: signals.length,
      formats: formats,
      exports: exports
    };
  }

  /**
   * Clone signal to writable card
   * @param {string} signalId - Signal ID
   * @param {Object} options - Clone options
   * @returns {Object} Clone result
   */
  cloneSignal(signalId, options = {}) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    return this.duplicator.clone(signal, options);
  }

  /**
   * Emulate signal (continuous broadcast)
   * @param {string} signalId - Signal ID
   * @param {Object} options - Emulation options
   * @returns {Object} Emulation controller
   */
  emulateSignal(signalId, options = {}) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    return this.duplicator.emulate(signal, options);
  }

  /**
   * Replay signal (one-time transmission)
   * @param {string} signalId - Signal ID
   * @param {Object} options - Replay options
   * @returns {Object} Replay result
   */
  replaySignal(signalId, options = {}) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    // Use transmission controller for replay
    const powerLevel = options.powerLevel || this.transmissionController.getPowerLevel();
    const repeatCount = options.repeatCount || 1;
    const delay = options.delay || 0;

    return this.transmissionController.transmitWithRepeat(signal, repeatCount, delay);
  }

  /**
   * Generate signal variants
   * @param {string} signalId - Base signal ID
   * @param {Object} options - Generation options
   * @returns {Array} Generated signal IDs
   */
  generateVariants(signalId, options = {}) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    const variants = this.duplicator.generate(signal, options);
    
    // Store all variants
    const variantIds = variants.map(variant => this.storage.addSignal(variant));

    return {
      success: true,
      count: variants.length,
      baseSignalId: signalId,
      variantIds: variantIds
    };
  }

  /**
   * Set transmission power level
   * @param {number} level - Power level (0-7)
   * @returns {Object} Result
   */
  setPowerLevel(level) {
    const success = this.transmissionController.setPowerLevel(level);
    return {
      success: success,
      currentLevel: this.transmissionController.getPowerLevel(),
      info: this.transmissionController.getPowerLevelInfo()
    };
  }

  /**
   * Get current transmission power level
   * @returns {Object} Power level information
   */
  getPowerLevel() {
    return this.transmissionController.getPowerLevelInfo();
  }

  /**
   * Toggle favorite status for a signal
   * @param {string} signalId - Signal ID
   * @returns {Object} Result
   */
  toggleFavorite(signalId) {
    const newStatus = this.storage.toggleFavorite(signalId);
    return {
      success: true,
      signalId: signalId,
      isFavorite: newStatus
    };
  }

  /**
   * Get all favorite signals
   * @returns {Array} Favorite signals
   */
  getFavorites() {
    return this.storage.getFavorites();
  }

  /**
   * Get transmission statistics
   * @returns {Object} Statistics
   */
  getTransmissionStats() {
    return this.transmissionController.getStats();
  }

  /**
   * Validate a signal
   * @param {string} signalId - Signal ID
   * @returns {Object} Validation result
   */
  validateSignal(signalId) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    return this.professionalFeatures.validateSignal(signal);
  }

  /**
   * Classify a signal
   * @param {string} signalId - Signal ID
   * @returns {Object} Classification result
   */
  classifySignal(signalId) {
    const signal = this.storage.getSignal(signalId);
    
    if (!signal) {
      return { success: false, error: 'Signal not found' };
    }

    return this.professionalFeatures.autoClassify(signal);
  }

  /**
   * Create backup of all signals
   * @returns {Object} Backup data
   */
  createBackup() {
    return this.storage.backup();
  }

  /**
   * Restore from backup
   * @param {Object} backupData - Backup data
   * @returns {Object} Result
   */
  restoreBackup(backupData) {
    const success = this.storage.restore(backupData);
    return {
      success: success,
      signalCount: this.storage.count()
    };
  }

  /**
   * Get application info
   * @returns {Object} Application information
   */
  getInfo() {
    return {
      name: 'RFID-SCAN-TRANS',
      version: this.version,
      description: 'Flipper Zero app for RFID sniffing with signal storage and transmission',
      features: [
        'Signal Storage System',
        'Multi-Format Export',
        'Duplication Modes',
        'Transmission Control',
        'Professional Features'
      ],
      stats: {
        totalSignals: this.storage.count(),
        favorites: this.storage.getFavorites().length,
        transmissions: this.transmissionController.getStats()
      }
    };
  }

  /**
   * Get file extension for format
   * @private
   */
  _getExtension(format) {
    const extensions = {
      flipper: 'sub',
      csv: 'csv',
      wiegand: 'txt',
      proxmark3: 'txt',
      json: 'json'
    };
    return extensions[format.toLowerCase()] || 'txt';
  }
}

module.exports = RFIDScanTrans;
