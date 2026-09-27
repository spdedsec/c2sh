# c2sh — C → Shell

A production-minded, Linux-first interactive Unix shell written in C17.

`c2sh` is designed to be something you can actually use every day while remaining a readable systems-programming project. It implements a real shell execution core instead of delegating commands to `system(3)`.

## What it is

- Interactive POSIX-style command execution
- Process creation with `fork()`/`execvp()`
- Pipelines and redirections
- Background jobs and job control
- Signals and process groups
- Builtins that must run inside the shell (`cd`, `export`, `unset`, etc.)
- Environment and parameter expansion
- Tilde expansion and pathname globbing
- Quotes and backslash escaping
- `&&`, `||`, `;`, and `&` command lists
- Persistent history
- A lightweight terminal line editor (no readline dependency)
- Tab completion for commands and paths
- Reverse history search (`Ctrl-R`)
- Git-aware prompt context when available
- Config file support (`~/.c2shrc`)
- Script/non-interactive mode (`c2sh -c`, `c2sh script.c2sh`)
- Syntax-aware error messages and exit statuses
- Strict compiler warnings and automated tests

## Design philosophy

The shell is split into five major stages:

```text
terminal → line editor → lexer/parser → expansion → executor → kernel
                                      ↑                 ↓
                                   builtins        jobs/signals
```

Parsing is deliberately separated from execution. This makes the command language testable and prevents process-management code from becoming coupled to raw input strings.

## Requirements

Linux or another POSIX-like system with:

- C17 compiler (GCC or Clang)
- GNU Make
- POSIX APIs (`fork`, `exec`, `pipe`, `termios`, `glob`, etc.)

No runtime framework is required.

## Build

```sh
make
make test
./build/c2sh
```

For an optimized build:

```sh
make CFLAGS='-std=c17 -O2 -DNDEBUG -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2'
```

Install locally:

```sh
sudo make install PREFIX=/usr/local
```

User-local install:

```sh
make install PREFIX="$HOME/.local"
```

## Usage

```sh
c2sh
c2sh -c 'printf "%s\\n" hello | tr a-z A-Z'
c2sh ./script.c2sh
```

Examples:

```sh
printf '%s\\n' hello | tr a-z A-Z
ls -la > listing.txt
cat < listing.txt | grep README
sleep 30 &
jobs
fg %1
```

## Builtins

`cd`, `pwd`, `echo`, `export`, `unset`, `env`, `history`, `jobs`, `fg`, `bg`, `kill`, `type`, `which`, `help`, `source`, `true`, `false`, `exit`.

Use `help` inside the shell for details.

## Configuration

On startup c2sh reads `~/.c2shrc` if it exists. It is interpreted as shell input. Keep this file under your own control; like any shell startup file, it can execute commands. Set `C2SH_NORC=1` to skip it for a session.

Useful variables:

- `C2SH_NO_COLOR=1` — disable prompt colors
- `C2SH_HISTORY_FILE` — override history path
- `C2SH_HISTORY_SIZE` — history limit (default 5000)

## Compatibility

c2sh intentionally aims for **interactive Unix shell compatibility**, not byte-for-byte Bash compatibility. Shell scripting is a separate compatibility target. Scripts that depend on Bash-specific syntax should continue to use Bash.

The initial language intentionally excludes advanced Bash-only features such as arrays, `[[ ]]`, process substitution, traps, shell functions, and command substitution. These can be added without redesigning the executor because the parser already produces an intermediate command representation.

## Security model

A shell necessarily executes programs with the user's privileges. c2sh does not sandbox commands. Treat shell scripts and startup files as executable code.

The project avoids `system()` and `popen()` for command execution. External commands are launched directly with `execvp()` after the parser has constructed argv arrays. This prevents an accidental second shell interpretation layer.

History is written with restrictive permissions where possible. Commands containing secrets should not be placed in shell history.

## Architecture

```text
include/
  shell.h       shared state and public interfaces
  lexer.h       tokenization
  parser.h      command AST / command lists
  editor.h      terminal line editor
  executor.h    process and pipeline execution
  builtins.h    shell builtins
  jobs.h        job table and process groups

src/
  main.c
  shell.c
  lexer.c
  parser.c
  expand.c
  editor.c
  executor.c
  builtins.c
  jobs.c
  prompt.c
  history.c
  util.c

The code intentionally keeps modules small. `main.c` only coordinates lifecycle; it does not contain the shell implementation.
```

## Testing

`make test` compiles a small test runner and exercises tokenization, parsing, expansion and a subprocess smoke test. Integration tests in `tests/integration.sh` cover pipelines, redirection, status propagation and background execution.

For memory/safety testing:

```sh
make clean
make CFLAGS='-std=c17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic'
make test
```

For Linux syscall-level debugging:

```sh
strace -f ./build/c2sh -c 'printf hello | wc -c'
```

## Release engineering

The canonical public repository should be hosted on GitHub. Git tags and GitHub Releases provide versioned source archives and attachable binaries. GitHub documents Releases as deployable software iterations that can package binaries and release notes.

Recommended release pipeline:

1. Merge into `main`.
2. Run compiler warnings, unit tests and integration tests.
3. Run sanitizer tests.
4. Build release artifacts for supported architectures.
5. Generate SHA-256 checksums.
6. Create an annotated `vX.Y.Z` tag.
7. Publish a GitHub Release with changelog and artifacts.
8. Publish Linux packages as the project matures.

Suggested distribution targets:

- GitHub Releases — source tarball and portable Linux binaries.
- Homebrew — macOS/Linux formula once the release cadence stabilizes.
- Debian/Ubuntu package — `.deb` once packaging metadata is mature.
- Arch Linux — PKGBUILD maintained by the project or community.
- AUR — community distribution, not the primary source of truth.

There is no server to deploy: this is a local system program. The production infrastructure is CI + release artifacts + package repositories, not Vercel.

## Versioning

Semantic Versioning is recommended:

- `0.x` while the command language is still evolving.
- `1.0.0` once the language and job-control behavior are considered stable.

Changing parsing semantics or builtin behavior should be treated as a compatibility concern and documented in `CHANGELOG.md`.

## Roadmap

### 0.1
Core REPL, parser, expansion, redirection, pipelines, builtins, history, basic jobs.

### 0.2
Better completion, richer history search, aliases, configuration, improved diagnostics, test expansion.

### 0.3
More complete interactive job control, shell functions, command substitution, startup profiles and portable build matrix.

### 0.4+
Plugin/extension API only if a stable need emerges. Avoid a plugin system until the core ABI is mature.

### 1.0
Stable command language, documented compatibility guarantees, Linux/macOS CI, packages, signed releases.

## Development workflow

```sh
make format-check
make test
make sanitize
```

Before a release:

```sh
make clean all test
make sanitize
./build/c2sh -c 'printf "%s\\n" "release smoke test"'
```

## License

GPL-3.0-or-later. See `LICENSE`.
