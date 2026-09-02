# Setup Guide

## Prerequisites

- C++17: `clang++ --version` or `g++ --version`
- CMake ≥ 3.16, Make
- Python 3.12 (or 3.11+), pip
- Docker (optional)

## Local Development

```bash
git clone <repo> && cd OS_Project

# Python env
python3.12 -m venv .venv
source .venv/bin/activate
pip install -r requirements-dev.txt

# C++ quick build
make cpp
./file_manager
./disk_scheduling --test 2>/dev/null || ./disk_scheduling

# CMake + CTest
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build -V

# Flask dev
python app.py
# → http://127.0.0.1:5000

# Tests
pytest tests/ -v --cov=app
```

## Production (gunicorn)

```bash
pip install -r requirements.txt
gunicorn --bind 0.0.0.0:5000 --workers 2 --threads 4 app:app
```

## Docker

```bash
docker compose up --build
# or
docker build -t os-project:latest .
docker run -p 5000:5000 -v $(pwd)/data:/data -e FILE_MANAGER_ROOT=/data os-project:latest
```

## Environment

Copy `.env.example` to `.env`:

```
FILE_MANAGER_ROOT=./data
HOST=127.0.0.1
PORT=5000
FLASK_DEBUG=0
```

`FILE_MANAGER_ROOT` is the sandbox for all Flask file ops. In Docker it is `/data` (mounted volume).

## Troubleshooting

- **Flask not found**: ensure `.venv` is activated and `pip install -r requirements.txt` succeeded.
- **C++ filesystem link error (old GCC)**: add `-lstdc++fs` or upgrade GCC ≥ 9.
- **Port 5000 in use (macOS AirPlay)**: set `PORT=5001` or disable AirPlay Receiver in System Settings.
