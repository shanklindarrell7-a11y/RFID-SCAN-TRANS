/**
 * Tests for Signal Duplicator
 */

const SignalDuplicator = require('../src/signal-duplicator');

describe('SignalDuplicator', () => {
  let duplicator;
  let testSignal;

  beforeEach(() => {
    duplicator = new SignalDuplicator();
    testSignal = {
      id: 'signal_123',
      name: 'Test Signal',
      protocol: 'T55xx',
      uid: '1234567890',
      metadata: { writable: true }
    };
  });

  test('should clone writable signal', () => {
    const result = duplicator.clone(testSignal);
    
    expect(result.mode).toBe('clone');
    expect(result.success).toBe(true);
    expect(result.writtenData).toBeDefined();
  });

  test('should reject non-writable signal for cloning', () => {
    const readOnlySignal = {
      ...testSignal,
      protocol: 'EM410x',
      metadata: {}
    };

    const result = duplicator.clone(readOnlySignal);
    
    expect(result.success).toBe(false);
    expect(result.message).toContain('validation failed');
  });

  test('should create emulation controller', () => {
    const controller = duplicator.emulate(testSignal);
    
    expect(controller.mode).toBe('emulate');
    expect(controller.start).toBeDefined();
    expect(controller.stop).toBeDefined();
    expect(controller.getStatus).toBeDefined();
  });

  test('should start and stop emulation', () => {
    const controller = duplicator.emulate(testSignal);
    
    const startResult = controller.start();
    expect(startResult.success).toBe(true);
    expect(controller.isActive).toBe(true);

    const stopResult = controller.stop();
    expect(stopResult.success).toBe(true);
    expect(controller.isActive).toBe(false);
  });

  test('should replay signal with repeat count', () => {
    const result = duplicator.replay(testSignal, { repeatCount: 3 });
    
    expect(result.mode).toBe('replay');
    expect(result.success).toBe(true);
    expect(result.transmissions).toBe(3);
  });

  test('should generate signal variants with increment', () => {
    const variants = duplicator.generate(testSignal, { 
      count: 5, 
      mode: 'increment' 
    });
    
    expect(variants.length).toBe(5);
    expect(variants[0].metadata.generatedFrom).toBe(testSignal.id);
    expect(variants[0].metadata.generationMode).toBe('increment');
  });

  test('should generate signal variants with decrement', () => {
    const variants = duplicator.generate(testSignal, { 
      count: 3, 
      mode: 'decrement' 
    });
    
    expect(variants.length).toBe(3);
    expect(variants[0].metadata.generationMode).toBe('decrement');
  });

  test('should generate signal variants with random', () => {
    const variants = duplicator.generate(testSignal, { 
      count: 3, 
      mode: 'random' 
    });
    
    expect(variants.length).toBe(3);
    expect(variants[0].metadata.generationMode).toBe('random');
  });

  test('should stop transmission', () => {
    const result = duplicator.stopTransmission();
    expect(result).toBe(true);
  });

  test('should prevent concurrent transmissions', () => {
    duplicator.isTransmitting = true;
    
    const result = duplicator.replay(testSignal);
    expect(result.message).toContain('transmission is active');
  });
});
