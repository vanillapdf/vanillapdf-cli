#!/usr/bin/env python3
"""Update the vcpkg pin to the latest release tag and optionally open a pull request.

Ported from scripts/update_vcpkg.py in the vanillapdf repository. That script
moves a git submodule and a builtin-baseline; here vcpkg is bootstrapped with
FetchContent, so the single source of truth is the GIT_TAG in the pin file.
Every dependency linked into the release binaries, OpenSSL included, comes from
the ports at that tag.

Run from update-vcpkg.yml, which supplies --vcpkg-repository and --pin-file.

Requirements: git, and the gh CLI unless --no-pr is passed.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path
from typing import Optional

PIN_PATTERN = re.compile(r"(GIT_TAG\s+)(\d{4}\.\d{2}\.\d{2})")
TAG_PATTERN = re.compile(r"refs/tags/(\d{4}\.\d{2}\.\d{2})$")


def run(cmd: list, cwd: Path, check: bool = True) -> subprocess.CompletedProcess:
    print(f"Running: {' '.join(cmd)}")
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, check=check)


def current_version(pin_file: Path) -> str:
    match = PIN_PATTERN.search(pin_file.read_text(encoding="utf-8"))
    if not match:
        raise RuntimeError(f"No vcpkg GIT_TAG found in {pin_file}")
    return match.group(2)


def latest_version(repo_root: Path, repository: str) -> str:
    result = run(["git", "ls-remote", "--tags", "--refs", repository], repo_root)
    tags = [m.group(1) for line in result.stdout.splitlines() if (m := TAG_PATTERN.search(line))]
    if not tags:
        raise RuntimeError("No release tags found in the vcpkg repository")
    # YYYY.MM.DD sorts chronologically as text.
    return max(tags)


def update_pin(pin_file: Path, version: str) -> None:
    # newline="" keeps the file's existing line endings.
    with open(pin_file, encoding="utf-8", newline="") as fh:
        text = fh.read()
    with open(pin_file, "w", encoding="utf-8", newline="") as fh:
        fh.write(PIN_PATTERN.sub(rf"\g<1>{version}", text, count=1))


def push_branch(repo_root: Path, branch: str) -> None:
    # Fetch the remote-tracking ref so --force-with-lease has a baseline. In a
    # shallow CI clone only the default branch is fetched, and git refuses the
    # push when the remote branch exists without one. Failing is fine: the
    # branch usually does not exist yet.
    run(["git", "fetch", "origin", f"refs/heads/{branch}:refs/remotes/origin/{branch}"],
        repo_root, check=False)
    run(["git", "push", "--force-with-lease", "origin", branch], repo_root)


def create_pull_request(repo_root: Path, current: str, latest: str, branch: str) -> Optional[str]:
    existing = run(["gh", "pr", "list", "--head", branch, "--state", "open",
                    "--json", "url", "--jq", ".[0].url"], repo_root, check=False)
    if existing.returncode == 0 and existing.stdout.strip():
        print(f"Pull request already exists: {existing.stdout.strip()}")
        return existing.stdout.strip()

    body = (
        f"Updates the vcpkg pin from `{current}` to `{latest}`.\n\n"
        "The release binaries link their dependencies statically, so this is how "
        "they pick up fixes in OpenSSL, zlib, and the other ports. Review the "
        "dependency versions in the build log before merging.\n\n"
        "Created by `scripts/update_vcpkg.py`."
    )
    result = run(["gh", "pr", "create", "--title", f"build: Update vcpkg to {latest}",
                  "--body", body, "--base", "main", "--head", branch], repo_root)
    print(f"Created pull request: {result.stdout.strip()}")
    return result.stdout.strip()


def main() -> None:
    parser = argparse.ArgumentParser(description="Update the vcpkg pin to the latest release tag")
    parser.add_argument("--vcpkg-repository", required=True, help="git URL of the vcpkg repository")
    parser.add_argument("--pin-file", required=True, type=Path,
                        help="file holding the vcpkg GIT_TAG, relative to the repository root")
    parser.add_argument("--force", action="store_true",
                        help="rewrite the branch even if already on the latest tag")
    parser.add_argument("--no-pr", action="store_true", help="skip pull request creation")
    parser.add_argument("--dry-run", action="store_true",
                        help="show what would be done without making changes")
    parser.add_argument("--repo-root", type=Path, default=Path.cwd(),
                        help="path to the repository root (default: current directory)")
    args = parser.parse_args()
    repo_root = args.repo_root.resolve()
    pin_file = repo_root / args.pin_file

    try:
        current = current_version(pin_file)
        latest = latest_version(repo_root, args.vcpkg_repository)
        print(f"Current vcpkg version: {current}")
        print(f"Latest vcpkg version: {latest}")

        if current == latest and not args.force:
            print("No update needed - already on the latest version")
            return

        branch = f"automated/update-vcpkg-{latest}"
        if args.dry_run:
            print(f"DRY RUN: Would update {args.pin_file} from {current} to {latest}")
            print(f"DRY RUN: Would push branch {branch}")
            if not args.no_pr:
                print("DRY RUN: Would create a pull request")
            return

        run(["git", "checkout", "-B", branch], repo_root)
        update_pin(pin_file, latest)
        run(["git", "add", str(args.pin_file)], repo_root)

        if run(["git", "diff", "--cached", "--quiet"], repo_root, check=False).returncode == 0:
            print("No changes to commit")
            return

        run(["git", "commit", "-s", "-m", f"build: Update vcpkg to {latest}"], repo_root)
        push_branch(repo_root, branch)
        if not args.no_pr:
            create_pull_request(repo_root, current, latest, branch)
    except subprocess.CalledProcessError as error:
        print(f"Error: {error}\n{error.stderr}")
        sys.exit(1)
    except (OSError, RuntimeError) as error:
        print(f"Error: {error}")
        sys.exit(1)


if __name__ == "__main__":
    main()
