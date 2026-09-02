# API Reference — Flask File Manager

Base URL: `http://127.0.0.1:5000` (or `HOST:PORT` from env).

All file operations are confined to `FILE_MANAGER_ROOT` (default: project root). Filenames must match `^[A-Za-z0-9._-]{1,255}$`; path separators and `..` are rejected with `400`.

## Health

```
GET /health
→ 200 {"status":"ok","workspace":"/abs/path"}
```

## List Files

```
GET /list-files
→ 200 ["a.txt","b.cpp", ...]  (sorted)
```

## List Directories

```
GET /list-directories
→ 200 ["static","fixtures", ...]
```

## View File

```
GET /view-file?fileName=hello.txt
→ 200 "file content as text/plain"
→ 400 {"error":"Invalid filename..."}
→ 404 "File not found"
→ 413 {"error":"File too large to view"}
```

## Create File

```
POST /create-file
Content-Type: application/json
{"fileName":"hello.txt"}
→ 200 "File created successfully"
→ 400 {"error":"..."}
→ 409 {"error":"File already exists"}
```

## Delete File

```
POST /delete-file
{"fileName":"hello.txt"}
→ 200 "File deleted successfully"
→ 404 "File not found"
```

## Rename File

```
POST /rename-file
{"oldFileName":"a.txt","newFileName":"b.txt"}
→ 200 "File renamed successfully"
→ 404 "File not found"
→ 409 {"error":"Destination already exists"}
```

## Edit File

```
POST /edit-file
{"fileName":"hello.txt","content":"new content"}
→ 200 "File edited successfully"
→ 400 {"error":"Content too large"}  (>10 MiB)
→ 404 "File not found"
```

## Search File

```
POST /search-file
{"fileName":"hello.txt"}
→ 200 {"found":true,"fileName":"hello.txt"}
→ 200 {"found":false}
```

## File Details

```
POST /file-details
{"fileName":"hello.txt"}
→ 200 {"name":"hello.txt","size":123,"created":...,"modified":...,"mode":"0o644"}
→ 404 "File not found"
```

## Sort File Contents

```
POST /sort-file-contents
{"fileName":"hello.txt"}
→ 200 "sorted line1\nsorted line2\n..."
  (also overwrites file on disk)
→ 404 "File not found"
```

## Error Envelope

Validation errors: `400 {"error":"..."}`. Not found: `404` (text or JSON per endpoint). Payload too large: `413`.

## curl Examples

```bash
curl http://127.0.0.1:5000/list-files | jq
curl -X POST -H "Content-Type: application/json" -d '{"fileName":"demo.txt"}' http://127.0.0.1:5000/create-file
curl -X POST -H "Content-Type: application/json" -d '{"fileName":"demo.txt","content":"hello\nworld\n"}' http://127.0.0.1:5000/edit-file
curl "http://127.0.0.1:5000/view-file?fileName=demo.txt"
curl -X POST -H "Content-Type: application/json" -d '{"fileName":"demo.txt"}' http://127.0.0.1:5000/sort-file-contents
curl -X POST -H "Content-Type: application/json" -d '{"fileName":"demo.txt"}' http://127.0.0.1:5000/delete-file
```
