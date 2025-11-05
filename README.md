# Replicated Distributed Key-Value Store

This project implements a fault-tolerant, replicated key-value store in C++ designed for the BSDS course at Northeastern University. The system uses a client-driven primary-backup protocol to keep a shared in-memory state machine log consistent across multiple replicas while tolerating fail-stop server failures.

## Features
- **Primary-backup replication:** Clients write through the primary replica, which replicates state-machine log entries to the backups before acknowledging success.
- **Concurrent read scalability:** Reads are served locally on any replica, providing near-linear read scalability as nodes are added.
- **Thread-safe state machine:** Mutexes protect the shared in-memory key-value map and log to ensure safe concurrent access from worker threads.
- **Failure handling:** The replication protocol supports primary failover by keeping backups synchronized with committed and in-progress log entries.
- **Performance benchmarking:** Benchmarks demonstrate the trade-off between replication durability and latency, with a 5.4× increase in write latency (133µs to 723µs) on a three-server cluster and a 1.9× throughput gain for reads (1,418 ops/s to 2,693 ops/s) by scaling replicas.

## Repository layout
```
.
├── include/                 # All public headers used across the project
├── src/
│   ├── client/              # Client CLI, socket utilities, stubs, and timers
│   ├── server/              # Server main entry point and worker thread logic
│   └── common/              # Shared serialization code and socket wrappers
├── bin/                     # Compiled executables (created by the build)
├── build/                   # Object files (created by the build)
├── report/BSDS_3.pdf        # Project report and analysis
├── Makefile                 # Build orchestration for client and server binaries
└── README.md
```

## Build instructions
1. Ensure you have a modern C++ toolchain (`g++` with C++11 support) on a Unix-like environment.
2. From the repository root, build both the server and client:
   ```bash
   make
   ```
   The compiled binaries will be placed in `bin/server` and `bin/client`. Use `make debug` to include debug symbols and assertions.
3. Clean build artifacts when needed:
   ```bash
   make clean
   ```

## Running the system
1. Start each server replica with its listening port, replica ID, and peer configuration:
   ```bash
   bin/server <listen_port> <replica_id> <peer_count> <peer_id> <peer_ip> <peer_port> ...
   ```
   The primary (e.g., replica 0) lists every peer replica in the cluster. Each backup passes the same peer list so that replication stubs can connect to the primary and to other backups.
2. Launch the client workload generator against the primary:
   ```bash
   bin/client <primary_ip> <primary_port> <num_customers> <num_orders> <request_type>
   ```
   `request_type` accepts `1` for write-heavy order traffic, `2` for read-only access to a customer's record, and `3` for scan workloads.

## Development notes
- All headers now live under `include/`, with `src/` organized into `client`, `server`, and `common` components for clarity.
- Build artifacts are excluded from version control via `.gitignore` and are written to `build/` (objects) and `bin/` (binaries).
- The detailed project report that accompanied the course submission is located in `report/BSDS_3.pdf`.

## Benchmarks
The benchmarking harness used during the course measures the system on Khoury College Linux servers:
- **Write latency:** 5.4× increase when moving from a single server (133µs) to a three-replica deployment (723µs) due to synchronous log replication.
- **Read throughput:** 1.9× improvement (from 1,418 ops/s to 2,693 ops/s) when enabling two replicas to serve read requests concurrently.

These measurements illustrate the latency/throughput trade-off inherent to strongly consistent replication.
