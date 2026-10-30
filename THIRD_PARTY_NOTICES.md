# Données et dépendances tierces

## OpenStreetMap

La carte `data/demo.osm` est dérivée du fichier local `map2.osm` pour Bourtzwiller, Mulhouse. Données © OpenStreetMap et ses contributeurs, sous Open Database License 1.0. Voir [l'attribution OpenStreetMap](https://www.openstreetmap.org/copyright) et [la licence ODbL 1.0](https://opendatacommons.org/licenses/odbl/1-0/).

Le fond cartographique Qt Location est distinct de cet extrait et conserve son attribution dans l'interface et les captures.

## Qt

La construction vérifiée utilise Qt 6.8.3, lié dynamiquement. Le dépôt de sources n'inclut pas de bibliothèques Qt, de SDK ou de paquets installés.

Les distributions binaires peuvent inclure des bibliothèques, plugins et modules QML Qt. Les notices et licences de ces composants restent distinctes du code du projet. Voir [les licences utilisées par Qt 6.8](https://doc.qt.io/qt-6.8/licenses-used-in-qt.html) et les sources de la version [Qt 6.8.3](https://download.qt.io/archive/qt/6.8/6.8.3/single/).

## Imagerie réelle : IGN et Sentinel-2

La vue aérienne utilise le service WMTS Géoplateforme de l'IGN, couche `ORTHOIMAGERY.ORTHOPHOTOS`. Les images sont des photographies aériennes archivées, distinctes du réseau OSM. Voir [BD ORTHO](https://cartes.gouv.fr/rechercher-une-donnee/dataset/IGNF_BD-ORTHO).

La vue satellite utilise **EOxCloudless https://cloudless.eox.at by EOX IT Services GmbH (Contains modified Copernicus Sentinel data 2024)**. Ce sont des images satellite Sentinel-2, pas une source vidéo en direct. L'usage personnel et académique relève des [conditions EOX](https://cloudless.eox.at/documentation/license), avec CC BY-NC-SA 4.0 pour l'usage non commercial ; l'usage commercial nécessite les droits correspondants auprès d'EOX. Les attributions restent visibles dans l'interface et les captures.

Aucun lot de tuiles ni poids de modèle n'est ajouté au dépôt. Les [fournisseurs cartographiques](map-providers/README.md) sont documentés séparément.

## Dépendances de vision

Le module optionnel Python dépend d'Ultralytics YOLO, du suivi ByteTrack fourni par Ultralytics, d'OpenCV, de NumPy et de PyTorch / Torchvision. Voir [Ultralytics — licences](https://www.ultralytics.com/license), [OpenCV — licence](https://opencv.org/license/) et [PyTorch — licence](https://github.com/pytorch/pytorch/blob/main/LICENSE).

Ultralytics propose son code sous AGPL-3.0 ou licence Enterprise. Cette dépendance et les poids téléchargés ont leurs propres conditions de distribution et d'utilisation ; aucune licence Enterprise n'est incluse ou revendiquée. Le dépôt ne contient pas ces bibliothèques installées ni ces poids.

Les extraits vidéo de validation proviennent des [assets officiels Ultralytics](https://github.com/ultralytics/assets/releases/tag/v0.0.0) et restent des données locales de test, sans être redistribués dans le dépôt.

## Code du projet

Aucune licence de réutilisation du code du projet n'a encore été choisie. L'accès aux sources ne constitue pas une attribution automatique de licence MIT, GPL ou autre. Les licences des données et dépendances tierces restent applicables à leurs composants respectifs.
