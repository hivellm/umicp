/*!
# Real-Time Data Processing Example

This example demonstrates real-time data processing patterns using UMICP
envelopes, including streaming data, buffering, and performance optimization.
*/

use umicp_core::{Envelope, Matrix, OperationType};
use std::collections::{HashMap, VecDeque};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
use std::thread;

/// Real-time data processing simulation
fn main() -> Result<(), Box<dyn std::error::Error>> {

    // Example 1: Streaming Data Processor

    let processor = Arc::new(StreamingProcessor::new());
    demonstrate_streaming_processor(processor.clone())?;

    // Example 2: Real-time Analytics

    let analytics = Arc::new(AnalyticsEngine::new());
    demonstrate_real_time_analytics(analytics.clone())?;

    // Example 3: Load Balancing Simulation

    demonstrate_load_balancing()?;

    // Example 4: Performance Monitoring

    let monitor = PerformanceMonitor::new();
    demonstrate_performance_monitoring(monitor)?;


    Ok(())
}

/// Streaming data processor with buffering and batch processing
struct StreamingProcessor {
    buffer: Mutex<VecDeque<Envelope>>,
    batch_size: usize,
    flush_interval: Duration,
    matrix: Matrix,
    processed_count: Mutex<u64>,
}

impl StreamingProcessor {
    fn new() -> Self {
        Self {
            buffer: Mutex::new(VecDeque::new()),
            batch_size: 100,
            flush_interval: Duration::from_millis(500),
            matrix: Matrix::new(),
            processed_count: Mutex::new(0),
        }
    }

    /// Add envelope to processing buffer
    fn add_envelope(&self, envelope: Envelope) {
        let mut buffer = self.buffer.lock().unwrap();
        buffer.push_back(envelope);

        // Auto-flush if buffer is full
        if buffer.len() >= self.batch_size {
            drop(buffer); // Release lock before processing
            self.flush_batch();
        }
    }

    /// Process batch of envelopes
    fn flush_batch(&self) {
        let mut buffer = self.buffer.lock().unwrap();
        if buffer.is_empty() {
            return;
        }

        let batch: Vec<_> = buffer.drain(..).collect();
        drop(buffer); // Release lock


        // Process batch
        for envelope in batch {
            self.process_envelope(envelope);
        }

        let mut count = self.processed_count.lock().unwrap();
        *count += 1;

    }

    /// Process individual envelope
    fn process_envelope(&self, envelope: Envelope) {
        // Simulate processing based on operation type
        match envelope.operation() {
            OperationType::Data => {
                // Perform matrix operations on data
                if let Some(capabilities) = envelope.capabilities() {
                    if let Some(data_size) = capabilities.get("data_size") {
                        if let Ok(size) = data_size.parse::<usize>() {
                            self.process_data_payload(size);
                        }
                    }
                }
            }
            OperationType::Control => {
                // Handle control messages
            }
            OperationType::Ack => {
                // Handle acknowledgments
            }
            OperationType::Error => {
                // Handle error messages
            }
            OperationType::Request => {
                // Handle requests
            }
            OperationType::Response => {
                // Handle responses
            }
        }
    }

    /// Process data payload with matrix operations
    fn process_data_payload(&self, size: usize) {
        if size == 0 || size > 10000 {
            return; // Skip invalid sizes
        }

        // Create test data for processing
        let mut data = vec![0.0f32; size];
        for i in 0..size {
            data[i] = (i as f32 * 0.1).sin();
        }

        // Perform matrix operations
        let mut processed_data = vec![0.0f32; size];

        // Apply some transformations
        if size >= 4 {
            // Vector scaling
            let _ = self.matrix.vector_scale(&data, 2.0, &mut processed_data);

            // If we have enough data, perform additional operations
            if size >= 8 {
                let mut temp_data = vec![0.0f32; size];
                let _ = self.matrix.vector_add(&processed_data, &data, &mut temp_data);
                processed_data.copy_from_slice(&temp_data);
            }
        }

        // Simulate processing time based on data size
        let processing_time = Duration::from_micros((size as u64).min(1000));
        thread::sleep(processing_time);
    }

    /// Get processor statistics
    fn get_stats(&self) -> ProcessorStats {
        let buffer = self.buffer.lock().unwrap();
        let processed_count = self.processed_count.lock().unwrap();

        ProcessorStats {
            buffer_size: buffer.len(),
            batch_size: self.batch_size,
            processed_batches: *processed_count,
            total_processed: *processed_count * self.batch_size as u64,
        }
    }
}

