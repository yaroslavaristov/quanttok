# Security Policy

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

If you believe you've found a security vulnerability in quanttok — memory
safety issues (this is a C++ project parsing untrusted text, so buffer
overflows / OOB reads in the pre-tokenizer or BPE merge engine are the main
concern), or anything in the Python bindings that could lead to memory
corruption or unexpected code execution — please report it privately:

- **Email:** yaroslavaristov@protonmail.com
- **Telegram (for faster initial contact, not full technical detail):**
  [@yaroslavaristov](https://t.me/yaroslavaristov)

Please include, where possible:

- A description of the vulnerability and its potential impact.
- Steps to reproduce, or a minimal input (e.g. a specific byte sequence /
  crafted text) that triggers the issue.
- The affected version / commit hash.
- Whether the issue is in the C++ core, the Python bindings, or the
  `tools/vocab_baker` offline tooling (the last of these processes
  attacker-controlled vocab files far less commonly, but is still in scope).

## What to Expect

- Acknowledgment of your report within a few days.
- An assessment of severity and, where applicable, an estimated timeline for
  a fix, communicated back to you directly.
- Credit in the release notes / `CHANGELOG.md` once a fix ships, unless you
  prefer to remain anonymous — let us know your preference in the report.

## Scope

In scope:
- `include/quanttok/**` (the C++ core: pre-tokenization, BPE merge engine,
  vocab lookup, compile-time table generation)
- `bindings/python/**` (the Python extension module and its thin wrapper)
- `tools/vocab_baker/**` and `tools/correctness_diff/**` when processing
  externally-supplied vocab/merges files

Out of scope:
- `benchmarks/**` and `examples/**` — these are demonstration code, not
  something exposed to untrusted input in a real deployment.
- Vulnerabilities in third-party dependencies (tiktoken, HF `tokenizers`,
  Google Benchmark, GoogleTest, pybind11) — please report those directly to
  the respective upstream project.

## Supported Versions

quanttok is pre-1.0 and under active development. Until a 1.0 release,
security fixes are only guaranteed for the latest commit on `main` — there
is no backport policy for older tags yet. This section will be updated once
versioning stabilizes.

