# Architecture

## Overview

Two subsystems share a common domain (file/disk management) but are independently deployable:

1. **File Manager CLI** (`main.cpp`) — interactive terminal application.
2. **File Manager Web** (`app.py` + `static/`) — Flask JSON API + vanilla JS frontend.
3. **Disk Scheduling Simulator** (`try.cpp`) — standalone algorithmic module.

No shared runtime; C++ and Python communicate only via the filesystem.

## Components

### 1. File Manager CLI (C++17)

```
FileOperation (abstract)
 ├─ ListFilesOperation          → std::filesystem::directory_iterator
 ├─ CreateFileOperation         → std::ofstream + exists() guard
 ├─ DeleteFileOperation         → std::filesystem::remove (no shell)
 ├─ RenameFileOperation         → std::filesystem::rename
 ├─ EditFileOperation           → $EDITOR allowlisted, system() only here (quoted)
 ├─ SearchFileOperation         → std::filesystem::exists
 ├─ FileDetailsOperation        → stat(2)
 ├─ ViewFileOperation           → std::ifstream line loop
 ├─ SortFileOperation           → read → sort → overwrite
 ├─ ListDirectoriesOperation    → directory_iterator + is_directory
 ├─ ListFilesWithExtensionOp    → extension filter
 ├─ CountDirectoriesOperation   → iterator count
 ├─ CountFilesOperation         → iterator count
 └─ SortFilesOperation          → collect → sort → print
```

- Polymorphism via `unique_ptr<FileOperation>` (RAII, no manual `delete`).
- Input validation in `isValidFileName` / `isValidExtension`; path separators rejected.
- `processChoice` validates `0` as exit before dispatch.

### 2. Web Backend (Flask)

- `WORKSPACE = Path(FILE_MANAGER_ROOT).resolve()` — all operations confined.
- `safe_path(filename)` validates regex, rejects separators, then `resolve().relative_to(WORKSPACE)` to block symlink escapes.
- Endpoints are thin wrappers: validate → filesystem op → JSON/text response with explicit status codes.
- `MAX_FILE_SIZE = 10 MiB` enforced on view/edit/sort.
- Static serving via `send_from_directory` with `relative_to` check.

Frontend (`static/script.js`) uses `fetch()` with `application/json`; no framework, no build step.

### 3. Disk Scheduling (C++17)

Pure functions in `try.cpp`:

```cpp
SchedulingResult cscan(requests, head, diskSize, goingRight)
SchedulingResult clook(requests, head, goingRight)
int calculateSeek(sequence, head)
```

- No global state; testable with `--test` / `DISK_SCHEDULING_TEST`.
- C-SCAN: services one direction, traverses to physical end (`diskSize-1`), wraps to `0`, continues.
- C-LOOK: services one direction, jumps directly to farthest request on opposite side (no edge travel).
- Validation throws `invalid_argument` for out-of-range inputs.

## Data Flow

```
CLI: stdin → processChoice → FileOperation::execute → filesystem → stdout
Web: browser prompt → fetch(JSON) → Flask safe_path → filesystem → JSON/text → DOM
Disk: stdin / demo vector → cscan/clook → SchedulingResult → stdout
```

## Build & Deploy

- **CMake** produces `file_manager`, `disk_scheduling`, `disk_scheduling_test`; `enable_testing()` registers CTest.
- **Makefile** is a convenience wrapper for `c++` direct builds and `cmake`.
- **Docker** multi-stage-friendly: `python:3.12-slim` + `build-essential`, builds C++ binaries, serves Flask via `gunicorn` (2 workers, 4 threads), healthcheck on `/health`.

## Decisions & Trade-offs

| Decision | Rationale | Alternative |
|----------|-----------|-------------|
| Keep `main.cpp`/`try.cpp` at repo root | Backward compat with existing coursework links | Move to `src/` (considered for v2) |
| `std::filesystem` over `system("ls")` | Eliminates shell injection | Keep shell for simplicity (rejected) |
| Allowlist regex for filenames | Simple, auditable | Full MIME/unicode support (overkill) |
| Single `WORKSPACE` sandbox | Easy to reason about, testable | Per-user chroot (complex) |
| Vanilla JS, no bundler | Zero build, coursework-friendly | React/Vite (heavier) |
| `gunicorn` in Docker | Production WSGI | `flask run` (dev only) |

## Future

- Extract `FileOperation` hierarchy to `include/file_manager.hpp` + `src/` for unit testing with GoogleTest.
- Add auth (session or token) before exposing web UI beyond localhost.
- Promote disk scheduling to a library (`libdisk_scheduling.a`) reusable from Python via pybind11.
