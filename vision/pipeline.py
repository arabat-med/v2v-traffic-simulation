"""Real YOLO + ByteTrack inference and incremental, versioned trajectory output."""

import hashlib
import json
import math
from pathlib import Path
import time

import cv2

from .calibration import Calibration, distance_m


def extract_frame(video, output, time_s=0.0):
    source = Path(video)
    output = Path(output)
    if not source.is_file():
        raise ValueError("Video file does not exist")
    if source.resolve() == output.resolve():
        raise ValueError("Output must not overwrite the input video")
    if not math.isfinite(time_s) or time_s < 0:
        raise ValueError("Frame time must be finite and non-negative")
    capture = cv2.VideoCapture(str(source))
    try:
        if not capture.isOpened():
            raise ValueError("OpenCV cannot open this video")
        capture.set(cv2.CAP_PROP_POS_MSEC, time_s * 1000)
        ok, frame = capture.read()
        if not ok:
            raise ValueError("No frame at the requested time")
        output.parent.mkdir(parents=True, exist_ok=True)
        if not cv2.imwrite(str(output), frame):
            raise ValueError("Cannot save the extracted image")
        return {"image": str(output), "width": frame.shape[1], "height": frame.shape[0]}
    finally:
        capture.release()


def analyze(video, output, model="yolo11n.pt", calibration=None, device="cpu", max_frames=None, confidence=0.25):
    from ultralytics import YOLO
    import ultralytics

    source, destination = Path(video), Path(output)
    if not source.is_file():
        raise ValueError("Video file does not exist")
    if max_frames is not None and max_frames < 1:
        raise ValueError("max_frames must be positive")
    if not math.isfinite(confidence) or not 0 < confidence <= 1:
        raise ValueError("confidence must be in (0,1]")
    if destination.exists() and any(destination.iterdir()):
        raise ValueError("Choose a new or empty output directory; existing results are preserved")
    camera = Calibration.load(calibration) if calibration else None
    capture = cv2.VideoCapture(str(source))
    writer = None
    handles = []
    complete = False
    try:
        if not capture.isOpened():
            raise ValueError("OpenCV cannot open this video")
        fps = capture.get(cv2.CAP_PROP_FPS)
        if not math.isfinite(fps) or fps <= 0:
            raise ValueError("A video with a valid, constant frame rate is required")
        width, height = int(capture.get(cv2.CAP_PROP_FRAME_WIDTH)), int(capture.get(cv2.CAP_PROP_FRAME_HEIGHT))
        if camera and (camera.width != width or camera.height != height):
            raise ValueError("Calibration image dimensions differ from the original video")
        detector = YOLO(model)
        vehicle_classes = [key for key, name in detector.names.items() if name in {"car", "motorcycle", "bus", "truck"}]
        if not vehicle_classes:
            raise ValueError("The model must contain car, motorcycle, bus or truck classes")
        destination.mkdir(parents=True, exist_ok=True)
        writer = cv2.VideoWriter(str(destination / "annotated.partial.mp4"), cv2.VideoWriter_fourcc(*"mp4v"), fps, (width, height))
        if not writer.isOpened():
            raise ValueError("No working MP4 encoder in this OpenCV installation")
        detections = (destination / "detections.partial.jsonl").open("w")
        handles.append(detections)
        trajectories = None
        if camera:
            trajectories = (destination / "trajectories.partial.jsonl").open("w")
            handles.append(trajectories)
            trajectories.write(json.dumps({
                "type": "v2v_trajectories", "schema_version": 1, "coordinate_system": "WGS84",
                "position_source": "camera_homography", "calibration_status": camera.status,
                "transmission_radius_m": 200, "fps": fps,
                "fit_rmse_m": camera.fit_rmse_m,
                "note": "Positions estimated from user ground references; V2V radius is simulated.",
            }) + "\n")
        detections.write(json.dumps({"type": "v2v_detections", "schema_version": 1, "fps": fps,
                                     "image_width": width, "image_height": height}) + "\n")
        previous = {}
        ids, located_ids = set(), set()
        frames, boxes_total, located_total = 0, 0, 0
        started = time.monotonic()
        while max_frames is None or frames < max_frames:
            ok, frame = capture.read()
            if not ok:
                break
            timestamp = frames / fps
            result = detector.track(frame, persist=True, tracker="bytetrack.yaml", classes=vehicle_classes,
                                    conf=confidence, device=device, verbose=False)[0]
            found, located = [], []
            if result.boxes is not None:
                for box in result.boxes:
                    bbox = [float(v) for v in box.xyxy[0].cpu().tolist()]
                    track_id = int(box.id.item()) if box.id is not None else None
                    item = {"id": track_id, "class_name": detector.names[int(box.cls.item())],
                            "confidence": float(box.conf.item()), "bbox_xyxy": bbox}
                    found.append(item)
                    if track_id is not None:
                        ids.add(track_id)
                    if camera and track_id is not None:
                        geo = camera.project((bbox[0]+bbox[2])/2, bbox[3])
                        if geo is not None:
                            speed = 0.0
                            if track_id in previous:
                                last_time, last_geo = previous[track_id]
                                if timestamp > last_time:
                                    speed = distance_m(last_geo, geo) / (timestamp-last_time)
                            previous[track_id] = (timestamp, geo)
                            located.append({"id": track_id, "lat": geo[0], "lon": geo[1], "speed_mps": speed,
                                            "confidence": item["confidence"], "class_name": item["class_name"]})
                            located_ids.add(track_id)
            # Metadata are fixed for a constant-frame-rate clip. Every decoded frame is retained,
            # including empty ones, so vehicles disappear at the correct replay timestamp.
            detections.write(json.dumps({"time_s": timestamp, "detections": found}, allow_nan=False) + "\n")
            if trajectories:
                trajectories.write(json.dumps({"time_s": timestamp, "vehicles": located}, allow_nan=False) + "\n")
            annotated = result.plot()
            if camera:
                cv2.polylines(annotated, [camera.hull.astype("int32")], True, (255, 210, 0), 2)
            cv2.putText(annotated, "VIDEO: detected vehicles | IDs: ByteTrack", (12, 28),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.65, (255,255,255), 2)
            writer.write(annotated)
            frames += 1; boxes_total += len(found); located_total += len(located)
        if frames == 0:
            raise ValueError("No video frame decoded")
        summary = {"schema_version": 1, "frames_processed": frames, "source_fps": fps,
                   "detections": boxes_total, "unique_track_ids": len(ids),
                   "georeferenced_observations": located_total, "georeferenced_ids": len(located_ids),
                   "model": Path(model).name, "tracker": "ByteTrack", "ultralytics_version": ultralytics.__version__,
                   "elapsed_seconds": time.monotonic()-started,
                   "calibration_status": camera.status if camera else "not_provided",
                   "fit_rmse_m": camera.fit_rmse_m if camera else None,
                   "warning": "Track IDs may fragment; no measured GPS accuracy or speed accuracy is claimed."}
        if calibration:
            summary["calibration_sha256"] = hashlib.sha256(Path(calibration).read_bytes()).hexdigest()
        (destination / "summary.json").write_text(json.dumps(summary, indent=2, allow_nan=False)+"\n")
        complete = True
        return summary
    finally:
        capture.release()
        if writer is not None:
            writer.release()
        for handle in handles:
            handle.close()
        if complete:
            for file in destination.glob("*.partial.*"):
                file.rename(file.with_name(file.name.replace(".partial.", ".")))
