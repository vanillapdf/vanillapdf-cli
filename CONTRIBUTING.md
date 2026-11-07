# Contributing to vanillapdf-cli

Thank you for considering contributing to **vanillapdf-cli**! Whether you're
fixing bugs, improving documentation, or proposing new commands — your input is
welcome.

This is the command-line front end for [vanillapdf](https://github.com/vanillapdf/vanillapdf),
a cross-platform PDF library. Issues in PDF parsing, encryption, or signing
internals usually belong in the library repository; issues in argument handling,
command behaviour, or output formatting belong here.

---

## 🐞 Reporting Bugs

1. Search the [GitHub Issues](https://github.com/vanillapdf/vanillapdf-cli/issues)
   to see whether it's already reported.
2. Include the following in a new report:
   - `vanillapdf-cli --version` output (CLI and linked library version)
   - Operating system and version
   - The exact command you ran
   - What happened vs. what you expected
   - A sample PDF that reproduces the issue, anonymized if needed

Security issues must **not** be filed as public issues — see [SECURITY.md](SECURITY.md).

---

## 💡 Feedback & Feature Requests

Missing a command or an option? Open an issue describing the use case, or send a
note to [info@vanillapdf.com](mailto:info@vanillapdf.com).

---

## 🛠️ Building and Testing

```sh
cmake -B build          # auto-bootstraps vcpkg, fetches vanillapdf
cmake --build build
ctest --test-dir build --output-on-failure          # Linux / macOS
ctest --test-dir build -C Debug --output-on-failure  # Windows
```

See [Building from source](https://vanillapdf.readthedocs.io/projects/cli/en/latest/building/) for
all configurations (system packages, existing vcpkg, static builds).

New commands follow the established pattern: each lives in
`src/commands/<name>.cpp` and exposes a `register_<name>(CLI::App&, int&)`
function declared in `commands/commands.h` and called from `main.cpp`. Please add tests under
`tests/` for new behaviour, and update the docs (`README.md`, `docs/commands/`,
and the table of contents in `docs/index.md`) alongside the code. Preview the
docs locally with:

```sh
pip install -r docs/requirements.txt
sphinx-build -W -b html docs docs/_build/html
```

---

## 🔁 Submitting Pull Requests

**🚨 Branch and PR workflow**
- **All changes go through a branch and pull request.** `main` is protected;
  never commit to it directly.
- Create a branch for every change: `feature/description` or `fix/description`.

**Pull request guidelines**
- 🧠 **Discuss major changes first** — for anything beyond a bug fix, open an
  issue before investing time.
- 🌳 **Base your PR on `main`.**
- 🧹 **Match the existing coding style** (the vanillapdf style: see
  [.claude/rules/coding-style.md](.claude/rules/coding-style.md)).
- 🧪 **Ensure tests pass** before submitting.
- ✅ **Sign your commits** (see DCO below).
- 📝 **Use clear commit messages**; conventional-commit prefixes (`fix:`,
  `docs:`, `feat:`) are appreciated.

---

## ✅ Developer Certificate of Origin (DCO)

By submitting a contribution, you certify that:

> "I certify that I have the right to submit this code under the open source
> license indicated in this repository and that I am doing so in good faith."

Sign off every commit with the `-s` flag:

```sh
git commit -s -m "Add rotate --clockwise alias"
```

which appends:

```
Signed-off-by: Your Name <you@example.com>
```

See [developercertificate.org](https://developercertificate.org) for details.

---

## 🔐 Release Tag Signing

Release tags are GPG-signed by a maintainer so users can verify a release is
authentic and untampered:

```sh
git tag -v v0.1.0   # verify a signed tag
```

Maintainer public keys are available on the
[GitHub organization profile](https://github.com/vanillapdf) and via public
keyservers.

---

## 🙏 Thank You!

Every bug report, idea, and pull request makes vanillapdf-cli better. We
appreciate your time and contributions!
