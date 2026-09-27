# SmartFileCompressor (SFC) 2.0
> **An OS-Driven System Engine for Intelligent File Compression, Process Handle Monitoring, & Low-Priority Archiving.**

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()

SmartFileCompressor (SFC) is a high-performance system service and CLI utility designed to optimize file storage by utilizing **core Operating System primitives**. Unlike generic zip tools, SFC inspects system process handles, manages thread priorities, leverages memory-mapped I/O (`mmap`), and runs low-priority background archiving without causing system lag or disk I/O bottlenecks.

---

## 🌟 Key OS Concepts & Features

- 🔍 **Active Process & Handle Locking Inspector**:
  - Automatically queries system process handles (`CreateToolhelp32Snapshot` / `NtQuery` on Windows, `/proc` & `fcntl` locks on Linux).
  - Prevents data corruption by refusing to compress files currently opened or locked by active applications (e.g., text editors, databases, IDEs).

- ⚡ **Zero-Copy Memory-Mapped I/O (`mmap`)**:
  - Maps input files directly into virtual address space (`CreateFileMapping` / `MapViewOfFile` on Windows, `mmap` on Linux).
  - Streams memory pages with constant $O(1)$ RAM usage regardless of file size.

- 🧵 **OS-Aware Worker ThreadPool & Priority Scheduling**:
  - Distributes streaming chunk compression across a lock-free worker thread pool.
  - Automatically assigns **Low / Idle CPU priority** (`SetThreadPriority` / `nice`) and **Background I/O priority** (`SetPriorityClass(PROCESS_MODE_BACKGROUND_BEGIN)` / `ioprio_set`).

- 📦 **Custom `.sfc` Binary Header Specification**:
  - Embeds a 32-byte binary header into every compressed archive:
    - **Magic Identifier**: `SFC1` (`0x53 0x46 0x43 0x31`)
    - **Original & Payload Sizes**: 64-bit unsigned integers
    - **Checksum**: CRC32 integrity verification
    - **Metadata**: Timestamp (mtime) preservation & algorithm tag

- 📡 **Client-Daemon Architecture & Inter-Process Communication (IPC)**:
  - Background daemon (`sfcd`) manages low-priority background jobs.
  - CLI client (`sfc`) interacts with `sfcd` over **Named Pipes** (Windows) or **Unix Domain Sockets** (Linux).

---

## 📂 Project Architecture

```
SmartFileCompressor/
├── CMakeLists.txt              # Cross-platform CMake configuration
├── include/
│   └── sfc/
│       ├── common.h            # SFC Header struct & format constants
│       ├── compressor.h        # Streaming chunk compressor interface
│       ├── decompressor.h      # Streaming chunk decompressor & checksum validator
│       ├── mmap_file.h         # Cross-platform memory mapping abstraction
│       ├── process_monitor.h   # OS process handle & lock inspector
│       ├── scheduler.h         # CPU & Disk I/O priority management
│       ├── thread_pool.h       # Lock-free OS-aware worker thread pool
│       └── ipc.h               # Named Pipes / Unix Sockets IPC layer
├── src/
│   ├── core/                   # Engine implementation (compressor, decompressor, mmap, pool)
│   ├── os/                     # OS-specific hooks (Windows Toolhelp / Linux proc & fcntl)
│   ├── ipc/                    # Cross-platform IPC client/server implementation
│   ├── cli/                    # CLI executable entry point (sfc)
│   └── daemon/                 # Daemon service entry point (sfcd)
└── tests/
    └── test_sfc.cpp            # Unit test suite for compression integrity & handle checks
```

---

## 🛠️ Building & Usage

### 1. Build Prerequisites
- C++17 compliant compiler (GCC 9+, Clang 10+, MSVC 2019+)
- `CMake 3.15+`
- `zlib` development library

### 2. Compilation
```bash
cmake -B build -S .
cmake --build build --config Release
```

### 3. Command Line Interface (`sfc`)

#### Compress a file:
```bash
./sfc compress data.log
# Produces data.log.sfc with SFC1 header & CRC32
```

#### Decompress a file:
```bash
./sfc decompress data.log.sfc
# Decompresses and verifies CRC32 checksum
```

#### Inspect binary header metadata:
```bash
./sfc inspect data.log.sfc
```

#### Inspect OS process handle locks:
```bash
./sfc check-lock active_file.db
# Checks if any running PID holds a lock on the target file
```

#### Query background daemon status over IPC:
```bash
./sfc daemon-status
```

---

## 🔬 Unit Testing

Run the built test suite to verify binary format integrity, CRC32 verification, and handle detection:
```bash
./test_sfc
```

---

## 📄 License
Distributed under the MIT License.
