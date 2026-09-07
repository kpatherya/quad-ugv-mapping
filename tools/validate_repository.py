#!/usr/bin/env python3
"""Validate repository structure without requiring a ROS installation."""

from __future__ import annotations

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROJECT_DOCS = [
    ROOT / "README.md",
    *sorted((ROOT / "docs").glob("*.md")),
    ROOT / "src" / "integrations" / "README.md",
    ROOT / "src" / "toolbox" / "README.md",
    ROOT / "src" / "vendor" / "README.md",
]


def report_error(errors: list[str], message: str) -> None:
    errors.append(message)
    print(f"ERROR: {message}")


def validate_structure(errors: list[str]) -> None:
    required = [
        "README.md",
        "LICENSE",
        "CITATION.cff",
        "docs/ARCHITECTURE.md",
        "docs/PROVENANCE.md",
        "docs/REPRODUCIBILITY.md",
        "src/integrations/vins_mono_ros2",
        "src/toolbox/point_cloud_converter",
        "src/vendor",
    ]
    for relative_path in required:
        if not (ROOT / relative_path).exists():
            report_error(errors, f"missing required path: {relative_path}")


def validate_packages(errors: list[str]) -> None:
    manifests = sorted((ROOT / "src").rglob("package.xml"))
    if not manifests:
        report_error(errors, "no ROS package manifests found")
        return

    names: dict[str, Path] = {}
    for manifest in manifests:
        try:
            package = ET.parse(manifest).getroot()
        except ET.ParseError as exc:
            report_error(errors, f"invalid XML in {manifest.relative_to(ROOT)}: {exc}")
            continue

        name = package.findtext("name", "").strip()
        if not name:
            report_error(errors, f"missing package name: {manifest.relative_to(ROOT)}")
        elif name in names:
            report_error(
                errors,
                f"duplicate package name {name}: {names[name].relative_to(ROOT)} and "
                f"{manifest.relative_to(ROOT)}",
            )
        else:
            names[name] = manifest

        license_value = package.findtext("license", "").strip()
        if not license_value or "TODO" in license_value.upper():
            print(f"WARNING: unresolved package license: {manifest.relative_to(ROOT)}")

    print(f"OK: parsed {len(manifests)} unique ROS package manifests")


def validate_local_links(errors: list[str]) -> None:
    markdown_link = re.compile(r"\[[^]]+\]\(([^)]+)\)")
    checked = 0
    for document in PROJECT_DOCS:
        if not document.exists():
            continue
        for raw_target in markdown_link.findall(document.read_text(encoding="utf-8")):
            target = raw_target.split("#", 1)[0]
            if not target or "://" in target or target.startswith("mailto:"):
                continue
            checked += 1
            resolved = (document.parent / target).resolve()
            if not resolved.exists():
                report_error(
                    errors,
                    f"broken link in {document.relative_to(ROOT)}: {raw_target}",
                )
    print(f"OK: checked {checked} local documentation links")


def validate_mapping_contract(errors: list[str]) -> None:
    launch_file = (
        ROOT
        / "src/integrations/vins_mono_ros2/vins_estimator/launch/euroc.launch.py"
    )
    source = launch_file.read_text(encoding="utf-8")
    required_fragments = [
        "('pointcloud_out', '/depth_pointcloud')",
        "('cloud_in', '/depth_pointcloud')",
    ]
    for fragment in required_fragments:
        if fragment not in source:
            report_error(errors, f"mapping topic contract missing: {fragment}")
    if all(fragment in source for fragment in required_fragments):
        print("OK: depth converter and OctoMap topic contract is connected")


def main() -> int:
    errors: list[str] = []
    validate_structure(errors)
    validate_packages(errors)
    validate_local_links(errors)
    validate_mapping_contract(errors)
    if errors:
        print(f"FAILED: {len(errors)} repository validation error(s)")
        return 1
    print("PASS: repository structure and static contracts are valid")
    return 0


if __name__ == "__main__":
    sys.exit(main())