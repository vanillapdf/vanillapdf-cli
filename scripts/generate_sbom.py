#!/usr/bin/env python3
"""Generate the SPDX SBOM of a release from the manifests vcpkg wrote while building.

GitHub's dependency graph cannot see vcpkg or FetchContent dependencies, so it says
nothing about what is linked into the binaries. vcpkg records every port it builds
in <installed>/<triplet>/share/<port>/vcpkg.spdx.json. vcpkg also installs things that
never reach the binary, so vcpkg.json decides what is listed and the manifests supply
the details, merged from all platforms. The CLI and vanillapdf, which vcpkg does not
build, are added by hand.

Run from the release workflow, which supplies every value; see --help.
"""

import argparse
import json
import re
import sys
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Set, Tuple

# The vcpkg.json feature that only the test suite uses.
TEST_FEATURE = "tests"


def spdx_id(name: str) -> str:
    return "SPDXRef-" + re.sub(r"[^A-Za-z0-9.-]", "-", name)


def package(name: str, version: str, location: str, license_id: str = "NOASSERTION",
            homepage: str = "") -> dict:
    result = {
        "name": name,
        "SPDXID": spdx_id(name),
        "versionInfo": version,
        "downloadLocation": location,
        "filesAnalyzed": False,
        "licenseConcluded": license_id,
        "licenseDeclared": license_id,
        "copyrightText": "NOASSERTION",
    }
    if homepage:
        result["homepage"] = homepage
    return result


def shipped_ports(vcpkg_manifest: Path) -> Set[str]:
    manifest = json.loads(vcpkg_manifest.read_text(encoding="utf-8"))
    return {
        dependency if isinstance(dependency, str) else dependency["name"]
        for name, feature in manifest["features"].items() if name != TEST_FEATURE
        for dependency in feature["dependencies"]
    }


def collect_ports(input_dir: Path, shipped: Set[str]) -> Dict[Tuple[str, str], dict]:
    ports = {}
    for path in sorted(input_dir.rglob("vcpkg.spdx.json")):
        document = json.loads(path.read_text(encoding="utf-8"))
        port = next(p for p in document["packages"] if p["SPDXID"] == "SPDXRef-port")
        if port["name"] not in shipped:
            continue
        ports[(port["name"], port["versionInfo"])] = package(
            port["name"],
            port["versionInfo"],
            port.get("downloadLocation", "NOASSERTION"),
            port.get("licenseConcluded", "NOASSERTION"),
            port.get("homepage", ""),
        )
    return ports


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("input_dir", type=Path, help="directory searched for vcpkg.spdx.json")
    parser.add_argument("--version", required=True, help="vanillapdf-cli version being released")
    parser.add_argument("--vanillapdf-tag", required=True, help="vanillapdf tag the release links")
    parser.add_argument("--repository", required=True, help="URL of the repository being released")
    parser.add_argument("--vcpkg-manifest", required=True, type=Path,
                        help="vcpkg.json that declares the shipped dependencies")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    if not args.vanillapdf_tag:
        sys.exit("Error: --vanillapdf-tag is empty")

    shipped = shipped_ports(args.vcpkg_manifest)
    ports = collect_ports(args.input_dir, shipped)
    # A dependency without a manifest would silently drop out of the SBOM.
    missing = sorted(shipped - {name for name, _ in ports})
    if missing:
        sys.exit(f"Error: no vcpkg.spdx.json under {args.input_dir} for: {', '.join(missing)}")

    root = package("vanillapdf-cli", args.version, f"git+{args.repository}@v{args.version}",
                   "Apache-2.0", args.repository)
    library = package("vanillapdf", args.vanillapdf_tag.lstrip("v"),
                      "git+https://github.com/vanillapdf/vanillapdf", "Apache-2.0",
                      "https://github.com/vanillapdf/vanillapdf")
    dependencies = [library] + [ports[key] for key in sorted(ports)]

    document = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"vanillapdf-cli-{args.version}",
        "documentNamespace": f"{args.repository}/sbom/{args.version}/{uuid.uuid4()}",
        "creationInfo": {
            "created": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "creators": ["Tool: vanillapdf-cli-generate_sbom"],
        },
        "packages": [root] + dependencies,
        "relationships": [
            {"spdxElementId": "SPDXRef-DOCUMENT", "relationshipType": "DESCRIBES",
             "relatedSpdxElement": root["SPDXID"]},
        ] + [
            {"spdxElementId": root["SPDXID"], "relationshipType": "STATIC_LINK",
             "relatedSpdxElement": dependency["SPDXID"]}
            for dependency in dependencies
        ],
    }

    args.output.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {args.output}: {len(dependencies)} dependencies")


if __name__ == "__main__":
    main()
