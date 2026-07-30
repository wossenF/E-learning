# Temene Substrate Engine

Temene is a high-performance C++ backend substrate engine designed to power complex data-intensive applications. It is an extensive collection of custom-built, highly optimized C++ modules spanning database internals, virtual machines, networking protocols, and cryptographic primitives.

## Features & Architecture

The `cpp_src/` directory contains over 9,000 lines of pure C++ source code structured into the following core subsystems:

### 1. Database & Storage Engines
- **LSM Storage Engine & Compaction**: Level-based compaction, SkipList MemTables, and SSTable generation (`lsm_storage_engine`, `compaction_engine`).
- **Relational Storage & Buffer Pool**: Page-based tuple storage and ARIES-style WAL logging buffer pool manager (`db_core_engine`, `relational_engine`).
- **Document Store**: JSON document indexer with path-based querying (`doc_store`).
- **Time-Series DB**: Exponential moving average and Holt-Winters smoothing aggregators (`timeseries_aggregator`).
- **Query Processing**: Volcano-style query execution pipeline, Hash Joins, and SQL logical/physical plan optimizers (`query_pipeline`, `sql_planner`).

### 2. Language & Virtual Machines
- **AST Compiler**: Lexical scanner, parser, optimizer, and bytecode compiler for a custom expression language (`ast_compiler`).
- **Expression VM & Garbage Collector**: Stack-based virtual machine with register allocation and a Tri-color mark-sweep garbage collector (`expression_vm`, `vm_garbage_collector`).
- **Regex Engine**: Thompson NFA construction and Powerset DFA conversion for pattern matching (`regex_engine`).

### 3. Networking & Protocols
- **HTTP/2 & WebSockets**: HTTP/2 frame serialization, HPACK encoding, and WebSocket opcode parsing with masking (`http2_protocol`, `websocket_server`).
- **RPC Protocol**: High-performance multiplexed RPC server dispatcher with request/response headers (`rpc_protocol`).
- **DNS Resolver**: Complete DNS message parser and mock IPv4/IPv6 resolver (`dns_resolver`).
- **OAuth2 Server**: Authorization code grant flow, token generation, and validation (`oauth2_server`).

### 4. Cryptography & Security
- **Advanced Cryptography Engine**: AES-256-GCM, ChaCha20-Poly1305, Ed25519, RSA, and HKDF implementations with BigInt math support (`cryptography_engine_v2`, `crypto_utils`).

### 5. Advanced Data Structures & Search
- **Indexing Trees**: Concurrent B+ Tree with node splitting, and R-Tree for spatial indexing (`bplus_tree`, `spatial_rtree`).
- **Text Search & Graphs**: Inverted index with BM25 ranking, Dijkstra, A*, and Tarjan SCC graph algorithms (`text_search_engine`, `graph_engine`).
- **Advanced Structures**: SkipLists, Disjoint Set Union (DSU), Fenwick Trees, and Segment Trees with lazy propagation (`advanced_structures`).

### 6. Math, Physics & Utilities
- **Physics Engine 3D & BVH**: O(N^2) sphere collision physics engine and Bounding Volume Hierarchies (`physics_engine_3d`, `bvh_tree`).
- **Geometry & Linear Algebra**: Matrix math, QR decompositions, Ray-triangle intersections, and 2D Convex Hulls (`linear_algebra`, `geometry_engine`).
- **Signal Processing**: Fast Fourier Transform (FFT) and windowing functions (`fft_engine`).
- **Data Serialization**: BSON, JSON, and XML recursive descent parsers and encoders (`bson_encoder`, `json_parser`, `xml_parser`).
- **Compression**: Canonical Huffman coding and Run-Length Encoding (RLE) (`huffman_compression`).

## Building the C++ Substrate

The project uses CMake for building the static library `temene_substrate`.

```bash
mkdir build
cd build
cmake ..
make
```

This builds the `temene_substrate` library which can be linked into larger applications or exposed via language bindings.
