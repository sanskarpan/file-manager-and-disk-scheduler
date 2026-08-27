# OS Project

Small operating-systems and systems-programming exercises collected from the original working directory.

## Contents

- `main.cpp`: interactive file-management menu using C++ filesystem and POSIX APIs.
- `try.cpp`: C-SCAN and C-LOOK disk-scheduling comparison.
- `inputty.txt`: sample input data used by the exercises.
- `static/`: browser assets retained from the file-manager exercise.

The Windows executables that were present locally are intentionally excluded from version control. Build native binaries locally with a compiler appropriate for your platform.

## Run

Compile the C++ exercises:

```bash
c++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o main
c++ -std=c++17 -Wall -Wextra -pedantic try.cpp -o disk-scheduling
```

The original `try.py` draft is incomplete/corrupted and is intentionally not tracked. The browser assets are retained as reference rather than presented as a runnable application.

## Notes

The file-manager assets were preserved as coursework. Do not turn them into a networked application without adding authentication, path validation, and safer file-operation handling.
