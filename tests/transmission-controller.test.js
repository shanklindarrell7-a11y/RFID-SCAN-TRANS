/**
 * Tests for Transmission Controller
 */

const TransmissionController = require('../src/transmission-controller');

describe('TransmissionController', () => {
  let controller;
  let testSignal;

  beforeEach(() => {
    controller = new TransmissionController();
    testSignal = {
      id: 'signal_123',
      name: 'Test Signal',
      protocol: 'EM410x',
      uid: '1234567890'
    };
  });

  test('should set power level', () => {
    expect(controller.setPowerLevel(5)).toBe(true);
    expect(controller.getPowerLevel()).toBe(5);
  });

  test('should reject invalid power level', () => {
    expect(controller.setPowerLevel(-1)).toBe(false);
    expect(controller.setPowerLevel(8)).toBe(false);
  });

  test('should get power level info', () => {
    controller.setPowerLevel(4);
    const info = controller.getPowerLevelInfo();
    
    expect(info.level).toBe(4);
    expect(info.percentage).toBeDefined();
    expect(info.range).toBeDefined();
    expect(info.description).toBeDefined();
  });

  test('should transmit signal', () => {
    const result = controller.transmit(testSignal);
    
    expect(result.timestamp).toBeDefined();
    expect(result.powerLevel).toBeDefined();
    expect(result.success).toBeDefined();
  });

  test('should transmit with custom power level', () => {
    const result = controller.transmit(testSignal, { powerLevel: 6 });
    
    expect(result.powerLevel).toBe(6);
  });

  test('should transmit with repeat count', () => {
    const result = controller.transmitWithRepeat(testSignal, 3);
    
    expect(result.results.length).toBe(3);
    expect(result.successRate).toBeDefined();
  });

  test('should create continuous transmission controller', () => {
    const continuous = controller.startContinuous(testSignal);
    
    expect(continuous.signal).toBe(testSignal);
    expect(continuous.start).toBeDefined();
    expect(continuous.stop).toBeDefined();
    expect(continuous.updatePowerLevel).toBeDefined();
    expect(continuous.getMonitoring).toBeDefined();
  });

  test('should start and stop continuous transmission', (done) => {
    const continuous = controller.startContinuous(testSignal, { interval: 100 });
    
    const startResult = continuous.start();
    expect(startResult.success).toBe(true);
    expect(continuous.isActive).toBe(true);

    setTimeout(() => {
      const stopResult = continuous.stop();
      expect(stopResult.success).toBe(true);
      expect(continuous.isActive).toBe(false);
      expect(stopResult.stats.transmissionCount).toBeGreaterThan(0);
      done();
    }, 250);
  });

  test('should update power level during continuous transmission', () => {
    const continuous = controller.startContinuous(testSignal);
    
    expect(continuous.updatePowerLevel(6)).toBe(true);
    expect(continuous.powerLevel).toBe(6);
    
    expect(continuous.updatePowerLevel(10)).toBe(false);
  });

  test('should track transmission statistics', () => {
    controller.transmit(testSignal);
    controller.transmit(testSignal);

    const stats = controller.getStats();
    
    expect(stats.totalTransmissions).toBeGreaterThanOrEqual(2);
    expect(stats.successRate).toBeDefined();
  });

  test('should reset statistics', () => {
    controller.transmit(testSignal);
    controller.resetStats();

    const stats = controller.getStats();
    expect(stats.totalTransmissions).toBe(0);
  });

  test('should prevent concurrent transmissions', () => {
    controller.isTransmitting = true;
    
    const result = controller.transmit(testSignal);
    expect(result.message).toContain('already in progress');
  });
});
