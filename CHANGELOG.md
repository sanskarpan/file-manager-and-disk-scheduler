# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-09-02

### Added
- Production scaffolding: `CMakeLists.txt`, `Makefile`, `Dockerfile`, `docker-compose.yml`, `.editorconfig`, `.clang-format`, `.env.example`.
- Flask backend `app.py` (reconstructed & hardened from scrambled `try.py`) with `safe_path` sandboxing, size limits, `/health`, and `gunicorn` support.
- `requirements.txt` / `requirements-dev.txt` with pinned versions.
- Documentation suite: `README.md`, `ARCHITECTURE.md`, `CONTRIBUTING.md`, `SECURITY.md`, `CHANGELOG.md`, `LICENSE` (MIT), `docs/API.md`, `docs/SETUP.md`.
- CI workflow `.github/workflows/ci.yml` (C++ build, CTest, pytest, ruff/black).
- GitHub templates: issue, bug report, PR template.
- Tests: `tests/test_app.py` (pytest) covering all Flask endpoints + traversal protection.
- Fixtures: `fixtures/sample.txt`, `fixtures/input_requests.txt`.

### Changed
- `main.cpp`: rewrote to secure modern C++17 — `std::filesystem` replaces `system("ls"/"rm")`, filename allowlisting, `unique_ptr` RAII, robust input handling, fixed `processChoice` exit logic.
- `try.cpp`: replaced duplicate file-manager code with proper C-SCAN/C-LOOK simulator (pure functions, validation, `--test` self-test).
- `.gitignore`: comprehensive OS/editor/C++/Python/Docker ignores; removed incorrect `try.py`/`prac.js` source ignores.

### Fixed
- `try.cpp` off-by-one bug: `choice >=0 && choice < size` with `operations[choice-1]` caused segfault on `0`; fixed to `choice==0` early return.
- `try.py` scrambled content: replaced with validated `app.py`.
- `prac.js` empty file: removed (no longer tracked).

### Removed
- Binaries `main.exe` / `try.exe` from working tree (now `.gitignore`'d; build locally).
- Junk fixtures `12mb.txt` / `inputty.txt` (Huffman fragments) moved to `fixtures/` as curated samples.

[Unreleased]: https://github.com/anomalyco/OS_Project/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/anomalyco/OS_Project/releases/tag/v1.0.0
