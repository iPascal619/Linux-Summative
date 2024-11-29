# System Programming Projects

## Overview
This repository contains four comprehensive system programming projects demonstrating advanced C and Python programming techniques.

## Projects

### 1. System Resource Monitoring
- Tracks real-time system performance
- Monitors CPU, Memory, and Network Usage
- Generates performance graphs
- Logs system metrics and active processes

#### Features
- C program for resource tracking
- Python script for data visualization
- Generates `usage.txt` and `processes.txt`

#### Requirements
- GCC Compiler
- Python 3.x
- Libraries: 
  - `matplotlib`
  - `psutil`

#### Compilation
```bash
# Compile C Resource Monitor
gcc system_resource_monitor.c -o resource_monitor

# Run Python Plotting Script
python plot_system_metrics.py
```

### 2. Multi-Threading (Producer-Consumer Model)
- Simulates industrial bottle production scenario
- Implements thread synchronization
- Manages a bounded queue with producer and consumer threads

#### Features
- Configurable queue size (default 10)
- Producer thread adds bottles every 2 seconds
- Consumer thread removes bottles every 3 seconds
- Prevents queue overflow and underflow

#### Compilation
```bash
gcc -pthread producer_consumer.c -o producer_consumer
./producer_consumer
```

### 3. Socket Programming (Chat System)
- Multi-client chat application
- Implements user authentication
- Supports bidirectional messaging

#### Features
- Central server managing up to 4 clients
- Username-based authentication
- Client-to-client messaging
- Online user listing

#### Compilation
```bash
gcc chat_server.c -pthread -o chat_server
./chat_server
```

### 4. Network IP Discovery
- Captures IP addresses and hostnames
- Client-server architecture
- Logs network device information

#### Features
- Discovers network interfaces
- Server logs device details
- Supports multiple network devices
- Multithreaded design

#### Compilation
```bash
gcc -pthread ip_capture.c -o ip_capture
./ip_capture
```

## Prerequisites
- Linux/Unix Environment
- GCC Compiler
- POSIX Threads (pthread)
- Python 3.x (for resource plotting)

## Permissions
Some scripts require root/sudo privileges for network and system resource access.

## Contributing
Contributions, issues, and feature requests are welcome!
