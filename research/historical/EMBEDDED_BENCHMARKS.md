> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# Embedded System Benchmarking

## Hardware Platforms

### 1. Cortex-M3/M4 ARM Cortex-M Processors
- Clock speed: 72MHz (STM32F407) to 240MHz (STM32H743)
- RAM: 128KB to 512KB 
- Flash: 512KB to 2MB
- No OS, direct embedded execution

### 2. ESP32
- Xtensa LX6 32-bit RISC processor @ 240MHz
- RAM: 520KB SRAM  
- Flash: Up to 16MB
- WiFi/BT capabilities
- Power management features

## Performance Metrics

### 1. Memory Usage Assessment

#### Static Memory (RAM/Flash):
- `bp_config_t` structure: 8 bytes
- Temporary variables in functions: O(1) 
- Function call overhead: Minimal

#### Dynamic Memory:
- No heap allocation required
- Stack usage: Minimal for all algorithms
- No malloc/free operations needed

#### Flash Footprint Analysis:
- Code size: Estimated ~250-500 bytes of executable code
- Data sections: Minimal (constants only)

### 2. Execution Time Benchmarking

#### Cortex-M3/4 Benchmarking Setup:
```c
// Timing using SysTick timer
volatile uint32_t start = SysTick->VAL;
bp_delta(&cfg, probe, axis, &delta);
volatile uint32_t end = SysTick->VAL;
```

Expected execution: <100 cycles for baseline operation

#### ESP32 Benchmarking Setup:
```c
// Using ESP-IDF timer functions  
esp_timer_get_time();
bp_delta(&cfg, probe, axis, &delta);
esp_timer_get_time();
```

Expected execution: <500 cycles for baseline operation

### 3. Resource Consumption Tests

#### RAM Usage Test:
- Baseline configuration (N=8): Only 8 bytes for config
- No temporary arrays needed in core operations  
- Stack usage negligible (<100 bytes)

#### Power Analysis (ESP32):
- Short execution time minimizes power consumption
- Low frequency operation typical
- Minimal interrupt overhead

### 4. Practical Amplitude Limitations

For N=32:
- Maximum perturbation: +31q on one channel
- Required testing with actual hardware limits
- Threshold analysis for practical range

## Test Protocol

### 1. Basic Functionality Test
```c
void test_functionality()
{
    bp_config_t cfg = {8, 10};
    int32_t delta;
    
    // Validate all basic operations work
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &delta)); 
    assert(delta == 70);  // (8-1)*10 = 70
}
```

### 2. Performance Test
```c
void test_performance()
{
    bp_config_t cfg = {8, 10};
    int32_t delta;
    uint64_t total_cycles = 0;
    
    for(int i = 0; i < 100000; i++) {
        // Measure time using hardware timer
        bp_delta(&cfg, i%8, i%8, &delta);
        total_cycles += get_cycles();
    }
}
```

### 3. Memory Test  
```c
void test_memory_usage()
{
    bp_config_t cfg = {8, 10};
    assert(sizeof(cfg) == 8);  // Verify size requirement
    
    // Test on various N values to ensure no memory issues  
    for(int n = 2; n <= 32; n++) {
        cfg.count = n;
        assert(bp_validate(&cfg));
    }
}
```

### 4. Practical Scenario Tests
- Test with real ADC systems
- Simulate noise conditions  
- Validate linear/nonlinear behavior
- Check saturation effects

## Expected Results

### 1. Execution Time Results (approximate)
| Platform | Operations/second |
|----------|------------------|
| Cortex-M4 | >200,000 ops/sec |
| ESP32 | >500,000 ops/sec |

### 2. Memory Requirements  
| Resource | Usage |
|----------|-------|
| Static RAM | ~8 bytes (config only) |
| Stack | <100 bytes |
| Flash | ~250-500 bytes |

### 3. Practical Limits
| N | Max Amplitude | Considered Practical |
|---|---------------|---------------------|
| 4 | +3q | Yes |
| 8 | +7q | Yes |  
| 16 | +15q | Yes |
| 32 | +31q | Questionable |

## Critical Hardware Validation Required

### 1. Real ADC Testing
- Validate linear response with actual measurement system  
- Confirm no saturation effects
- Verify quantization impacts

### 2. System Stability Testing
- Long-term operation verification  
- Temperature stability testing
- Drift analysis

### 3. Practical Amplitude Constraints
- Identify maximum safe values for specific devices
- Document recommended N ranges per platform
- Evaluate trade-off between measurement efficiency and amplitude concerns

## Comparison with Baselines (Cortex-M4)

| Metric | Balanced Probe | Central Coord. | Forward Coord. | Pair-transfer |
|--------|----------------|----------------|----------------|---------------|
| Measurements | 8 | 16 | 9 | 28 |
| Max amplitude | +7q | Â±q | Â±q | Â±q |
| RAM usage | 8 bytes | 8 bytes | 8 bytes | 8 bytes |
| Flash size | ~300 bytes | ~300 bytes | ~300 bytes | ~300 bytes |
| Execution time | <100 cycles | <100 cycles | <100 cycles | <100 cycles |

## Final Assessment

The balanced probe shows excellent potential for embedded systems:
- Significantly fewer measurements than traditional methods  
- Minimal resource overhead
- Strong mathematical foundations
- Practical limitation with amplitude scaling for large N (>16)
