/**
 * RFIDAnalyzer - Module for analyzing RFID tag data
 */
class RFIDAnalyzer {
  /**
   * Analyze a single tag
   * @param {Object} tag - Tag to analyze
   * @returns {Object} Analysis results
   */
  analyze(tag) {
    return {
      tagId: tag.id,
      analysis: {
        protocol: this.analyzeProtocol(tag),
        security: this.analyzeSecurity(tag),
        dataIntegrity: this.analyzeDataIntegrity(tag),
        compatibility: this.analyzeCompatibility(tag)
      },
      timestamp: new Date().toISOString()
    };
  }

  /**
   * Analyze protocol information
   * @param {Object} tag - Tag data
   * @returns {Object} Protocol analysis
   */
  analyzeProtocol(tag) {
    const protocols = {
      'EM4100': { type: 'Low Frequency', readonly: true, cloneable: true },
      'HID': { type: 'Low Frequency', readonly: true, cloneable: true },
      'Mifare': { type: 'High Frequency', readonly: false, cloneable: true },
      'NTAG': { type: 'High Frequency', readonly: false, cloneable: true }
    };

    const protocolInfo = protocols[tag.protocol] || { type: 'Unknown', readonly: true, cloneable: false };

    return {
      name: tag.protocol,
      ...protocolInfo,
      frequency: tag.frequency
    };
  }

  /**
   * Analyze security features
   * @param {Object} tag - Tag data
   * @returns {Object} Security analysis
   */
  analyzeSecurity(tag) {
    const hasEncryption = ['Mifare', 'DESFire'].includes(tag.protocol);
    const hasAuthentication = ['Mifare', 'DESFire', 'iClass'].includes(tag.protocol);

    return {
      encrypted: hasEncryption,
      authenticated: hasAuthentication,
      securityLevel: hasEncryption && hasAuthentication ? 'high' : hasAuthentication ? 'medium' : 'low'
    };
  }

  /**
   * Analyze data integrity
   * @param {Object} tag - Tag data
   * @returns {Object} Data integrity analysis
   */
  analyzeDataIntegrity(tag) {
    const hasRawData = tag.rawData && tag.rawData.length > 0;
    const hasUID = tag.uid && tag.uid.length > 0;

    return {
      complete: hasRawData && hasUID,
      confidence: hasRawData && hasUID ? 1.0 : hasRawData ? 0.7 : 0.3,
      dataLength: tag.rawData ? tag.rawData.length : 0
    };
  }

  /**
   * Analyze compatibility for cloning/emulation
   * @param {Object} tag - Tag data
   * @returns {Object} Compatibility analysis
   */
  analyzeCompatibility(tag) {
    const protocolAnalysis = this.analyzeProtocol(tag);

    return {
      canClone: protocolAnalysis.cloneable,
      canEmulate: true,
      canReplay: true,
      canGenerate: tag.protocol !== 'Unknown',
      supportedOperations: this.getSupportedOperations(tag)
    };
  }

  /**
   * Get supported operations for a tag
   * @param {Object} tag - Tag data
   * @returns {Array} List of supported operations
   */
  getSupportedOperations(tag) {
    const operations = ['read'];
    const protocolAnalysis = this.analyzeProtocol(tag);

    if (protocolAnalysis.cloneable) {
      operations.push('clone');
    }
    
    operations.push('emulate', 'replay');

    if (!protocolAnalysis.readonly) {
      operations.push('write');
    }

    return operations;
  }

  /**
   * Batch analyze multiple tags
   * @param {Array} tags - Array of tags to analyze
   * @returns {Array} Analysis results for all tags
   */
  batchAnalyze(tags) {
    return tags.map(tag => this.analyze(tag));
  }

  /**
   * Generate statistics from analyzed tags
   * @param {Array} tags - Array of tags
   * @returns {Object} Statistics
   */
  generateStatistics(tags) {
    const protocolCounts = {};
    const frequencyCounts = {};

    tags.forEach(tag => {
      protocolCounts[tag.protocol] = (protocolCounts[tag.protocol] || 0) + 1;
      frequencyCounts[tag.frequency] = (frequencyCounts[tag.frequency] || 0) + 1;
    });

    return {
      totalTags: tags.length,
      protocols: protocolCounts,
      frequencies: frequencyCounts,
      generatedAt: new Date().toISOString()
    };
  }
}

module.exports = RFIDAnalyzer;