#[derive(Debug)]
struct ProcessorStats {
    buffer_size: usize,
    batch_size: usize,
    processed_batches: u64,
    total_processed: u64,
}

/// Real-time analytics engine
struct AnalyticsEngine {
    metrics: Mutex<HashMap<String, MetricData>>,
    matrix: Matrix,
}

#[derive(Clone)]
struct MetricData {
    sum: f64,
    count: u64,
    min: f64,
    max: f64,
    avg: f64,
    last_updated: Instant,
}

impl AnalyticsEngine {
    fn new() -> Self {
        Self {
            metrics: Mutex::new(HashMap::new()),
            matrix: Matrix::new(),
        }
    }

    /// Process analytics data
    fn process_data(&self, key: &str, values: &[f32]) {
        let mut metrics = self.metrics.lock().unwrap();

        let metric = metrics.entry(key.to_string()).or_insert(MetricData {
            sum: 0.0,
            count: 0,
            min: f64::INFINITY,
            max: f64::NEG_INFINITY,
            avg: 0.0,
            last_updated: Instant::now(),
        });

        // Update metrics
        for &value in values {
            let value_f64 = value as f64;
            metric.sum += value_f64;
            metric.count += 1;
            metric.min = metric.min.min(value_f64);
            metric.max = metric.max.max(value_f64);
        }

        metric.avg = metric.sum / metric.count as f64;
        metric.last_updated = Instant::now();
    }

    /// Get analytics summary
    fn get_summary(&self) -> HashMap<String, serde_json::Value> {
        let metrics = self.metrics.lock().unwrap();
        let mut summary = HashMap::new();

        for (key, metric) in metrics.iter() {
            summary.insert(key.clone(), serde_json::json!({
                "count": metric.count,
                "average": metric.avg,
                "min": metric.min,
                "max": metric.max,
                "sum": metric.sum,
                "last_updated_ms": metric.last_updated.elapsed().as_millis()
            }));
        }

        summary
    }

    /// Perform real-time statistical analysis
    fn analyze_data(&self, data: &[f32]) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        let mut results = HashMap::new();

        if data.is_empty() {
            return Ok(results);
        }

        // Basic statistics
        let sum: f64 = data.iter().map(|&x| x as f64).sum();
        let mean = sum / data.len() as f64;
        results.insert("mean".to_string(), mean);

        let variance: f64 = data.iter()
            .map(|&x| {
                let diff = x as f64 - mean;
                diff * diff
            })
            .sum::<f64>() / data.len() as f64;
        results.insert("variance".to_string(), variance);
        results.insert("std_dev".to_string(), variance.sqrt());

        // Find min/max
        let min = data.iter().fold(f32::INFINITY, |a, &b| a.min(b)) as f64;
        let max = data.iter().fold(f32::NEG_INFINITY, |a, &b| a.max(b)) as f64;
        results.insert("min".to_string(), min);
        results.insert("max".to_string(), max);

        // Percentiles (simplified)
        let mut sorted_data: Vec<f32> = data.to_vec();
        sorted_data.sort_by(|a, b| a.partial_cmp(b).unwrap());

        let p25_idx = (sorted_data.len() as f64 * 0.25) as usize;
        let p75_idx = (sorted_data.len() as f64 * 0.75) as usize;
        let p95_idx = (sorted_data.len() as f64 * 0.95) as usize;

        results.insert("p25".to_string(), sorted_data.get(p25_idx).copied().unwrap_or(0.0) as f64);
        results.insert("p75".to_string(), sorted_data.get(p75_idx).copied().unwrap_or(0.0) as f64);
        results.insert("p95".to_string(), sorted_data.get(p95_idx).copied().unwrap_or(0.0) as f64);

        Ok(results)
    }
}

/// Load balancing simulation
fn demonstrate_load_balancing() -> Result<(), Box<dyn std::error::Error>> {

    let num_workers = 4;
    let total_tasks = 100;
    let mut worker_stats = vec![0u32; num_workers];

    // Simulate task distribution
    for task_id in 0..total_tasks {
        let worker_id = task_id % num_workers;

        // Simulate processing time
        let processing_time = 10 + (task_id % 20); // 10-30ms variation
        thread::sleep(Duration::from_millis(processing_time as u64));

        worker_stats[worker_id as usize] += 1;

        if task_id % 20 == 0 {
        }
    }

    // Show load balancing results
    for (i, &tasks) in worker_stats.iter().enumerate() {
        let percentage = (tasks as f32 / total_tasks as f32) * 100.0;
    }

    let max_load = worker_stats.iter().max().unwrap();
    let min_load = worker_stats.iter().min().unwrap();
    let load_imbalance = (*max_load as f32 - *min_load as f32) / *max_load as f32 * 100.0;


    Ok(())
}

