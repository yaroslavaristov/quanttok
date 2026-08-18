# Contributing to quanttok

Thanks for considering contributing. This project is young and moving fast —
read this doc before opening a PR, it'll save both of us time.

## Code of Conduct

This project follows the [Contributor Covenant](CODE_OF_CONDUCT.md). Report
unacceptable behavior to yaroslavaristov@protonmail.com.

## Project layout

Read the top-level `README.md` first for the architecture overview
(pre-tokenization → BPE merge → vocab lookup → batch/serving layer). Rough
map of where things live:

- `include/quanttok/` — the C++ core. Header-only.
- `bindings/python/` — pybind11 extension module + thin Python wrapper.
- `tools/vocab_baker/` — offline tool that converts `.tiktoken`/HF
  `tokenizer.json` vocab files into compile-time C++ headers.
- `benchmarks/cpp/` — isolated micro-benchmarks of internal layers.
- `benchmarks/python/` — comparative benchmarks against tiktoken / HF
  `tokenizers`, run through their real Python APIs (see `README.md` for why
  this split exists — comparing a raw C++ core against a competitor's
  Python API would be a dishonest benchmark).
- `tests/` — GoogleTest suite for the C++ core, including byte-for-byte
  correctness comparisons against golden files (see below).

## Setting up a dev environment

### Requirements

- **Linux only**, for now — the CI matrix and the SIMD dispatch code both
  currently assume Linux/x86_64. Contributions extending this are welcome
  but will need their own CI job before being mergeable.
- **GCC 14+ or a comparably recent Clang.** This project uses C++23
  (`std::print`, `std::format`, concepts). On a fresh Ubuntu 24.04 the
  default `g++` is 13, which does not have `<print>` — install `g++-14`
  explicitly:
  ```bash
  sudo apt-get install -y g++-14
  ```
- **CMake ≥ 3.25.**
- **Python 3.10+** for the bindings and comparative benchmarks.

### Building the C++ core, tests, and benchmarks

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Useful CMake options — see `CMakeLists.txt` for the full list:

| Option                          | Effect                                              |
|----------------------------------|------------------------------------------------------|
| `QUANTTOK_BUILD_TESTS`          | Build the GoogleTest suite (default `ON`)            |
| `QUANTTOK_BUILD_BENCHMARKS`     | Build the C++ Google Benchmark binaries              |
| `QUANTTOK_BUILD_EXAMPLES`       | Build the C++ example binaries                       |
| `QUANTTOK_WARNINGS_AS_ERRORS`   | Treat every compiler warning as a build failure       |
| `QUANTTOK_ENABLE_CLANG_TIDY`    | Run clang-tidy as part of the build (see `.clang-tidy`) |
| `QUANTTOK_ENABLE_ASAN` / `_UBSAN` | Sanitizer instrumentation for Debug builds          |
| `QUANTTOK_ENABLE_TSAN`          | ThreadSanitizer — for `batch/` / `spsc_pipeline.hpp` work; mutually exclusive with ASan/UBSan |

### Installing the Python bindings

The publishable package lives in `bindings/python/` (its `pyproject.toml` is
the one that actually gets built into a wheel for PyPI). The root
`pyproject.toml` exists purely for contributor convenience — it does **not**
publish a separate package, it just points `pip` at the real one so you can
install from the repo root without knowing the internal layout:

```bash
pip install -e .
```

This is equivalent to `pip install -e bindings/python/` — both work; the
root command is just shorter. Root `pyproject.toml` also holds shared dev
tooling config (`[tool.ruff]`, `[tool.pytest.ini_options]`) so linting and
testing behave consistently regardless of which directory you run them from.

### Running the Python binding tests

```bash
pip install -e .[dev]
pytest bindings/python/tests/
```

### Running comparative benchmarks

```bash
pip install -r benchmarks/python/requirements.txt
python benchmarks/python/bench_vs_tiktoken.py
python benchmarks/python/bench_vs_hf_tokenizers.py
```

