import math
import unittest

from vision.calibration import Calibration, EARTH_RADIUS


def known_plane():
    # Explicit synthetic metre-square, not a real geographic calibration.
    lat0, lon0 = 47.772, 7.31
    references = []
    for px, py, east, north in [(10,10,0,0), (110,10,10,0), (110,110,10,10), (10,110,0,10)]:
        references.append({"pixel":[px,py], "geo":[lat0+math.degrees(north/EARTH_RADIUS),
            lon0+math.degrees(east/(EARTH_RADIUS*math.cos(math.radians(lat0))))]})
    return {"schema_version":1, "image_width":120, "image_height":120,
            "reference_points":references, "calibration_status":"synthetic_test"}


class CalibrationTests(unittest.TestCase):
    def test_known_ground_plane_projects_midpoint(self):
        config = known_plane()
        camera = Calibration(config)
        point = camera.project(60,60)
        expected = [(config['reference_points'][0]['geo'][i]+config['reference_points'][2]['geo'][i])/2 for i in range(2)]
        self.assertAlmostEqual(point[0],expected[0],places=8)
        self.assertAlmostEqual(point[1],expected[1],places=8)
        self.assertLess(camera.fit_rmse_m,1e-4)

    def test_does_not_extrapolate_outside_ground_references(self):
        self.assertIsNone(Calibration(known_plane()).project(0,0))

    def test_collinear_or_missing_points_are_rejected(self):
        config = known_plane()
        config['reference_points'] = config['reference_points'][:3]
        with self.assertRaises(ValueError):Calibration(config)
        config = known_plane()
        for i,p in enumerate(config['reference_points']):p['pixel']=[10+i*20,10]
        with self.assertRaises(ValueError):Calibration(config)

    def test_inconsistent_extra_ground_reference_is_rejected(self):
        config = known_plane()
        config['reference_points'].append({'pixel':[60,60], 'geo':[47.775,7.312]})
        with self.assertRaises(ValueError):Calibration(config)

    def test_nonfinite_coordinates_are_rejected(self):
        config = known_plane()
        config['reference_points'][0]['pixel'][0] = float('nan')
        with self.assertRaises(ValueError):Calibration(config)


if __name__ == '__main__':unittest.main()
