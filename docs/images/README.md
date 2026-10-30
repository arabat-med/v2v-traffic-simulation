# Captures réelles de l'application

Toutes les images de ce dossier sont des rendus réels de la fenêtre Qt Quick, sans retouche des véhicules, de la carte ou des compteurs.

| Image | Contenu |
| --- | --- |
| [01-interface-qt6-qml.png](01-interface-qt6-qml.png) | Capture initiale avec fond routier OpenStreetMap, conservée |
| [02-vue-aerienne-ign.png](02-vue-aerienne-ign.png) | Sélecteur de fonds, photographies aériennes IGN et véhicules simulés |
| [03-satellite-sentinel2.png](03-satellite-sentinel2.png) | Véritable mosaïque satellite Sentinel-2 2024 fournie par EOxCloudless, et véhicules simulés |

Conditions : Ubuntu 24.04 x86_64, Qt 6.8.3, rendu logiciel `offscreen`, fenêtre 1 400 × 900, carte OSM embarquée de Bourtzwiller, 100 véhicules simulés, facteur affiché 1×. Captures réalisées le 30 octobre 2026.

Les images aériennes et satellite ne sont pas des flux en direct. Les icônes de véhicules, les cercles et les connexions viennent du moteur de simulation ; ils ne sont pas détectés dans ces fonds d'images. Les compteurs affichés ne sont pas un benchmark.

Attributions conservées dans les captures : © IGN pour l'aérien ; EOxCloudless https://cloudless.eox.at by EOX IT Services GmbH (Contains modified Copernicus Sentinel data 2024) pour le satellite ; OpenStreetMap pour la vue routière. Voir les [notices](../../THIRD_PARTY_NOTICES.md) et [les fournisseurs](../../map-providers/README.md).

## Reproduire les vues

Après la construction décrite dans le [README principal](../../README.md#installation-compilation-et-exécution), depuis la racine :

```bash
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/linux-release/appfinalproduct --vehicles 100 --basemap aerial \
  --screenshot-delay 20 --screenshot docs/images/02-vue-aerienne-ign.png

QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/linux-release/appfinalproduct --vehicles 100 --basemap satellite \
  --screenshot-delay 20 --screenshot docs/images/03-satellite-sentinel2.png
```

Le programme démarre, attend le délai demandé, capture la fenêtre et quitte. Les positions et rayons changent entre les exécutions. Le réseau et le cache conditionnent le chargement complet des tuiles : vérifier visuellement chaque capture.

## Vidéos et calibration

Le moteur Qt n'enregistre pas de vidéo ; un enregistreur d'écran externe peut montrer les contrôles et le déplacement simulé. L'extension Python produit une véritable vidéo annotée à partir d'une vidéo fournie : voir le [guide Computer Vision](../../vision/README.md).

Les vidéos de validation et leurs annotations restent locales. Les tests géographiques synthétiques ne constituent pas une démonstration de calibration sur le terrain. Les futurs écrans à capturer avec une caméra réelle sont : saisie des repères, vidéo annotée, puis lecture des trajectoires sur la carte avec la même scène géolocalisée.
