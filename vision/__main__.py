"""Run with python -m vision extract-frame ... or python -m vision analyze ..."""

import argparse
import json
import sys


def main():
    parser = argparse.ArgumentParser(description="Vehicle video → YOLO / ByteTrack → calibrated Qt trajectories")
    sub = parser.add_subparsers(dest="command", required=True)
    frame = sub.add_parser("extract-frame", help="Extract an original-resolution image for camera calibration")
    frame.add_argument("--video", required=True)
    frame.add_argument("--output", required=True)
    frame.add_argument("--time", type=float, default=0.0)
    analyze = sub.add_parser("analyze", help="Detect and track vehicles; optionally project them to WGS84")
    analyze.add_argument("--video", required=True)
    analyze.add_argument("--output", required=True)
    analyze.add_argument("--model", default="yolo11n.pt", help="Official model name (downloaded on first use) or trusted local weights")
    analyze.add_argument("--calibration", help="Ground references saved by the Qt calibration tool")
    analyze.add_argument("--device", default="cpu")
    analyze.add_argument("--max-frames", type=int)
    analyze.add_argument("--confidence", type=float, default=0.25)
    args = parser.parse_args()
    try:
        from .pipeline import analyze as run_analysis, extract_frame
        if args.command == "extract-frame":
            result = extract_frame(args.video, args.output, args.time)
        else:
            result = run_analysis(args.video, args.output, args.model, args.calibration,
                                  args.device, args.max_frames, args.confidence)
        print(json.dumps(result, indent=2))
        return 0
    except (ValueError, OSError, ImportError, KeyError, TypeError, RuntimeError) as error:
        print(f"Vision error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
