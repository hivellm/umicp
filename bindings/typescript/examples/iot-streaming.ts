/**
 * UMICP TypeScript Bindings - IoT Data Streaming Example
 * Demonstrates real-time sensor data streaming using UMICP envelopes
 */

import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

interface SensorReading {
  sensorId: string;
  sensorType: 'temperature' | 'humidity' | 'pressure' | 'motion' | 'light';
  value: number;
  unit: string;
  location: string;
  timestamp: number;
  metadata: {
    batteryLevel?: number;
    signalStrength?: number;
    calibrationOffset?: number;
  };
}

interface StreamConfig {
  batchSize: number;
  flushInterval: number;
  compression: boolean;
  priority: 'low' | 'normal' | 'high' | 'critical';
}

class IoTDataStreamer {
  private sensorBuffer: SensorReading[] = [];
  private config: StreamConfig;
  private isStreaming: boolean = false;
  private flushTimer?: NodeJS.Timeout;
  private sequenceNumber: number = 0;

  constructor(config: Partial<StreamConfig> = {}) {
    this.config = {
      batchSize: 50,
      flushInterval: 5000, // 5 seconds
      compression: true,
      priority: 'normal',
      ...config
    };
  }

  // Add sensor reading to buffer
  addReading(reading: SensorReading): void {
    this.sensorBuffer.push(reading);

    // Auto-flush if buffer is full
    if (this.sensorBuffer.length >= this.config.batchSize) {
      this.flush();
    }
  }

  // Start streaming
  startStreaming(): void {
    if (this.isStreaming) {
      console.log('📡 Streaming already active');
      return;
    }

    this.isStreaming = true;
    console.log('📡 Starting IoT data streaming...');
    console.log(`   Batch size: ${this.config.batchSize}`);
    console.log(`   Flush interval: ${this.config.flushInterval}ms`);
    console.log(`   Compression: ${this.config.compression ? 'enabled' : 'disabled'}`);
    console.log(`   Priority: ${this.config.priority}`);
    console.log();

    // Set up periodic flush
    this.flushTimer = setInterval(() => {
      if (this.sensorBuffer.length > 0) {
        this.flush();
      }
    }, this.config.flushInterval);
  }

  // Stop streaming
  stopStreaming(): void {
    if (!this.isStreaming) {
      return;
    }

    this.isStreaming = false;
    console.log('🛑 Stopping IoT data streaming...');

    if (this.flushTimer) {
      clearInterval(this.flushTimer);
      this.flushTimer = undefined;
    }

    // Flush remaining data
    if (this.sensorBuffer.length > 0) {
      this.flush();
    }

    console.log('✅ Streaming stopped');
  }

  // Flush buffer to create envelope
  private flush(): void {
    if (this.sensorBuffer.length === 0) {
      return;
    }

    const batchData = [...this.sensorBuffer];
    this.sensorBuffer.length = 0; // Clear buffer

    const envelope = this.createBatchEnvelope(batchData);
    const serialized = envelope.serialize();

    console.log(`📤 Flushed ${batchData.length} sensor readings`);
    console.log(`   Envelope size: ${serialized.length} bytes`);
    console.log(`   Sequence: ${this.sequenceNumber}`);

    // In a real application, this would be sent to a server
    // For demo purposes, we'll just log the envelope
    console.log(`   Sample reading: ${batchData[0].sensorType}=${batchData[0].value}${batchData[0].unit}`);
    console.log();
  }

  // Create batch envelope from sensor readings
  private createBatchEnvelope(readings: SensorReading[]): any {
    this.sequenceNumber++;

    // Calculate batch statistics
    const sensorTypes = [...new Set(readings.map(r => r.sensorType))];
    const locations = [...new Set(readings.map(r => r.location))];
    const avgTimestamp = readings.reduce((sum, r) => sum + r.timestamp, 0) / readings.length;

    // Create envelope with batch data
    const envelope = UMICP.createEnvelope({
      from: `iot-gateway-${Date.now().toString().slice(-6)}`,
      to: 'iot-data-collector',
      operation: OperationType.DATA,
      messageId: `batch-${this.sequenceNumber}-${Date.now()}`,
      capabilities: {
        'message_type': 'sensor_batch',
        'batch_size': readings.length.toString(),
        'sequence_number': this.sequenceNumber.toString(),
        'sensor_types': sensorTypes.join(','),
        'locations': locations.join(','),
        'avg_timestamp': avgTimestamp.toString(),
        'compression': this.config.compression.toString(),
        'priority': this.config.priority,
        'data_format': 'json',
        'protocol_version': '1.0'
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: JSON.stringify(readings).length,
        encoding: EncodingType.UTF8,
        count: readings.length
      }
    });

    return envelope;
  }

  // Get streaming statistics
  getStats(): any {
    return {
      isStreaming: this.isStreaming,
      bufferSize: this.sensorBuffer.length,
      batchSize: this.config.batchSize,
      flushInterval: this.config.flushInterval,
      totalBatchesSent: this.sequenceNumber,
      compressionEnabled: this.config.compression,
      priority: this.config.priority
    };
  }
}

class IoTSensorSimulator {
  private sensors: SensorConfig[] = [];
  private streamer: IoTDataStreamer;
  private simulationTimer?: NodeJS.Timeout;

  constructor(streamer: IoTDataStreamer) {
    this.streamer = streamer;
    this.initializeSensors();
  }

