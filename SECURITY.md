# Security Policy

## Overview

This document outlines the security support policy and responsible vulnerability
disclosure process for **vanillapdf-cli**, the command-line front end for the
[vanillapdf](https://github.com/vanillapdf/vanillapdf) library.

Because the CLI is a thin layer over the library, many security-relevant
behaviours (PDF parsing, encryption, digital signatures) live in vanillapdf
itself. Vulnerabilities in those areas may be redirected to, or coordinated
with, the [vanillapdf security process](https://github.com/vanillapdf/vanillapdf/security/policy).

## Supported Versions

The following vanillapdf-cli versions currently receive **security fixes**:

| Version | Supported |
| ------- | --------- |
| 0.1.x   | ✅        |
| < 0.1   | ❌        |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it **privately and
responsibly**:

- **GitHub (preferred):** Submit a draft security advisory at
  [github.com/vanillapdf/vanillapdf-cli/security/advisories](https://github.com/vanillapdf/vanillapdf-cli/security/advisories)
- **Email (alternative):** Contact us at `security@vanillapdf.com`

> ⚠️ **Do NOT open a public GitHub issue for security vulnerabilities.** Public
> issues are visible to everyone and may put users at risk before a fix is
> available.

### What to Include in Your Report

- Affected version(s) of vanillapdf-cli (and the linked vanillapdf version)
- A clear description of the vulnerability and its potential impact
- Steps to reproduce, including any relevant PDF files, commands, or
  proof-of-concept
- The environment (OS, compiler, build configuration) where it was observed
- Your assessment of severity, if possible

## Severity Classification

| Severity | Description |
| -------- | ----------- |
| **Critical** | Remote code execution, memory corruption exploitable without user interaction |
| **High** | Significant data exposure, privilege escalation, or exploitable crash |
| **Medium** | Limited impact vulnerabilities requiring specific conditions to trigger |
| **Low** | Minor issues with negligible security impact |

## Response Timeline

- **Acknowledgement:** Within 72 hours of receiving the report
- **Initial assessment:** Within 7 days
- **Fix or mitigation:** Within 7–14 days for High/Critical; up to 30 days for
  Medium/Low, depending on complexity

## Disclosure Policy

- We will notify you when the fix is ready and coordinate a disclosure date.
- We aim to publish a GitHub security advisory and patch release simultaneously.
- We ask that you refrain from public disclosure until the fix has been released.
- If we cannot resolve the issue within a reasonable timeframe, we will work with
  you to agree on an appropriate disclosure date.

## Reporter Credit

We are grateful to security researchers who help keep vanillapdf-cli safe. With
your permission, we will acknowledge your contribution in the published advisory.

---

_Last updated: July 2026_
