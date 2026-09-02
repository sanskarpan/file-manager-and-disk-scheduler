import tempfile
from pathlib import Path

import app as app_module


def make_client(tmp_path=None):
    # Use isolated workspace per test
    if tmp_path is None:
        tmp = tempfile.mkdtemp()
        ws = Path(tmp)
    else:
        ws = Path(tmp_path)
        ws.mkdir(parents=True, exist_ok=True)
    app_module.WORKSPACE = ws.resolve()
    app_module.app.config["TESTING"] = True
    return app_module.app.test_client(), ws


def test_health():
    c, _ = make_client()
    r = c.get("/health")
    assert r.status_code == 200
    assert r.get_json()["status"] == "ok"


def test_create_list_view_delete():
    c, ws = make_client()
    # create
    r = c.post("/create-file", json={"fileName": "hello.txt"})
    assert r.status_code == 200
    # list
    r = c.get("/list-files")
    assert "hello.txt" in r.get_json()
    # edit
    r = c.post("/edit-file", json={"fileName": "hello.txt", "content": "b\na\n"})
    assert r.status_code == 200
    # view
    r = c.get("/view-file", query_string={"fileName": "hello.txt"})
    assert r.status_code == 200
    assert "b" in r.get_data(as_text=True)
    # sort
    r = c.post("/sort-file-contents", json={"fileName": "hello.txt"})
    assert r.status_code == 200
    assert r.get_data(as_text=True).splitlines() == ["a", "b"]
    # details
    r = c.post("/file-details", json={"fileName": "hello.txt"})
    assert r.status_code == 200
    assert r.get_json()["name"] == "hello.txt"
    # search found
    r = c.post("/search-file", json={"fileName": "hello.txt"})
    assert r.get_json()["found"] is True
    # rename
    r = c.post("/rename-file", json={"oldFileName": "hello.txt", "newFileName": "hi.txt"})
    assert r.status_code == 200
    r = c.get("/list-files")
    assert "hi.txt" in r.get_json()
    # delete
    r = c.post("/delete-file", json={"fileName": "hi.txt"})
    assert r.status_code == 200
    r = c.post("/search-file", json={"fileName": "hi.txt"})
    assert r.get_json()["found"] is False


def test_path_traversal_blocked():
    c, _ = make_client()
    for bad in ["../app.py", "a/b.txt", "..", ""]:
        r = c.post("/create-file", json={"fileName": bad})
        assert r.status_code == 400
    # view traversal
    r = c.get("/view-file", query_string={"fileName": "../app.py"})
    assert r.status_code == 400
    # edit traversal
    r = c.post("/edit-file", json={"fileName": "../../etc/passwd", "content": "x"})
    assert r.status_code == 400


def test_list_directories():
    c, ws = make_client()
    (ws / "subdir").mkdir()
    r = c.get("/list-directories")
    assert "subdir" in r.get_json()


def test_not_found_cases():
    c, _ = make_client()
    assert c.get("/view-file", query_string={"fileName": "nope.txt"}).status_code == 404
    assert c.post("/delete-file", json={"fileName": "nope.txt"}).status_code == 404
    assert c.post("/file-details", json={"fileName": "nope.txt"}).status_code == 404
    assert c.post("/sort-file-contents", json={"fileName": "nope.txt"}).status_code == 404
    assert c.post("/rename-file", json={"oldFileName": "nope.txt", "newFileName": "x.txt"}).status_code == 404


def test_create_conflict():
    c, _ = make_client()
    c.post("/create-file", json={"fileName": "dup.txt"})
    r = c.post("/create-file", json={"fileName": "dup.txt"})
    assert r.status_code == 409


def test_rename_conflict():
    c, _ = make_client()
    c.post("/create-file", json={"fileName": "a.txt"})
    c.post("/create-file", json={"fileName": "b.txt"})
    r = c.post("/rename-file", json={"oldFileName": "a.txt", "newFileName": "b.txt"})
    assert r.status_code == 409
