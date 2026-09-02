# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 1.x     | Yes       |

## Hardening Measures

- **C++ CLI**: all `system("ls")` / `system("rm ...")` removed; `std::filesystem` used. Filenames validated against `^[A-Za-z0-9._-]{1,255}$`; path separators and `..` rejected. `delete` refuses directories. `$EDITOR` allowlisted.
- **Flask**: `safe_path()` enforces `WORKSPACE` confinement via `Path.resolve().relative_to(WORKSPACE)` (blocks symlink escapes). `MAX_FILE_SIZE` (10 MiB) limits view/edit/sort. No `FLASK_DEBUG` in production; served via `gunicorn`.
- **Docker**: non-root workspace `/data`, `HEALTHCHECK` on `/health`.

## Reporting a Vulnerability

**Do not file a public issue.**

- Use GitHub Security Advisories: `Security` → `Report a vulnerability`, or email the maintainer.
- Include: description, impact, reproduction steps, and suggested fix if any.
- We aim to acknowledge within 48 hours and release a fix within 14 days.

## Known Limitations

- No authentication on the web UI — do not expose to untrusted networks without adding auth (see `ARCHITECTURE.md` Future).
- `EditFileOperation` still uses `system()` with a quoted filename and allowlisted editor; a future version will use `fork/exec` or a built-in editor.

## Secure Deployment Checklist

- [ ] Set `FILE_MANAGER_ROOT` to an isolated directory (e.g., Docker volume `/data`).
- [ ] Run with `FLASK_DEBUG=0` and `gunicorn` (not `flask run`).
- [ ] Put behind a reverse proxy with TLS if exposed.
- [ ] Add authentication/authorization.
- [ ] Regularly update `requirements.txt` (`pip-audit` recommended).
