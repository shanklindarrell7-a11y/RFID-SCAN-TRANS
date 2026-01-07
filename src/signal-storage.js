/**
 * Signal Storage System
 * Manages RFID signal library with metadata, search, and favorites
 */

class SignalStorage {
  constructor() {
    this.signals = new Map();
    this.favorites = new Set();
    this.backupEnabled = true;
    this.autoSave = true;
  }

  /**
   * Add a signal to the library
   * @param {Object} signal - Signal data with metadata
   * @returns {string} Signal ID
   */
  addSignal(signal) {
    const id = this._generateId();
    const signalData = {
      id,
      name: signal.name || `Signal_${id}`,
      protocol: signal.protocol || 'Unknown',
      uid: signal.uid || '',
      data: signal.data || '',
      frequency: signal.frequency || 125000,
      modulation: signal.modulation || 'ASK',
      bitrate: signal.bitrate || 64,
      timestamp: new Date().toISOString(),
      signalStrength: signal.signalStrength || 0,
      notes: signal.notes || '',
      favorite: false,
      metadata: signal.metadata || {}
    };

    this.signals.set(id, signalData);
    
    if (this.autoSave) {
      this._autoSave(id, signalData);
    }

    return id;
  }

  /**
   * Get a signal by ID
   * @param {string} id - Signal ID
   * @returns {Object|null} Signal data or null
   */
  getSignal(id) {
    return this.signals.get(id) || null;
  }

  /**
   * Search signals by criteria
   * @param {Object} criteria - Search criteria
   * @returns {Array} Matching signals
   */
  searchSignals(criteria) {
    const results = [];
    
    for (const [id, signal] of this.signals) {
      let matches = true;

      if (criteria.name && !signal.name.toLowerCase().includes(criteria.name.toLowerCase())) {
        matches = false;
      }
      
      if (criteria.protocol && signal.protocol.toLowerCase() !== criteria.protocol.toLowerCase()) {
        matches = false;
      }
      
      if (criteria.uid && signal.uid !== criteria.uid) {
        matches = false;
      }

      if (criteria.favorite !== undefined && signal.favorite !== criteria.favorite) {
        matches = false;
      }

      if (matches) {
        results.push({ ...signal });
      }
    }

    return results;
  }

  /**
   * Filter signals by protocol
   * @param {string} protocol - Protocol name
   * @returns {Array} Filtered signals
   */
  filterByProtocol(protocol) {
    return this.searchSignals({ protocol });
  }

  /**
   * Get all favorite signals
   * @returns {Array} Favorite signals
   */
  getFavorites() {
    return this.searchSignals({ favorite: true });
  }

  /**
   * Toggle favorite status
   * @param {string} id - Signal ID
   * @returns {boolean} New favorite status
   */
  toggleFavorite(id) {
    const signal = this.signals.get(id);
    if (signal) {
      signal.favorite = !signal.favorite;
      if (signal.favorite) {
        this.favorites.add(id);
      } else {
        this.favorites.delete(id);
      }
      return signal.favorite;
    }
    return false;
  }

  /**
   * Update signal metadata
   * @param {string} id - Signal ID
   * @param {Object} updates - Fields to update
   * @returns {boolean} Success status
   */
  updateSignal(id, updates) {
    const signal = this.signals.get(id);
    if (signal) {
      Object.assign(signal, updates);
      if (this.autoSave) {
        this._autoSave(id, signal);
      }
      return true;
    }
    return false;
  }

  /**
   * Delete a signal
   * @param {string} id - Signal ID
   * @returns {boolean} Success status
   */
  deleteSignal(id) {
    this.favorites.delete(id);
    return this.signals.delete(id);
  }

  /**
   * Get all signals
   * @returns {Array} All signals
   */
  getAllSignals() {
    return Array.from(this.signals.values());
  }

  /**
   * Backup all signals
   * @returns {Object} Backup data
   */
  backup() {
    return {
      version: '1.0.0',
      timestamp: new Date().toISOString(),
      signals: Array.from(this.signals.entries()),
      favorites: Array.from(this.favorites)
    };
  }

  /**
   * Restore from backup
   * @param {Object} backupData - Backup data
   * @returns {boolean} Success status
   */
  restore(backupData) {
    try {
      this.signals = new Map(backupData.signals);
      this.favorites = new Set(backupData.favorites);
      return true;
    } catch (error) {
      return false;
    }
  }

  /**
   * Clear all signals
   */
  clear() {
    this.signals.clear();
    this.favorites.clear();
  }

  /**
   * Get signal count
   * @returns {number} Number of signals
   */
  count() {
    return this.signals.size;
  }

  /**
   * Generate unique ID
   * @private
   */
  _generateId() {
    return `signal_${Date.now()}_${Math.random().toString(36).substring(2, 11)}`;
  }

  /**
   * Auto-save signal to storage
   * @private
   */
  _autoSave(id, signal) {
    // Simulated auto-save functionality
    // In real implementation, this would write to SD card
  }
}

module.exports = SignalStorage;