Both scripts run against the **same vocab and the same corpus** — see
`benchmarks/corpora/README.md` for corpus provenance. Don't add a new
comparison benchmark that uses a different vocab than the others; that
produces numbers that look like a regression/improvement but are actually
just an apples-to-oranges comparison. `bench_summary.py` aggregates both
into the table/chart used in the top-level `README.md`.

## Correctness first, speed second

Before optimizing anything, a change must not break byte-for-byte parity
with the reference tokenizers on the existing corpus. The flow:

1. `tools/correctness_diff/` runs tiktoken / HF `tokenizers` (their real
   Python implementations) over `benchmarks/corpora/*.txt` and produces
   golden token-id files.
2. `tests/test_correctness_vs_tiktoken.cpp` and
   `tests/test_correctness_vs_hf.cpp` compare quanttok's C++ output against
   those golden files directly — no Python in that test path, so a failure
   points at the algorithm, not the bindings.

If you're touching `pretokenize/` or `bpe/`, run these tests before and
after your change. A performance improvement that changes output is a bug,
not an optimization — open an issue to discuss it before changing expected
behavior.

## Style

- **Formatting:** `.clang-format` (LLVM-based, 120 col). Run
  `clang-format -i` on changed files, or use `pre-commit` (below) to do it
  automatically.
- **Static analysis:** `.clang-tidy` — curated check set (see the file
  itself for what's enabled and why). Runs in CI on every PR; you can run it
  locally with `-DQUANTTOK_ENABLE_CLANG_TIDY=ON`.
- **Doc comments:** Doxygen-style `///` in `.hpp` public headers (see
  `include/quanttok/bpe/vocab.hpp` for the expected level of detail once
  it exists). Inline `//` comments in `.cpp`/`.inl` files explaining *why*,
  not *what* — the code should already say what it does.
- **Naming:** see `.clang-tidy`'s `readability-identifier-naming` section
  for the enforced convention (classes/structs `CamelCase`, functions/
  variables `lower_case`, private members trailing `_`, `constexpr` values
  `kCamelCase`).

### Optional: pre-commit hooks

`.pre-commit-config.yaml` is in the repo but does nothing until you opt in:

```bash
pip install pre-commit
pre-commit install
```

After that, `clang-format`, `ruff`, and a few basic hygiene checks (trailing
whitespace, large files, line endings) run automatically on `git commit`.
This is a local convenience, not a substitute for CI — a PR is judged by
`.github/workflows/ci.yml`, not by whether you ran this.

## Architectural contracts — read before touching `batch/spsc_pipeline.hpp`

If your change touches the SPSC serving pipeline, the same single-producer/
single-consumer discipline documented for `ferrolink` applies here — exactly
one producer, exactly one consumer, no soft-failure fallback on
misconfiguration. If you're not already familiar with that class of
constraint, ask before submitting a PR that touches `batch/spsc_pipeline.hpp`
or `detail/arena_allocator.hpp`.

## Submitting a PR

1. Open an issue first for anything that isn't a small, obvious fix —
   especially anything touching `compiletime/` or the merge engine, where a
   change in approach has ripple effects across generated code.
2. Keep PRs focused. A PR that mixes a formatting pass with a behavioral
   change is harder to review and harder to bisect later.
3. Update `NOTICE` in the same PR if you add, remove, or replace a
   build/test dependency.
4. Update `CHANGELOG.md` under `[Unreleased]`.
5. CI must be green: build (multiple compilers), tests, clang-tidy,
   correctness-vs-golden-files. Benchmark regression checks post a comment
   on the PR — a regression isn't an automatic block, but it needs an
   explanation in the PR description.

## Questions

Open a GitHub Discussion, or reach out directly:
- Email: yaroslavaristov@protonmail.com
- Telegram: [@yaroslavaristov](https://t.me/yaroslavaristov)
