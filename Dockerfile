FROM python:3.12-slim AS base

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1

WORKDIR /app

# System deps for C++ build (optional)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

# Build C++ binaries
RUN c++ -std=c++17 -Wall -Wextra -O2 main.cpp -o /usr/local/bin/file_manager && \
    c++ -std=c++17 -Wall -Wextra -O2 try.cpp -o /usr/local/bin/disk_scheduling

EXPOSE 5000

ENV FILE_MANAGER_ROOT=/data \
    HOST=0.0.0.0 \
    PORT=5000

VOLUME ["/data"]

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD python -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:5000/health').read()" || exit 1

CMD ["gunicorn", "--bind", "0.0.0.0:5000", "--workers", "2", "--threads", "4", "app:app"]
