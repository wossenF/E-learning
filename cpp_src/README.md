# Minimal C++ JSON-like parser

This folder contains a small, self-contained JSON-like parser (`json_parser.cpp`/`.h`) intended
as a meaningful C++ substrate for fuzzing. It implements a permissive parser that exercises
string escaping, numeric parsing, arrays, and nested objects without depending on external libs.

Files:
- `json_parser.h` — public header
- `json_parser.cpp` — parser implementation
- `api.h` / `impl1.cpp` / other helpers — auxiliary code used by the project

Build: These sources can be built locally via `CMakeLists.txt` in this folder or from the
repository root. They are maintained as a native substrate library and examples.
