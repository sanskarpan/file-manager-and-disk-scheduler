"""
File Manager - Flask backend (production-hardened).

Reconstructed from scrambled try.py and static/script.js contract.
Security: path traversal prevention, filename allowlist, size limits.
"""
import os
import re
from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory

BASE_DIR = Path(__file__).resolve().parent
STATIC_DIR = BASE_DIR / "static"
# Confine file operations to a sandboxed workspace (project root by default,
# override with FILE_MANAGER_ROOT env var).
WORKSPACE = Path(os.environ.get("FILE_MANAGER_ROOT", str(BASE_DIR))).resolve()

app = Flask(__name__, static_folder=str(STATIC_DIR))

# --- Security helpers ---

FILENAME_RE = re.compile(r"^[A-Za-z0-9._-]{1,255}$")
EXT_RE = re.compile(r"^[A-Za-z0-9_-]{1,32}$")
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB

def safe_path(filename: str) -> Path:
    """Validate filename and return absolute workspace-confined path."""
    if not filename or not FILENAME_RE.match(filename):
        raise ValueError(f"Invalid filename: {filename!r}")
    # Disallow path separators / traversal attempts that sneak past regex
    if "/" in filename or "\\" in filename or ".." in filename:
        raise ValueError("Path separators not allowed")
    p = (WORKSPACE / filename).resolve()
    # Ensure p is within WORKSPACE (handles symlink escapes)
    try:
        p.relative_to(WORKSPACE)
    except ValueError as exc:
        raise ValueError("Path traversal detected") from exc
    return p

def error(msg, code=400):
    return jsonify({"error": msg}), code


# --- Static ---
@app.route("/")
def index():
    return send_from_directory(str(STATIC_DIR), "index.html")

@app.route("/<path:path>")
def serve_static(path):
    # Only serve files that actually live under static/
    target = (STATIC_DIR / path).resolve()
    try:
        target.relative_to(STATIC_DIR.resolve())
    except ValueError:
        return error("Not found", 404)
    if target.is_file():
        return send_from_directory(str(STATIC_DIR), path)
    return error("Not found", 404)


# --- API ---
@app.route("/list-files", methods=["GET"])
def list_files():
    files = [p.name for p in WORKSPACE.iterdir() if p.is_file()]
    return jsonify(sorted(files))

@app.route("/list-directories", methods=["GET"])
def list_directories():
    dirs = [p.name for p in WORKSPACE.iterdir() if p.is_dir()]
    return jsonify(sorted(dirs))

@app.route("/create-file", methods=["POST"])
def create_file():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if p.exists():
        return error("File already exists", 409)
    try:
        p.touch(exist_ok=False)
    except OSError as e:
        return error(str(e), 500)
    return "File created successfully", 200

@app.route("/delete-file", methods=["POST"])
def delete_file():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if not p.is_file():
        return "File not found", 404
    try:
        p.unlink()
    except OSError as e:
        return error(str(e), 500)
    return "File deleted successfully", 200

@app.route("/rename-file", methods=["POST"])
def rename_file():
    data = request.get_json(silent=True) or {}
    old = data.get("oldFileName", "").strip()
    new = data.get("newFileName", "").strip()
    try:
        old_p = safe_path(old)
        new_p = safe_path(new)
    except ValueError as e:
        return error(str(e), 400)
    if not old_p.exists():
        return "File not found", 404
    if new_p.exists():
        return error("Destination already exists", 409)
    try:
        old_p.rename(new_p)
    except OSError as e:
        return error(str(e), 500)
    return "File renamed successfully", 200

@app.route("/edit-file", methods=["POST"])
def edit_file():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    content = data.get("content", "")
    if not isinstance(content, str):
        return error("Content must be a string", 400)
    if len(content.encode("utf-8")) > MAX_FILE_SIZE:
        return error("Content too large", 413)
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if not p.is_file():
        return "File not found", 404
    try:
        p.write_text(content, encoding="utf-8")
    except OSError as e:
        return error(str(e), 500)
    return "File edited successfully", 200

@app.route("/search-file", methods=["POST"])
def search_file():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError:
        return jsonify({"found": False})
    return jsonify({"found": p.exists(), "fileName": file_name})

@app.route("/file-details", methods=["POST"])
def file_details():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if not p.is_file():
        return "File not found", 404
    try:
        st = p.stat()
        details = {
            "name": p.name,
            "size": st.st_size,
            "created": st.st_ctime,
            "modified": st.st_mtime,
            "mode": oct(st.st_mode & 0o777),
        }
    except OSError as e:
        return error(str(e), 500)
    return jsonify(details)

@app.route("/view-file", methods=["GET"])
def view_file():
    file_name = request.args.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if not p.is_file():
        return "File not found", 404
    if p.stat().st_size > MAX_FILE_SIZE:
        return error("File too large to view", 413)
    try:
        return p.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        return error(str(e), 500)

@app.route("/sort-file-contents", methods=["POST"])
def sort_file_contents():
    data = request.get_json(silent=True) or {}
    file_name = data.get("fileName", "").strip()
    try:
        p = safe_path(file_name)
    except ValueError as e:
        return error(str(e), 400)
    if not p.is_file():
        return "File not found", 404
    try:
        lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
        lines.sort()
        p.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")
        return "\n".join(lines)
    except OSError as e:
        return error(str(e), 500)

@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok", "workspace": str(WORKSPACE)})

if __name__ == "__main__":
    # Never run with debug=True in production; controlled via env.
    debug = os.environ.get("FLASK_DEBUG", "0") == "1"
    host = os.environ.get("HOST", "127.0.0.1")
    port = int(os.environ.get("PORT", "5000"))
    app.run(host=host, port=port, debug=debug)
