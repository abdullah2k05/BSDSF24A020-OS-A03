# BSDSF24A020-OS-A03

A custom UNIX shell written in C for the Operating Systems Programming
Assignment 3.

## Features

- Base fork-exec-wait shell
- Built-in commands: `cd`, `exit`, `help`, `jobs`
- Command history with `history` and `!n`
- GNU Readline integration (tab completion, arrow-key history)
- I/O redirection (`<`, `>`) and pipes (`|`)
- Command chaining (`;`) and background execution (`&`)
- `if ... then ... else ... fi` control structure
- Shell variables (`VAR=value`, `$VAR`, `set`)

## Build

```bash
make
```

## Run

```bash
./bin/myshell
```

## Clean

```bash
make clean
```

## Directory Structure

- `src/` – C source files
- `include/` – header files
- `bin/` – compiled executable (`myshell`)
- `obj/` – intermediate object files
- `Makefile` – build script
