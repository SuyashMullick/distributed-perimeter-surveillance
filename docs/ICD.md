# Interface Control Document (ICD)

All messages are UTF-8, newline-delimited JSON objects over ZeroMQ PUB/SUB.

## 1. Common Fields

- `timestamp_utc`: ISO-8601 string in UTC with milliseconds (e.g., `"2026-02-23T12:34:56.123Z"`)
- `monotonic_ns`: Unsigned 64-bit integer tracking internal nanoseconds.
- `sequence_number`: Monotonically increasing per node starting at 1.

## 2. Message Types

### 2.1 DisturbanceEvent
Published by Sensor Nodes.
```json
{  
  "msg_type": "DisturbanceEvent",  
  "event_id": "uuid string",  
  "node_id": "string",  
  "sequence_number": 123,  
  "timestamp_utc": "2026-02-23T12:34:56.123Z",  
  "monotonic_ns": 1234567890,  
  "signal_amplitude": 0.42,  
  "signal_energy": 12.34,  
  "event_type": "WALKING|VEHICLE|DIGGING|WIND",  
  "generated_seed": 1337  
}
```

### 2.2 NodeStatus
Published by Sensor Nodes at 1 Hz.
```json
{  
  "msg_type": "NodeStatus",  
  "node_id": "string",  
  "timestamp_utc": "2026-02-23T12:34:56.123Z",  
  "monotonic_ns": 1234567890,  
  "health": "OK|DEGRADED|FAILED",  
  "uptime_s": 12.345,  
  "last_sequence_number": 123  
}
```

### 2.3 CentralAlert
Stored locally by Central Processor.
```json
{  
  "msg_type": "CentralAlert",  
  "alert_id": "uuid string",  
  "event_id": "uuid string",  
  "source_node_id": "string",  
  "timestamp_utc": "2026-02-23T12:34:56.456Z",  
  "monotonic_ns": 1234569999,  
  "classification": "LOW|MEDIUM|HIGH",  
  "processing_latency_ms": 42.0  
}
```

## 3. Error Handling
- Missing required fields throw runtime schema exceptions which are trapped, causing the message to traverse to `invalid_messages_total` metric drop counter.
- ZeroMQ handles raw socket dropping inherently if HWM is breached or no PUB paths exist.
