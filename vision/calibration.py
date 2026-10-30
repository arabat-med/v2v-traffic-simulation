"""Planar camera calibration from image pixels to a local WGS84 tangent plane."""

import json
import math
from pathlib import Path

import cv2
import numpy as np

EARTH_RADIUS = 6371000.0


class Calibration:
    def __init__(self, config):
        if config.get("schema_version") != 1:
            raise ValueError("Unsupported calibration schema_version")
        self.width = int(config["image_width"])
        self.height = int(config["image_height"])
        if self.width <= 0 or self.height <= 0:
            raise ValueError("Image dimensions must be positive")
        references = config["reference_points"]
        if len(references) < 4:
            raise ValueError("At least four non-collinear ground correspondences are required")
        pixels = np.asarray([p["pixel"] for p in references], dtype=np.float64)
        geo = np.asarray([p["geo"] for p in references], dtype=np.float64)
        if pixels.shape != (len(references), 2) or geo.shape != pixels.shape:
            raise ValueError("Each reference needs pixel [x,y] and geo [latitude,longitude]")
        if not np.isfinite(pixels).all() or not np.isfinite(geo).all():
            raise ValueError("Calibration coordinates must be finite")
        if ((pixels < 0).any() or (pixels[:, 0] >= self.width).any()
                or (pixels[:, 1] >= self.height).any()):
            raise ValueError("Reference pixels must lie inside the original image")
        if (np.abs(geo[:, 0]) > 85).any() or (np.abs(geo[:, 1]) > 180).any():
            raise ValueError("Invalid latitude / longitude")
        self.lat0, self.lon0 = geo.mean(axis=0)
        self.cos_lat0 = math.cos(math.radians(self.lat0))
        ground = np.column_stack((
            np.radians(geo[:, 1] - self.lon0) * EARTH_RADIUS * self.cos_lat0,
            np.radians(geo[:, 0] - self.lat0) * EARTH_RADIUS,
        ))
        if np.max(np.linalg.norm(ground, axis=1)) > 5000:
            raise ValueError("This local planar calibration is limited to a 5 km radius")
        if (np.linalg.matrix_rank(pixels - pixels.mean(axis=0)) < 2
                or np.linalg.matrix_rank(ground - ground.mean(axis=0)) < 2):
            raise ValueError("Calibration points must not be collinear")
        self.matrix, _ = cv2.findHomography(pixels, ground, method=0)
        if (self.matrix is None or not np.isfinite(self.matrix).all()
                or np.linalg.matrix_rank(self.matrix) != 3):
            raise ValueError("Degenerate homography; choose better ground references")
        predicted = cv2.perspectiveTransform(pixels.reshape(-1, 1, 2), self.matrix).reshape(-1, 2)
        self.fit_rmse_m = float(np.sqrt(np.mean(np.sum((predicted - ground)**2, axis=1))))
        tolerance = float(config.get("max_reprojection_error_m", 2.0))
        if not math.isfinite(tolerance) or not 0 < tolerance <= 20:
            raise ValueError("Reprojection tolerance must be in (0,20] metres")
        if self.fit_rmse_m > tolerance:
            raise ValueError(f"Inconsistent reference points: fit error {self.fit_rmse_m:.2f} m > {tolerance:.2f} m")
        self.hull = cv2.convexHull(pixels.astype(np.float32))
        self.status = config.get("calibration_status", "user_control_points")
        if self.status not in ("user_control_points", "synthetic_test"):
            raise ValueError("Unknown calibration status")

    @classmethod
    def load(cls, path):
        return cls(json.loads(Path(path).read_text()))

    def project(self, x, y):
        """Return (lat,lon) inside the calibrated hull, otherwise None. No extrapolation."""
        if not math.isfinite(x) or not math.isfinite(y):
            return None
        if cv2.pointPolygonTest(self.hull, (float(x), float(y)), False) < 0:
            return None
        homogeneous = self.matrix @ np.array([x, y, 1.0])
        if abs(homogeneous[2]) < 1e-9:
            return None
        east, north = homogeneous[:2] / homogeneous[2]
        if not np.isfinite([east, north]).all() or math.hypot(east, north) > 5000:
            return None
        return (self.lat0 + math.degrees(north / EARTH_RADIUS),
                self.lon0 + math.degrees(east / (EARTH_RADIUS * self.cos_lat0)))


def distance_m(a, b):
    lat1, lon1, lat2, lon2 = map(math.radians, (*a, *b))
    value = math.sin((lat2-lat1)/2)**2 + math.cos(lat1)*math.cos(lat2)*math.sin((lon2-lon1)/2)**2
    return 2 * EARTH_RADIUS * math.asin(math.sqrt(min(1.0, value)))