/// Performance monitoring system
struct PerformanceMonitor {
    metrics: Mutex<HashMap<String, Vec<Duration>>>,
}

impl PerformanceMonitor {
    fn new() -> Self {
        Self {
            metrics: Mutex::new(HashMap::new()),
        }
    }

    /// Record operation timing
    fn record_timing(&self, operation: &str, duration: Duration) {
        let mut metrics = self.metrics.lock().unwrap();
        metrics.entry(operation.to_string())
            .or_insert_with(Vec::new)
            .push(duration);
    }

    /// Get performance statistics
    fn get_stats(&self) -> HashMap<String, PerformanceStats> {
        let metrics = self.metrics.lock().unwrap();
        let mut stats = HashMap::new();

        for (operation, timings) in metrics.iter() {
            if timings.is_empty() {
                continue;
            }

            let total: Duration = timings.iter().sum();
            let avg = total / timings.len() as u32;

            let mut sorted_timings = timings.clone();
            sorted_timings.sort();

            let p50 = sorted_timings[timings.len() / 2];
            let p95 = sorted_timings[(timings.len() as f64 * 0.95) as usize];
            let p99 = sorted_timings[(timings.len() as f64 * 0.99) as usize];

            stats.insert(operation.clone(), PerformanceStats {
                count: timings.len(),
                total_time: total,
                avg_time: avg,
                p50_time: p50,
                p95_time: p95,
                p99_time: p99,
            });
        }

        stats
    }
}

#[derive(Debug)]
struct PerformanceStats {
    count: usize,
    total_time: Duration,
    avg_time: Duration,
    p50_time: Duration,
    p95_time: Duration,
    p99_time: Duration,
}

// Demonstration functions

fn demonstrate_streaming_processor(processor: Arc<StreamingProcessor>) -> Result<(), Box<dyn std::error::Error>> {
    let num_envelopes = 250;

    // Generate test envelopes
    for i in 0..num_envelopes {
        let envelope = Envelope::builder()
            .from("sensor-001")
            .to("processor")
            .operation(OperationType::Data)
            .message_id(&uuid::Uuid::new_v4().to_string())
            .capability("data_size", "100")
            .capability("sequence", &i.to_string())
            .capability("timestamp", &chrono::Utc::now().timestamp().to_string())
            .build()?;

        processor.add_envelope(envelope);

        // Small delay to simulate real-time streaming
        thread::sleep(Duration::from_millis(10));
    }

    // Final flush
    processor.flush_batch();

    // Show statistics
    let stats = processor.get_stats();

    Ok(())
}

fn demonstrate_real_time_analytics(analytics: Arc<AnalyticsEngine>) -> Result<(), Box<dyn std::error::Error>> {
    let data_streams = vec![
        ("temperature", vec![20.5, 21.0, 19.8, 22.1, 20.9]),
        ("humidity", vec![65.2, 67.1, 63.8, 68.5, 66.3]),
        ("pressure", vec![1013.2, 1012.8, 1014.1, 1013.5, 1013.9]),
    ];


    for (stream_name, data) in data_streams {
        analytics.process_data(stream_name, &data);

        // Perform statistical analysis
        let analysis = analytics.analyze_data(&data)?;
                 stream_name,
                 analysis["mean"],
                 analysis["std_dev"],
                 analysis["min"],
                 analysis["max"]);
    }

    // Show analytics summary
    let summary = analytics.get_summary();
    for (key, value) in summary {
    }

    Ok(())
}

fn demonstrate_performance_monitoring(monitor: PerformanceMonitor) -> Result<(), Box<dyn std::error::Error>> {

    let operations = ["matrix_multiply", "vector_add", "dot_product", "envelope_create"];

    // Simulate operations with varying execution times
    for _ in 0..100 {
        for operation in &operations {
            let start = Instant::now();

            // Simulate operation with random duration
            let base_time = match *operation {
                "matrix_multiply" => 5,
                "vector_add" => 1,
                "dot_product" => 2,
                "envelope_create" => 3,
                _ => 1,
            };

            let variation = (rand::random::<f32>() - 0.5) * base_time as f32;
            let duration_ms = (base_time as f32 + variation).max(0.1) as u64;

            thread::sleep(Duration::from_millis(duration_ms));

            let elapsed = start.elapsed();
            monitor.record_timing(operation, elapsed);
        }
    }

    // Show performance statistics
    let stats = monitor.get_stats();

    for (operation, stat) in stats {
    }

    Ok(())
}
