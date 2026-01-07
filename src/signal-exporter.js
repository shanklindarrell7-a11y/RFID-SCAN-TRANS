/**
 * Multi-Format Export System
 * Export RFID signals to various formats
 */

class SignalExporter {
  constructor() {
    this.formats = ['flipper', 'csv', 'wiegand', 'proxmark3', 'json'];
  }

  /**
   * Export signal to Flipper Zero format (.sub)
   * @param {Object} signal - Signal data
   * @returns {string} Flipper format content
   */
  exportToFlipper(signal) {
    const lines = [
      'Filetype: Flipper SubGhz RAW File',
      'Version: 1',
      `Frequency: ${signal.frequency}`,
      `Preset: ${signal.modulation || 'FuriHalSubGhzPresetOok650Async'}`,
      `Protocol: ${signal.protocol}`,
      `Bit: ${signal.bitrate || 64}`,
      `Key: ${signal.uid || '00 00 00 00'}`,
      `Data: ${signal.data || 'RAW_Data'}`,
      ''
    ];
    return lines.join('\n');
  }

  /**
   * Export signal to CSV format
   * @param {Object} signal - Signal data
   * @returns {string} CSV format content
   */
  exportToCSV(signal) {
    const headers = 'ID,Name,Protocol,UID,Frequency,Modulation,Bitrate,Timestamp,SignalStrength,Notes';
    const row = [
      signal.id,
      `"${signal.name}"`,
      signal.protocol,
      signal.uid,
      signal.frequency,
      signal.modulation,
      signal.bitrate,
      signal.timestamp,
      signal.signalStrength,
      `"${signal.notes || ''}"`
    ].join(',');
    
    return `${headers}\n${row}`;
  }

  /**
   * Export multiple signals to CSV database
   * @param {Array} signals - Array of signals
   * @returns {string} CSV database content
   */
  exportMultipleToCSV(signals) {
    const headers = 'ID,Name,Protocol,UID,Frequency,Modulation,Bitrate,Timestamp,SignalStrength,Notes';
    const rows = signals.map(signal => {
      return [
        signal.id,
        `"${signal.name}"`,
        signal.protocol,
        signal.uid,
        signal.frequency,
        signal.modulation,
        signal.bitrate,
        signal.timestamp,
        signal.signalStrength,
        `"${signal.notes || ''}"`
      ].join(',');
    });
    
    return [headers, ...rows].join('\n');
  }

  /**
   * Export signal to Wiegand format
   * @param {Object} signal - Signal data
   * @returns {string} Wiegand format content
   */
  exportToWiegand(signal) {
    // Standard Wiegand 26-bit format
    const lines = [
      '# Wiegand Format Export',
      `# Protocol: ${signal.protocol}`,
      `# Timestamp: ${signal.timestamp}`,
      '',
      '# Wiegand Data',
      `Facility Code: ${this._extractFacilityCode(signal.uid)}`,
      `Card Number: ${this._extractCardNumber(signal.uid)}`,
      `Raw Hex: ${signal.uid}`,
      `Binary: ${this._hexToBinary(signal.uid)}`,
      ''
    ];
    return lines.join('\n');
  }

  /**
   * Export signal to Proxmark3 command format
   * @param {Object} signal - Signal data
   * @returns {string} Proxmark3 commands
   */
  exportToProxmark3(signal) {
    const commands = [
      '# Proxmark3 Commands',
      `# Signal: ${signal.name}`,
      `# Protocol: ${signal.protocol}`,
      '',
      '# Read command:',
      this._getProxmark3ReadCommand(signal.protocol),
      '',
      '# Clone command:',
      this._getProxmark3CloneCommand(signal),
      '',
      '# Simulate command:',
      this._getProxmark3SimulateCommand(signal),
      ''
    ];
    return commands.join('\n');
  }

  /**
   * Export signal to JSON format
   * @param {Object} signal - Signal data
   * @returns {string} JSON content
   */
  exportToJSON(signal) {
    const exportData = {
      version: '1.0.0',
      exportDate: new Date().toISOString(),
      signal: {
        id: signal.id,
        name: signal.name,
        protocol: signal.protocol,
        uid: signal.uid,
        data: signal.data,
        frequency: signal.frequency,
        modulation: signal.modulation,
        bitrate: signal.bitrate,
        timestamp: signal.timestamp,
        signalStrength: signal.signalStrength,
        notes: signal.notes,
        metadata: signal.metadata
      }
    };
    return JSON.stringify(exportData, null, 2);
  }

  /**
   * Export multiple signals to JSON
   * @param {Array} signals - Array of signals
   * @returns {string} JSON content
   */
  exportMultipleToJSON(signals) {
    const exportData = {
      version: '1.0.0',
      exportDate: new Date().toISOString(),
      count: signals.length,
      signals: signals
    };
    return JSON.stringify(exportData, null, 2);
  }

  /**
   * Batch export signals in multiple formats
   * @param {Array} signals - Array of signals
   * @param {Array} formats - Array of format names
   * @returns {Object} Exported data by format
   */
  batchExport(signals, formats = this.formats) {
    const exports = {};

    formats.forEach(format => {
      switch (format) {
        case 'flipper':
          exports.flipper = signals.map(s => this.exportToFlipper(s));
          break;
        case 'csv':
          exports.csv = this.exportMultipleToCSV(signals);
          break;
        case 'wiegand':
          exports.wiegand = signals.map(s => this.exportToWiegand(s));
          break;
        case 'proxmark3':
          exports.proxmark3 = signals.map(s => this.exportToProxmark3(s));
          break;
        case 'json':
          exports.json = this.exportMultipleToJSON(signals);
          break;
      }
    });

    return exports;
  }

  /**
   * Extract facility code from UID
   * @private
   */
  _extractFacilityCode(uid) {
    // Simplified extraction - real implementation would parse actual Wiegand format
    if (!uid || uid.length < 4) return '000';
    return parseInt(uid.substring(0, 2), 16).toString().padStart(3, '0');
  }

  /**
   * Extract card number from UID
   * @private
   */
  _extractCardNumber(uid) {
    // Simplified extraction - real implementation would parse actual Wiegand format
    if (!uid || uid.length < 4) return '00000';
    return parseInt(uid.substring(2), 16).toString().padStart(5, '0');
  }

  /**
   * Convert hex to binary
   * @private
   */
  _hexToBinary(hex) {
    if (!hex) return '';
    return hex.split('').map(h => {
      return parseInt(h, 16).toString(2).padStart(4, '0');
    }).join('');
  }

  /**
   * Get Proxmark3 read command for protocol
   * @private
   */
  _getProxmark3ReadCommand(protocol) {
    const commands = {
      'EM410x': 'lf em 410x read',
      'HID': 'lf hid read',
      'T55xx': 'lf t55xx read',
      'Indala': 'lf indala read',
      'AWID': 'lf awid read',
      'default': 'lf search'
    };
    return commands[protocol] || commands.default;
  }

  /**
   * Get Proxmark3 clone command
   * @private
   */
  _getProxmark3CloneCommand(signal) {
    return `lf ${signal.protocol.toLowerCase()} clone --id ${signal.uid}`;
  }

  /**
   * Get Proxmark3 simulate command
   * @private
   */
  _getProxmark3SimulateCommand(signal) {
    return `lf ${signal.protocol.toLowerCase()} sim --id ${signal.uid}`;
  }
}

module.exports = SignalExporter;
