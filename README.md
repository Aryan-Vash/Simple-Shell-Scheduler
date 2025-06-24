# Simple Scheduler and Shell Projects

This repository contains two C-based projects:

1. **Simple Scheduler** – A basic implementation of a custom process/task scheduler.
2. **Simple Shell** – A lightweight shell that executes user-defined tasks.

---

## 🔧 Prerequisites

- GCC or any C compiler
- Terminal (Linux/macOS preferred)
- [Visual Studio Code](https://code.visualstudio.com/) with C/C++ extension

---

## 📁 Project Structure

```
.
├── Simple Scheduler/
│   ├── scheduler.c
│   ├── shell.c
│   ├── fib.c, fact.c, henry.c, helloworld.c
│   ├── dummy_main.h
│   ├── file.txt
│   └── Documentation.pdf
│
├── Simple Shell/
│   ├── simple-shell.c
│   ├── fib.c, helloworld.c
│   ├── file.txt
│   └── Documentation.pdf
```

---

## ▶️ How to Run (Using VS Code)

### 1. Open in VS Code

- Open the entire folder (`Simple Scheduler` or `Simple Shell`) in VS Code.
- Ensure C/C++ extension is installed.

### 2. Compile the Code

Open terminal in VS Code (`Ctrl + ~`) and run:

```bash
# For Simple Scheduler
gcc scheduler.c shell.c fib.c fact.c henry.c helloworld.c -o scheduler

# For Simple Shell
gcc simple-shell.c fib.c helloworld.c -o simple_shell
```

### 3. Run the Executable

```bash
./scheduler
# or
./simple_shell
```

---

## 📄 Documentation

Each folder includes a `Documentation.pdf` file that describes the design, structure, and use cases of the corresponding project.

---

## 🛠 Notes

- `.vscode/settings.json` helps configure build/run tasks in VS Code.
- Binaries (`fib`, `fact`, etc.) are included for reference; you can rebuild them using the `.c` files.

---

## 📬 Author

Feel free to modify or extend these projects. Contributions are welcome!