  private initializeSensors(): void {
    // Temperature sensors
    this.sensors.push(
      { id: 'temp-001', type: 'temperature', location: 'warehouse-a', baseValue: 22, variance: 3, unit: '°C' },
      { id: 'temp-002', type: 'temperature', location: 'warehouse-b', baseValue: 18, variance: 2, unit: '°C' },
      { id: 'temp-003', type: 'temperature', location: 'office', baseValue: 24, variance: 1, unit: '°C' }
    );

    // Humidity sensors
    this.sensors.push(
      { id: 'hum-001', type: 'humidity', location: 'warehouse-a', baseValue: 65, variance: 10, unit: '%' },
      { id: 'hum-002', type: 'humidity', location: 'warehouse-b', baseValue: 70, variance: 8, unit: '%' }
    );

    // Motion sensors
    this.sensors.push(
      { id: 'motion-001', type: 'motion', location: 'entrance', baseValue: 0, variance: 1, unit: 'boolean' },
      { id: 'motion-002', type: 'motion', location: 'storage-room', baseValue: 0, variance: 1, unit: 'boolean' }
    );

    // Light sensors
    this.sensors.push(
      { id: 'light-001', type: 'light', location: 'warehouse-a', baseValue: 500, variance: 100, unit: 'lux' }
    );
  }

  // Start simulation
  startSimulation(interval: number = 1000): void {
    console.log(`🎭 Starting IoT sensor simulation with ${this.sensors.length} sensors`);
    console.log(`   Update interval: ${interval}ms`);
    console.log();

    this.simulationTimer = setInterval(() => {
      this.generateReadings();
    }, interval);
  }

  // Stop simulation
  stopSimulation(): void {
    if (this.simulationTimer) {
      clearInterval(this.simulationTimer);
      this.simulationTimer = undefined;
    }
    console.log('🛑 Sensor simulation stopped');
  }

  // Generate sensor readings
  private generateReadings(): void {
    const timestamp = Date.now();

    for (const sensor of this.sensors) {
      const reading: SensorReading = {
        sensorId: sensor.id,
        sensorType: sensor.type as any,
        value: this.generateSensorValue(sensor),
        unit: sensor.unit,
        location: sensor.location,
        timestamp,
        metadata: {
          batteryLevel: 85 + Math.random() * 15, // 85-100%
          signalStrength: -30 + Math.random() * 20, // -30 to -10 dBm
          calibrationOffset: (Math.random() - 0.5) * 0.1 // Small offset
        }
      };

      this.streamer.addReading(reading);
    }
  }

  // Generate realistic sensor values
  private generateSensorValue(sensor: SensorConfig): number {
    let baseValue = sensor.baseValue;
    let variance = sensor.variance;

    // Time-based variations
    const hour = new Date().getHours();
    if (sensor.type === 'temperature') {
      // Temperature varies with time of day
      baseValue += Math.sin((hour - 6) * Math.PI / 12) * 2;
    } else if (sensor.type === 'light') {
      // Light varies with time of day
      baseValue = Math.max(10, baseValue * Math.sin((hour - 6) * Math.PI / 12));
    }

    // Add random variation
    const value = baseValue + (Math.random() - 0.5) * variance * 2;

    // Handle special cases
    if (sensor.type === 'motion') {
      return Math.random() > 0.95 ? 1 : 0; // 5% chance of motion
    } else if (sensor.type === 'humidity') {
      return Math.max(0, Math.min(100, value)); // Clamp to 0-100%
    }

    return Math.max(0, value);
  }
}

interface SensorConfig {
  id: string;
  type: string;
  location: string;
  baseValue: number;
  variance: number;
  unit: string;
}

async function iotStreamingExample() {
  console.log('🌡️ UMICP IoT Data Streaming Example');
  console.log('=====================================');

  // Create streamer with custom config
  const streamer = new IoTDataStreamer({
    batchSize: 20,
    flushInterval: 3000,
    compression: true,
    priority: 'normal'
  });

  // Create sensor simulator
  const simulator = new IoTSensorSimulator(streamer);

  // Show initial stats
  const initialStats = streamer.getStats();
  console.log('📊 Initial Configuration:');
  console.log(`   Batch size: ${initialStats.batchSize}`);
  console.log(`   Flush interval: ${initialStats.flushInterval}ms`);
  console.log(`   Compression: ${initialStats.compressionEnabled}`);
  console.log(`   Priority: ${initialStats.priority}`);
  console.log();

  // Start streaming and simulation
  streamer.startStreaming();
  simulator.startSimulation(500); // Update every 500ms

  // Run for 15 seconds
  console.log('⏳ Running simulation for 15 seconds...\n');

  await new Promise(resolve => setTimeout(resolve, 15000));

  // Stop simulation
  simulator.stopSimulation();
  streamer.stopStreaming();

  // Final statistics
  const finalStats = streamer.getStats();
  console.log('🎉 IoT Streaming Complete!');
  console.log('===========================');
  console.log(`📊 Final Statistics:`);
  console.log(`   Total batches sent: ${finalStats.totalBatchesSent}`);
  console.log(`   Remaining buffer size: ${finalStats.bufferSize}`);
  console.log(`   Simulation active: ${finalStats.isStreaming}`);
  console.log();
  console.log('💡 Key Benefits of UMICP in IoT:');
  console.log('   • Efficient batching of sensor readings');
  console.log('   • Real-time data streaming capabilities');
  console.log('   • Type-safe sensor data handling');
  console.log('   • Configurable compression and priorities');
  console.log('   • Scalable multi-sensor data collection');
}

// Run example if executed directly
if (require.main === module) {
  iotStreamingExample().catch(console.error);
}

export { IoTDataStreamer, IoTSensorSimulator, iotStreamingExample };

