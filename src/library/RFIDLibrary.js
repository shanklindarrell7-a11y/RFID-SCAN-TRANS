/**
 * RFIDLibrary - Module for storing and managing RFID tag data
 */
class RFIDLibrary {
  constructor() {
    this.tags = new Map();
  }

  /**
   * Store a captured tag in the library
   * @param {Object} tag - Tag data to store
   * @returns {boolean} Success status
   */
  store(tag) {
    if (!tag || !tag.id) {
      return false;
    }

    this.tags.set(tag.id, {
      ...tag,
      storedAt: new Date().toISOString()
    });

    return true;
  }

  /**
   * Retrieve a tag by ID
   * @param {string} tagId - Tag identifier
   * @returns {Object|null} Tag data or null
   */
  retrieve(tagId) {
    return this.tags.get(tagId) || null;
  }

  /**
   * Get all stored tags
   * @returns {Array} List of all tags
   */
  getAll() {
    return Array.from(this.tags.values());
  }

  /**
   * Search tags by criteria
   * @param {Object} criteria - Search criteria
   * @returns {Array} Matching tags
   */
  search(criteria) {
    const allTags = this.getAll();
    
    return allTags.filter(tag => {
      if (criteria.protocol && tag.protocol !== criteria.protocol) {
        return false;
      }
      if (criteria.uid && tag.uid !== criteria.uid) {
        return false;
      }
      if (criteria.frequency && tag.frequency !== criteria.frequency) {
        return false;
      }
      return true;
    });
  }

  /**
   * Delete a tag from library
   * @param {string} tagId - Tag identifier
   * @returns {boolean} Success status
   */
  delete(tagId) {
    return this.tags.delete(tagId);
  }

  /**
   * Get library size
   * @returns {number} Number of stored tags
   */
  size() {
    return this.tags.size;
  }

  /**
   * Clear all tags from library
   */
  clear() {
    this.tags.clear();
  }
}

module.exports = RFIDLibrary;
