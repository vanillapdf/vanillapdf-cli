# Coding Style

vanillapdf-cli follows the coding style of the [vanillapdf](https://github.com/vanillapdf/vanillapdf)
library (`.claude/rules/coding-style.md` there). The CLI is a C API consumer, so its closest
model is `src/vanillapdf.tools` in the library repository: when in doubt, do what the tools do.

## Files and Layout

- `.h` / `.cpp` files. The header declares, the source file defines; only templates and
  trivial inline members live in headers.
- Include guards, not `#pragma once`, named after the file:
  ```cpp
  #ifndef _VANILLAPDF_CLI_PAGE_RANGE_H
  #define _VANILLAPDF_CLI_PAGE_RANGE_H
  ...
  #endif /* _VANILLAPDF_CLI_PAGE_RANGE_H */
  ```
- Include order: the file's own header first, then project headers, then third-party
  (vanillapdf, CLI11, fmt), then the standard library, each group separated by a blank line.
- Namespaces are opened one per line and closed with a short comment:
  ```cpp
  namespace vanillapdf {
  namespace cli {
  ...
  } // cli
  } // vanillapdf
  ```
- File-local helpers, option structs and constants go in an anonymous namespace
  (`} // namespace`), not `static`.

## Naming

- Classes and structs: `PascalCase` (`PdfException`, `HandleGuard`, `SplitOptions`).
  Exceptions end in `Exception`.
- Free functions, locals, parameters and struct fields: `snake_case` (`resolve_page_range`,
  `page_count`). The command entry points are `register_<name>` and `process_<name>`.
- Class data members: `m_` prefix (`m_handle`, `m_code`).
- Constants: `UPPER_CASE` (`INFO_FIELDS`, `VANILLAPDF_CLI_ERROR_SUCCESS`).
- Prefer full, descriptive identifiers over abbreviations: `options` not `opts`, `command`
  not `cmd`, `signature` not `sig`, `dictionary` not `dict`, `iterator` not `it`.
  Loop indexes `i` and catch variables `ex` are fine.

## Commands

Each command follows the same shape:

```cpp
namespace {

struct SplitOptions {
    std::string input;
    std::string output;
};

int process_split(const SplitOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");
    ...
    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_split(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<SplitOptions>();
    auto* command = app.add_subcommand("split", "Split pages from a PDF document");
    ...
    command->callback([options, &exit_code]() {
        exit_code = process_split(*options);
    });
}
```

- Commands do not catch errors or call `std::exit`: they throw `PdfException` or
  `PageRangeException`, and `main` reports them once.
- Every option has a short and a long form where a short form exists (`-o,--output`).

## Error Handling

- Wrap every C API call in `THROW_IF_NOT_SUCCESS(call, "Failed to ...")`. The context is the
  user-facing message; the macro also records the call, file and line for `-v`.
- When a non-success result is expected (an optional entry that may be missing), store it in
  a named `error_type` variable and test it, then pass the variable to the macro for the
  remaining failures.
- Throw `PdfException(code, message)` directly for command-level errors (invalid arguments,
  nothing to do).

## Handles

- Own every C API handle with a `HandleGuard` alias (`DocumentGuard`, `BufferGuard`, ...).
  Pass `guard.out()` to Create/Get functions and the guard itself where a handle is
  expected; it converts implicitly.
- Add a missing alias to `utils/handle_guard.h` rather than releasing handles by hand.

## Variable Initialization

- Prefer `= value` over brace initialization `{value}`, and parentheses for constructors:
  `CLI::App app("...")`, not `CLI::App app{"..."}`.
- Return an empty value explicitly: `return std::string();`, not `return {};`.
- One declaration per line: `integer_type year = 0;` then `integer_type month = 0;`.

## Increment and Decrement

- No standalone `i++` / `++i` statements; use `count += 1`.
- Increments in a `for` header (`i++`) and prefix increments inside an expression
  (`entry.index = ++signature_index;`) are fine.

## Control Flow

- Every `if`, `else`, `for` and `while` body is in braces, even a single `return` or `continue`.
- `} else {` on one line.
- Combine consecutive conditions that lead to the same result with `||`.

## Function Bodies

- Opening brace at the end of the signature line, body on the following lines, even for
  one-statement getters. Single-line definitions only for compact tables (`switch` lookups,
  field tables).

## Nested Calls

- Do not pass the result of one function call directly into another; store each intermediate
  result in its own named variable first. `THROW_IF_NOT_SUCCESS(call, ...)` wrapping one C API
  call is the exception.
  - Good:
    ```cpp
    auto permissions = parse_permissions(options.allow);
    THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetUserAccessPermissions(settings, permissions), "...");
    ```
  - Avoid: `DocumentEncryptionSettings_SetUserAccessPermissions(settings, parse_permissions(options.allow))`
- When a variable holds the converted form of another, name it with a `_converted` suffix
  (`page_number_converted`).

## Move Semantics

- Do not use `std::move` unless it is mandatory, for example moving a move-only `HandleGuard`.

## Visual Cohesion

- Do not insert structurally different code into a group of similarly-looking lines without
  separating it with a blank line and a comment.
- Leave a blank line after a closing brace before the next statement.
