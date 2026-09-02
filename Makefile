CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
PYTHON ?= python3.12

BUILD_DIR := build

.PHONY: all clean cpp python test lint format docker help

all: cpp

cpp: file_manager disk_scheduling

file_manager: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o file_manager

disk_scheduling: try.cpp
	$(CXX) $(CXXFLAGS) try.cpp -o disk_scheduling

# CMake alternative
cmake:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --parallel

python:
	$(PYTHON) -m pip install -r requirements.txt

test: cpp
	@echo "== Disk scheduling self-test =="
	./disk_scheduling_test --test 2>/dev/null || ( $(CXX) $(CXXFLAGS) -DDISK_SCHEDULING_TEST=1 try.cpp -o /tmp/disk_test && /tmp/disk_test --test )
	@echo "== Python tests =="
	$(PYTHON) -m pytest tests/ -v --cov=app --cov-report=term-missing

lint:
	ruff check app.py
	black --check app.py

format:
	black app.py
	clang-format -i main.cpp try.cpp

clean:
	rm -rf $(BUILD_DIR) file_manager disk_scheduling disk_scheduling_test *.o
	rm -rf .pytest_cache __pycache__ .coverage htmlcov

docker:
	docker build -t os-project:latest .

help:
	@grep -E '^[a-zA-Z_-]+:.*?##' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
