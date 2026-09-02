# OS Project — File Manager & Disk Scheduling

[![CI](https://github.com/anomalyco/OS_Project/actions/workflows/ci.yml/badge.svg)](https://github.com/anomalyco/OS_Project/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)
[![Python 3.12](https://img.shields.io/badge/python-3.12-blue.svg)](requirements.txt)
[![code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

Production-grade Operating Systems coursework: a **secure file-management suite** (CLI in C++17 + optional Flask web UI) and a **disk-scheduling simulator** (C-SCAN / C-LOOK). Built with hardening, tests, CI, and container support.

> **Status:** Production-ready. See [ARCHITECTURE.md](ARCHITECTURE.md), [SECURITY.md](SECURITY.md), and [CHANGELOG.md](CHANGELOG.md).

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Quick Start](#quick-start)
- [Usage](#usage)
  - [CLI File Manager](#cli-file-manager)
  - [Disk Scheduling Simulator](#disk-scheduling-simulator)
  - [Web UI (Flask)](#web-ui-flask)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Development](#development)
- [Testing](#testing)
- [Docker](#docker)
- [Project Structure](#project-structure)
- [Security](#security)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **File Manager CLI (`main.cpp`)** — 14 operations via polymorphic `FileOperation` hierarchy, now hardened: `std::filesystem` instead of `system("ls")`, filename allowlisting, path-traversal prevention, RAII with `unique_ptr`, robust input handling.
- **Disk Scheduling (`try.cpp`)** — Pure-function C-SCAN and C-LOOK implementations with head/disk-size validation, seek-distance calculation, and `--test` self-check.
- **Web UI (`app.py` + `static/`)** — Flask backend reconstructed from the original scrambled draft; sandboxed to `FILE_MANAGER_ROOT`, size limits, JSON error contracts, health check.
- **Production Scaffolding** — CMake + Makefile, Dockerfile, docker-compose, GitHub Actions CI, `.editorconfig`/`.clang-format`, pytest suite, gunicorn for production serving.

---

## Architecture

```
                ┌─────────────┐
                │  Browser    │
                │ static/*.js │─── fetch() ───┐
                └─────────────┘                ▼
                                        ┌──────────┐
                                        │ Flask    │  app.py (sandboxed to WORKSPACE)
                                        │  app.py  │──▶ filesystem (Path.resolve + relative_to)
                                        └──────────┘
                ┌─────────────┐                ▲
                │  CLI (C++)  │  main.cpp      │
                │ FileManager │── direct fs ───┘
                └─────────────┘

                ┌─────────────┐
                │ Disk Sched. │  try.cpp  (pure functions, no I/O side-effects)
                └─────────────┘
```

Detailed design: [ARCHITECTURE.md](ARCHITECTURE.md). API contracts: [docs/API.md](docs/API.md).

---

## Quick Start

### Prerequisites

- C++17 compiler (`clang++` / `g++`), CMake ≥ 3.16, Make
- Python 3.12+

### 1. Build C++ binaries

```bash
# Direct
c++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o file_manager
c++ -std=c++17 -Wall -Wextra -pedantic try.cpp -o disk_scheduling

# Or via CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Or via Makefile
make cpp
```

### 2. Run Flask web UI

```bash
python3.12 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

# Dev (auto-reload disabled by default for safety)
python app.py
# → http://127.0.0.1:5000

# Production
gunicorn --bind 0.0.0.0:5000 --workers 2 app:app
```

### 3. Docker (one-command)

```bash
docker compose up --build
# → http://localhost:5000  (workspace persisted in ./data)
```

---

## Usage

### CLI File Manager

```bash
./file_manager
```

Menu:

```
===== File Management Menu =====
 1. Show all files
 2. Create a new file
 3. Delete an existing file
 4. Rename a file
 5. Edit file content ($EDITOR or nano)
 6. Search for a file
 7. Show details of a file
 8. View contents of a file
 9. Sort contents of a file
10. List all directories
11. List files with a particular extension
12. Count total directories
13. Count total files
14. Sort files (directory listing)
 0. Exit
```

Security notes: filenames are validated (`^[A-Za-z0-9._-]{1,255}$`), path separators rejected, delete refuses directories, editor is allowlisted via `$EDITOR`.

### Disk Scheduling Simulator

```bash
./disk_scheduling
# Interactive prompts: disk size, head, request list

# Self-test (used in CI)
./disk_scheduling --test  # or the CMake test binary
```

Example session:

```
Enter disk size (default 200): 200
Enter initial head position (0-199) [50]: 50
Enter number of requests: 7
Enter 7 request positions:
82 170 43 140 24 16 190

--- C-SCAN (right) ---
Head start: 50
Seek sequence: 50 -> 82 -> 140 -> 170 -> 190 -> 199 -> 0 -> 16 -> 24 -> 43
Total seek distance: 391

--- C-LOOK (right) ---
Head start: 50
Seek sequence: 50 -> 82 -> 140 -> 170 -> 190 -> 16 -> 24 -> 43
Total seek distance: 341
```

### Web UI (Flask)

Open `http://127.0.0.1:5000` — buttons map 1:1 to API routes. All file operations are confined to `FILE_MANAGER_ROOT` (default: project root; in Docker: `/data`).

---

## API Reference

| Method | Path | Body / Query | Success | Error |
|--------|------|--------------|---------|-------|
| GET | `/` | — | `index.html` | — |
| GET | `/health` | — | `{"status":"ok"}` | — |
| GET | `/list-files` | — | `["a.txt", ...]` | — |
| GET | `/list-directories` | — | `["static", ...]` | — |
| GET | `/view-file?fileName=x.txt` | query | file content (text) | 400/404/413 |
| POST | `/create-file` | `{"fileName":"x.txt"}` | 200 | 400/409 |
| POST | `/delete-file` | `{"fileName":"x.txt"}` | 200 | 404 |
| POST | `/rename-file` | `{"oldFileName":"a","newFileName":"b"}` | 200 | 400/404/409 |
| POST | `/edit-file` | `{"fileName":"x","content":"..."}` | 200 | 400/404/413 |
| POST | `/search-file` | `{"fileName":"x"}` | `{"found":bool}` | — |
| POST | `/file-details` | `{"fileName":"x"}` | `{"size":123,...}` | 404 |
| POST | `/sort-file-contents` | `{"fileName":"x"}` | sorted text | 404 |

Full spec: [docs/API.md](docs/API.md)

`curl` examples:

```bash
curl http://127.0.0.1:5000/list-files
curl -X POST -H "Content-Type: application/json" -d '{"fileName":"hello.txt"}' http://127.0.0.1:5000/create-file
curl "http://127.0.0.1:5000/view-file?fileName=hello.txt"
```

---

## Configuration

| Env var | Default | Description |
|---------|---------|-------------|
| `FILE_MANAGER_ROOT` | project root | Sandbox directory for all file operations |
| `HOST` | `127.0.0.1` | Flask bind host |
| `PORT` | `5000` | Flask bind port |
| `FLASK_DEBUG` | `0` | Set `1` to enable debug (never in production) |
| `EDITOR` | `nano` | Editor for CLI option 5 |

Copy `.env.example` → `.env` for local overrides.

---

## Development

```bash
# Formatting & lint
black app.py
ruff check app.py
clang-format -i main.cpp try.cpp

# Or via Make
make format
make lint
```

Conventions: [CONTRIBUTING.md](CONTRIBUTING.md) · EditorConfig + ClangFormat enforced in CI.

---

## Testing

```bash
# C++ (CMake CTest)
cmake -S . -B build && cmake --build build && ctest --test-dir build -V

# C++ direct
c++ -std=c++17 -DDISK_SCHEDULING_TEST=1 try.cpp -o /tmp/disk_test && /tmp/disk_test --test

# Python
pytest tests/ -v --cov=app --cov-report=term-missing

# All via Make
make test
```

---

## Docker

```bash
docker build -t os-project:latest .
docker run -p 5000:5000 -v $(pwd)/data:/data -e FILE_MANAGER_ROOT=/data os-project:latest

# Compose (recommended)
docker compose up --build
```

Health check: `GET /health` — Docker `HEALTHCHECK` polls it every 30s.

---

## Project Structure

```
.
├── main.cpp              # Secure file-manager CLI (C++17)
├── try.cpp               # C-SCAN / C-LOOK simulator (C++17)
├── app.py                # Flask backend (production-hardened)
├── static/
│   ├── index.html
│   ├── script.js
│   └── styles.css
├── tests/
│   └── test_app.py       # pytest suite for Flask API
├── fixtures/             # Sample inputs for manual testing
├── docs/
│   ├── API.md
│   └── SETUP.md
├── .github/
│   ├── workflows/ci.yml
│   └── ISSUE_TEMPLATE/
├── CMakeLists.txt
├── Makefile
├── Dockerfile
├── docker-compose.yml
├── requirements.txt
├── .clang-format
├── .editorconfig
├── LICENSE
├── ARCHITECTURE.md
├── CONTRIBUTING.md
├── SECURITY.md
└── CHANGELOG.md
```

---

## Security

See [SECURITY.md](SECURITY.md). Key hardening:

- No `system("rm ...")` / `system("ls ...")` — replaced with `std::filesystem` and `Path.resolve().relative_to(WORKSPACE)`.
- Filename allowlist and size limits.
- No debug mode in production; gunicorn with 2 workers.
- Report vulnerabilities via GitHub Security Advisories — do not file public issues.

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). PRs must pass `ci.yml` (C++ build + pytest + lint).

---

## License

[MIT](LICENSE) — Copyright (c) 2026 OS Project Contributors
