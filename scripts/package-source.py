#!/usr/bin/env python3
"""Export the current source files without Git history, SDKs or local datasets."""

import argparse
import hashlib
import json
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parents[1]
REQUIRED = [
    "CMakeLists.txt", "README.md", "THIRD_PARTY_NOTICES.md", ".gitignore", ".gitattributes",
    "main.cpp", "Main.qml", "CalibrationDialog.qml",
    *[f"{module}.{extension}" for module in (
        "roadgraph", "node", "edge", "vehicle", "trafficsimulation",
        "roadgraphmodel", "vehiclemodel", "interferencegraph", "trajectoryreplay", "calibrationwriter",
    ) for extension in ("h", "cpp")],
    "data/demo.osm", "data/README.md", "tests/core_tests.cpp",
    "scripts/package-source.py", ".github/workflows/ci.yml",
    "docs/images/README.md", "docs/images/01-interface-qt6-qml.png",
    "tests/fixtures/synthetic-trajectories.jsonl",
    *[f"map-providers/{name}" for name in ("street", "satellite", "terrain", "cycle", "transit", "night-transit", "hiking", "README.md")],
    "vision/__init__.py", "vision/__main__.py", "vision/calibration.py", "vision/pipeline.py",
    "vision/requirements.txt", "vision/README.md", "vision/tests/test_calibration.py",
    "docs/validation/vision.json",
]
OPTIONAL = ["web/index.html", "web/styles.css", "web/vercel.json", "web/assets/interface-qt6-qml.png",
            "docs/images/02-vue-aerienne-ign.png", "docs/images/03-satellite-sentinel2.png"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "build/distribution/v2v-source.zip")
    args = parser.parse_args()
    missing = [name for name in REQUIRED if not (ROOT / name).is_file()]
    if missing:
        raise SystemExit("Missing source files: " + ", ".join(missing))
    names = REQUIRED + [name for name in OPTIONAL if (ROOT / name).is_file()]
    output = args.output.resolve()
    if output in [(ROOT / name).resolve() for name in names]:
        raise SystemExit("The archive must not overwrite a source file")
    output.parent.mkdir(parents=True, exist_ok=True)
    manifest = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in names}
    prefix = "v2v-traffic-simulation/"
    with ZipFile(output, "w", compression=ZIP_DEFLATED, compresslevel=9) as archive:
        for name in names:
            archive.write(ROOT / name, prefix + name)
        archive.writestr(prefix + "SOURCE_MANIFEST.json", json.dumps({
            "description": "Current working-tree source snapshot; excludes Git history and generated files",
            "files_sha256": manifest,
        }, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps({"archive": str(output), "files": len(names) + 1,
                      "size_bytes": output.stat().st_size,
                      "sha256": hashlib.sha256(output.read_bytes()).hexdigest()}))


if __name__ == "__main__":
    main()
