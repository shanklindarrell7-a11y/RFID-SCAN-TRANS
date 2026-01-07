/**
 * RFIDExporter - Module for exporting RFID tag data to various formats
 * Supports: .sub (Flipper SubGHz), .csv, .wiegand, .pm3 (Proxmark3), .json
 */
class RFIDExporter {
  /**
   * Export tag(s) to specified format
   * @param {Array|Object} tags - Tag or array of tags to export
   * @param {string} format - Export format (sub, csv, wiegand, pm3, json)
   * @returns {Object} Export result with data
   */
  export(tags, format) {
    const tagArray = Array.isArray(tags) ? tags : [tags];

    if (tagArray.length === 0) {
      return {
        success: false,
        error: 'No tags to export'
      };
    }

    const exporters = {
      'sub': this.exportToSub.bind(this),
      'csv': this.exportToCsv.bind(this),
      'wiegand': this.exportToWiegand.bind(this),
      'pm3': this.exportToPm3.bind(this),
      'json': this.exportToJson.bind(this)
    };

    const exporter = exporters[format.toLowerCase()];
    if (!exporter) {
      return {
        success: false,
        error: `Unsupported format: ${format}`
      };
    }

    try {
      const data = exporter(tagArray);
      return {
        success: true,
        format: format,
        data: data,
        tagCount: tagArray.length,
        timestamp: new Date().toISOString()
      };
    } catch (error) {
      return {
        success: false,
        error: error.message
      };
    }
  }

  /**
   * Export to Flipper SubGHz format (.sub)
   * @param {Array} tags - Tags to export
   * @returns {string} SubGHz format data
   */
  exportToSub(tags) {
    let output = '# Flipper SubGHz RFID Export\n';
    output += `# Generated: ${new Date().toISOString()}\n`;
    output += `# Tag Count: ${tags.length}\n\n`;

    tags.forEach((tag, index) => {
      output += `# Tag ${index + 1}\n`;
      output += `Filetype: Flipper SubGHz RAW File\n`;
      output += `Version: 1\n`;
      output += `Frequency: ${this.parseFrequency(tag.frequency)}\n`;
      output += `Preset: FuriHalSubGhzPresetOok650Async\n`;
      output += `Protocol: ${tag.protocol}\n`;
      output += `UID: ${tag.uid}\n`;
      output += `Data: ${tag.rawData}\n`;
      output += `\n`;
    });

    return output;
  }

  /**
   * Export to CSV format (.csv)
   * @param {Array} tags - Tags to export
   * @returns {string} CSV format data
   */
  exportToCsv(tags) {
    let output = 'ID,Timestamp,Protocol,Frequency,UID,RawData,SignalStrength\n';

    tags.forEach(tag => {
      const signalStrength = tag.metadata?.signalStrength || 0;
      output += `"${tag.id}","${tag.timestamp}","${tag.protocol}","${tag.frequency}","${tag.uid}","${tag.rawData}",${signalStrength}\n`;
    });

    return output;
  }

  /**
   * Export to Wiegand format (.wiegand)
   * @param {Array} tags - Tags to export
   * @returns {string} Wiegand format data
   */
  exportToWiegand(tags) {
    let output = '# Wiegand Format Export\n';
    output += `# Generated: ${new Date().toISOString()}\n\n`;

    tags.forEach((tag, index) => {
      // Convert UID to Wiegand format (26-bit standard)
      const wiegandData = this.convertToWiegand(tag.uid);
      
      output += `# Tag ${index + 1}: ${tag.id}\n`;
      output += `Protocol: ${tag.protocol}\n`;
      output += `UID: ${tag.uid}\n`;
      output += `Wiegand26: ${wiegandData.wiegand26}\n`;
      output += `FacilityCode: ${wiegandData.facilityCode}\n`;
      output += `CardNumber: ${wiegandData.cardNumber}\n`;
      output += `Binary: ${wiegandData.binary}\n`;
      output += `\n`;
    });

    return output;
  }

  /**
   * Export to Proxmark3 format (.pm3)
   * @param {Array} tags - Tags to export
   * @returns {string} Proxmark3 format data
   */
  exportToPm3(tags) {
    let output = '# Proxmark3 RFID Tag Export\n';
    output += `# Generated: ${new Date().toISOString()}\n\n`;

    tags.forEach((tag, index) => {
      output += `# Tag ${index + 1}\n`;
      output += `[Tag]\n`;
      output += `Protocol=${tag.protocol}\n`;
      output += `UID=${tag.uid}\n`;
      output += `Data=${tag.rawData}\n`;
      output += `Frequency=${tag.frequency}\n`;
      
      // Add Proxmark3-specific commands
      if (tag.protocol === 'EM4100') {
        output += `Command=lf em 410x clone --id ${tag.uid}\n`;
      } else if (tag.protocol === 'HID') {
        output += `Command=lf hid clone --raw ${tag.rawData}\n`;
      } else if (tag.protocol === 'Mifare') {
        output += `Command=hf mf cload --1k -f ${tag.id}\n`;
      }
      
      output += `\n`;
    });

    return output;
  }

  /**
   * Export to JSON format (.json)
   * @param {Array} tags - Tags to export
   * @returns {string} JSON format data
   */
  exportToJson(tags) {
    const exportData = {
      exportVersion: '1.0',
      exportTimestamp: new Date().toISOString(),
      tagCount: tags.length,
      tags: tags.map(tag => ({
        id: tag.id,
        timestamp: tag.timestamp,
        protocol: tag.protocol,
        frequency: tag.frequency,
        uid: tag.uid,
        rawData: tag.rawData,
        metadata: tag.metadata || {}
      }))
    };

    return JSON.stringify(exportData, null, 2);
  }

  /**
   * Parse frequency to numeric value
   * @param {string} frequency - Frequency string (e.g., "125kHz")
   * @returns {number} Numeric frequency
   */
  parseFrequency(frequency) {
    const match = frequency.match(/(\d+)/);
    return match ? parseInt(match[1]) * 1000 : 125000;
  }

  /**
   * Convert UID to Wiegand format
   * @param {string} uid - Tag UID
   * @returns {Object} Wiegand format data
   */
  convertToWiegand(uid) {
    // Simple conversion - extract facility code and card number
    // In real implementation, would use actual Wiegand protocol conversion
    const uidNum = parseInt(uid.substring(0, 8), 16);
    const facilityCode = (uidNum >> 16) & 0xFF;
    const cardNumber = uidNum & 0xFFFF;
    
    // Generate 26-bit Wiegand
    const wiegand26 = ((facilityCode & 0xFF) << 17) | (cardNumber & 0xFFFF);
    const binary = wiegand26.toString(2).padStart(26, '0');

    return {
      wiegand26: wiegand26.toString(),
      facilityCode: facilityCode.toString(),
      cardNumber: cardNumber.toString(),
      binary: binary
    };
  }

  /**
   * Get list of supported export formats
   * @returns {Array} Supported format names
   */
  getSupportedFormats() {
    return ['sub', 'csv', 'wiegand', 'pm3', 'json'];
  }
}

module.exports = RFIDExporter;
