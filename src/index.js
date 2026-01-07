/**
 * RFID-SCAN-TRANS
 * Main application integrating all modules
 * Workflow: Capture → Store → Analyze → Transmit/Duplicate → Export
 */

const RFIDCapture = require('./capture/RFIDCapture');
const RFIDLibrary = require('./library/RFIDLibrary');
const RFIDAnalyzer = require('./analysis/RFIDAnalyzer');
const RFIDTransmission = require('./transmission/RFIDTransmission');
const RFIDExporter = require('./export/RFIDExporter');

class RFIDScanTrans {
  constructor() {
    this.capture = new RFIDCapture();
    this.library = new RFIDLibrary();
    this.analyzer = new RFIDAnalyzer();
    this.transmission = new RFIDTransmission();
    this.exporter = new RFIDExporter();
  }

  /**
   * Complete workflow: Capture a tag and store it in the library
   * @param {Object} tagData - Raw tag data
   * @returns {Object} Result with captured and stored tag
   */
  captureAndStore(tagData) {
    // Step 1: Capture
    const capturedTag = this.capture.capture(tagData);
    
    // Step 2: Store
    const stored = this.library.store(capturedTag);
    
    return {
      success: stored,
      tag: capturedTag,
      message: stored ? 'Tag captured and stored successfully' : 'Failed to store tag'
    };
  }

  /**
   * Complete workflow: Analyze a stored tag
   * @param {string} tagId - Tag ID to analyze
   * @returns {Object} Analysis results
   */
  analyzeTag(tagId) {
    // Retrieve from library
    const tag = this.library.retrieve(tagId);
    
    if (!tag) {
      return {
        success: false,
        error: 'Tag not found in library'
      };
    }

    // Step 3: Analyze
    const analysis = this.analyzer.analyze(tag);
    
    return {
      success: true,
      tag: tag,
      analysis: analysis
    };
  }

  /**
   * Complete workflow: Clone a stored tag
   * @param {string} tagId - Tag ID to clone
   * @returns {Object} Clone operation result
   */
  cloneTag(tagId) {
    const tag = this.library.retrieve(tagId);
    
    if (!tag) {
      return {
        success: false,
        error: 'Tag not found in library'
      };
    }

    // Step 4: Transmit (Clone)
    return this.transmission.clone(tag);
  }

  /**
   * Complete workflow: Emulate a stored tag
   * @param {string} tagId - Tag ID to emulate
   * @returns {Object} Emulation operation result
   */
  emulateTag(tagId) {
    const tag = this.library.retrieve(tagId);
    
    if (!tag) {
      return {
        success: false,
        error: 'Tag not found in library'
      };
    }

    // Step 4: Transmit (Emulate)
    return this.transmission.emulate(tag);
  }

  /**
   * Complete workflow: Replay a stored tag
   * @param {string} tagId - Tag ID to replay
   * @param {number} count - Number of replays
   * @returns {Object} Replay operation result
   */
  replayTag(tagId, count = 1) {
    const tag = this.library.retrieve(tagId);
    
    if (!tag) {
      return {
        success: false,
        error: 'Tag not found in library'
      };
    }

    // Step 4: Transmit (Replay)
    return this.transmission.replay(tag, count);
  }

  /**
   * Generate a new tag and optionally store it
   * @param {Object} params - Generation parameters
   * @param {boolean} store - Whether to store in library
   * @returns {Object} Generated tag result
   */
  generateTag(params, store = true) {
    // Step 4: Transmit (Generate)
    const result = this.transmission.generate(params);
    
    if (result.success && store) {
      this.library.store(result.tag);
    }

    return result;
  }

  /**
   * Export tags from library
   * @param {Array|string} tagIds - Tag ID(s) to export, or 'all' for all tags
   * @param {string} format - Export format (sub, csv, wiegand, pm3, json)
   * @returns {Object} Export result
   */
  exportTags(tagIds, format) {
    let tags;

    if (tagIds === 'all') {
      tags = this.library.getAll();
    } else {
      const ids = Array.isArray(tagIds) ? tagIds : [tagIds];
      tags = ids.map(id => this.library.retrieve(id)).filter(tag => tag !== null);
    }

    if (tags.length === 0) {
      return {
        success: false,
        error: 'No tags found to export'
      };
    }

    // Step 5: Export
    return this.exporter.export(tags, format);
  }

  /**
   * Get library statistics
   * @returns {Object} Statistics about stored tags
   */
  getStatistics() {
    const tags = this.library.getAll();
    return this.analyzer.generateStatistics(tags);
  }

  /**
   * Search library for tags matching criteria
   * @param {Object} criteria - Search criteria
   * @returns {Array} Matching tags
   */
  searchLibrary(criteria) {
    return this.library.search(criteria);
  }

  /**
   * Get all tags from library
   * @returns {Array} All stored tags
   */
  getAllTags() {
    return this.library.getAll();
  }

  /**
   * Clear all data
   */
  clearAll() {
    this.capture.clear();
    this.library.clear();
    if (this.transmission.isEmulating()) {
      this.transmission.stopEmulation();
    }
  }
}

module.exports = RFIDScanTrans;

// CLI Interface (if run directly)
if (require.main === module) {
  console.log('═══════════════════════════════════════════════════');
  console.log('  RFID-SCAN-TRANS - Flipper Zero RFID Tool');
  console.log('═══════════════════════════════════════════════════');
  console.log('');
  console.log('Workflow: Capture → Store → Analyze → Transmit/Duplicate');
  console.log('');
  console.log('Signal Chain:');
  console.log('  RFID Tag → Capture → Library → [Clone/Emulate/Replay/Generate]');
  console.log('');
  console.log('Export Options:');
  console.log('  Library → Export → [.sub / .csv / .wiegand / .pm3 / .json]');
  console.log('');
  console.log('═══════════════════════════════════════════════════');
  console.log('');

  // Demonstration
  const app = new RFIDScanTrans();

  console.log('📡 Demo: Capturing sample RFID tag...');
  const result1 = app.captureAndStore({
    rawData: '1234567890ABCDEF',
    protocol: 'EM4100',
    frequency: '125kHz',
    uid: '1234567890AB',
    signalStrength: 85
  });
  console.log(`✓ Tag captured: ${result1.tag.id}`);
  console.log('');

  console.log('📊 Demo: Analyzing tag...');
  const analysis = app.analyzeTag(result1.tag.id);
  console.log(`✓ Protocol: ${analysis.analysis.analysis.protocol.name}`);
  console.log(`✓ Security Level: ${analysis.analysis.analysis.security.securityLevel}`);
  console.log(`✓ Cloneable: ${analysis.analysis.analysis.compatibility.canClone}`);
  console.log('');

  console.log('🔄 Demo: Generating new tag...');
  const generated = app.generateTag({ protocol: 'HID', frequency: '125kHz' });
  console.log(`✓ Generated tag: ${generated.tag.id}`);
  console.log(`✓ UID: ${generated.tag.uid}`);
  console.log('');

  console.log('📤 Demo: Exporting to JSON...');
  const exported = app.exportTags('all', 'json');
  console.log(`✓ Exported ${exported.tagCount} tags`);
  console.log('');

  console.log('📈 Library Statistics:');
  const stats = app.getStatistics();
  console.log(`✓ Total tags: ${stats.totalTags}`);
  console.log(`✓ Protocols: ${JSON.stringify(stats.protocols)}`);
  console.log('');

  console.log('✅ Demo complete! All modules working correctly.');
}
