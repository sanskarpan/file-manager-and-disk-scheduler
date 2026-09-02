# Contributing

## Getting Started

1. Fork and clone.
2. Create a feature branch: `git checkout -b feat/your-feature`.
3. Install tools: C++17 compiler, CMake ≥3.16, Python 3.12.
4. Set up Python env:

```bash
python3.12 -m venv .venv && source .venv/bin/activate
pip install -r requirements-dev.txt
```

## Development Workflow

- Format before committing:
  ```bash
  make format   # black + clang-format
  make lint     # ruff + black --check
  ```
- Build & test:
  ```bash
  make cpp && make test
  # or
  cmake -S . -B build && cmake --build build && ctest --test-dir build -V
  pytest tests/ -v
  ```

## Commit Messages

Use Conventional Commits:

```
feat: add C-LOOK direction flag
fix: prevent path traversal in safe_path
docs: expand API examples
chore: bump Flask to 3.1.0
```

## Pull Requests

- Keep PRs focused; one feature/fix per PR.
- Include tests for new behavior.
- Update `CHANGELOG.md` under `## [Unreleased]`.
- Ensure CI passes: C++ build, pytest, ruff/black.

## Code Style

- C++: Google-based `.clang-format`, 100 col, C++17, `-Wall -Wextra -Wpedantic`.
- Python: `black` (88 col), `ruff` lint, type hints where practical.
- No `system("ls")` / `system("rm ...")` — use `std::filesystem` / `pathlib`.

## Reporting Issues

Use the templates in [.github/ISSUE_TEMPLATE](.github/ISSUE_TEMPLATE). Include repro steps, expected vs actual, and environment (`c++ --version`, `python --version`).

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).
