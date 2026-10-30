# Véhicules filmés → carte Qt

Extension locale : **YOLO11n / Ultralytics** détecte les voitures, motos, bus et camions ; **ByteTrack** conserve des identifiants entre les images ; **OpenCV** lit, annote et projette les observations. Le fichier de trajectoires est ensuite lu par l'application Qt, qui réutilise ses modèles de véhicules et son graphe de portée.

La liaison passe par un fichier JSON Lines. Elle n'est pas un flux caméra en direct et ne nécessite pas de serveur. FastAPI, Docker et MLflow ne sont pas utilisés.

## Installation vérifiée sur Linux, Python 3.12 et CPU

Depuis la racine des sources :

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install torch==2.10.0 torchvision==0.25.0 \
  --index-url https://download.pytorch.org/whl/cpu
python -m pip install -r vision/requirements.txt
python -m vision --help
```

Les roues CPU évitent l'installation de bibliothèques CUDA. Le premier traitement télécharge les poids officiels `yolo11n.pt` si aucun modèle local n'est fourni. Les vidéos, poids et résultats restent hors du dépôt. Voir les [notices des dépendances](../THIRD_PARTY_NOTICES.md), notamment la licence d'Ultralytics.

## 1. Extraire une image de sa caméra fixe

Utiliser une vidéo à fréquence d'images constante et une caméra immobile. Ne pas redimensionner ni recadrer séparément l'image servant à la calibration.

```bash
python -m vision extract-frame \
  --video /chemin/vers/video.mp4 \
  --output vision-inputs/camera.png
```

## 2. Associer l'image à la carte

Lancer l'application Qt et cliquer **Calibrer une caméra…**, puis **Ouvrir l'image de la caméra…**.

1. Choisir un point **au sol**, identifiable sur l'image : angle d'un marquage, coin d'un trottoir, etc.
2. Cliquer le même point sur la carte. Pour la France, la vue aérienne IGN permet de voir les détails du sol.
3. Répéter avec au moins quatre points non alignés, répartis autour de la zone de passage. Utiliser davantage de points si possible.
4. Enregistrer `vision-inputs/calibration.json`.

Le fichier contient la taille de l'image originale et les correspondances `pixel: [x, y]` / `geo: [latitude, longitude]`. La saisie des points n'atteste pas leur précision : le calcul et le contrôle de cohérence sont faits lors du traitement Python.

Le modèle suppose un sol approximativement plan et une caméra fixe. La distorsion optique n'est pas corrigée automatiquement. Une caméra mobile ou une route en fort relief nécessitent une autre méthode de calibration.

## 3. Détecter, suivre et géolocaliser

```bash
python -m vision analyze \
  --video /chemin/vers/video.mp4 \
  --calibration vision-inputs/calibration.json \
  --output vision-output/essai-01 \
  --device cpu
```

Choisir un dossier de sortie neuf. `--max-frames 300` permet de limiter un premier essai ; `--model /chemin/vers/yolo11n.pt` utilise des poids locaux.

| Sortie | Contenu réel |
| --- | --- |
| `annotated.mp4` | Vidéo avec boîtes, classes et identifiants de suivi ; zone calibrée tracée |
| `detections.jsonl` | Toutes les détections, coordonnées en pixels, temps et identifiants disponibles |
| `trajectories.jsonl` | Positions estimées WGS84, uniquement dans l'enveloppe des points de calibration |
| `summary.json` | Comptes observés, versions, durée du traitement et erreur d'ajustement |

Le point projeté est le milieu du bas de la boîte du véhicule, approximation de son contact au sol. Les vitesses sont des différences de positions estimées, sans filtrage ni validation métrologique. Les identifiants ByteTrack peuvent être interrompus ou changer après une occultation ; leur nombre ne constitue pas un comptage de véhicules uniques garanti.

Sans `--calibration`, le traitement produit la vidéo et les détections en pixels, **aucune trajectoire géographique**. Qt refuse ce fichier de détections à l'import.

Les sorties incomplètes portent `.partial` et ne doivent pas être importées. Les fichiers finaux sont nommés après la fin réussie du traitement.

## 4. Lire les observations dans Qt

Dans l'interface, cliquer **Importer des trajectoires vidéo…**, sélectionner `trajectories.jsonl`, puis **START**. Les boutons pause, reset et le facteur temporel s'appliquent à la lecture. **Revenir à la simulation** restaure les véhicules aléatoires.

```bash
./build/linux-release/appfinalproduct \
  --tracks vision-output/essai-01/trajectories.jsonl \
  --basemap aerial --autostart
```

La carte se centre sur les positions importées. Les véhicules observés ne sont pas déplacés aléatoirement ni contraints aux routes OSM. Leurs apparitions et disparitions suivent les images exportées, y compris les images sans détection. La lecture conserve la dernière image disponible jusqu'au timestamp suivant ; elle n'interpole pas les positions.

**Les cercles et connexions restent une simulation V2V géométrique**, avec un rayon de 200 m par défaut dans l'export. Ils ne sont pas mesurés par la caméra. Qt accepte au plus 64 Mio, 100 000 images et 2 000 observations par image dans un fichier.

## Validation et limites

```bash
python -m unittest discover -s vision/tests -v
```

Les tests de projection utilisent un plan synthétique connu. Le fichier [synthetic-trajectories.jsonl](../tests/fixtures/synthetic-trajectories.jsonl) vérifie la lecture Qt ; il est explicitement étiqueté **TEST SYNTHÉTIQUE** dans l'interface. Il ne représente aucune vidéo réelle.

Une inférence sur une vidéo de véhicules peut vérifier la détection et le suivi. Elle ne prouve pas la précision GPS d'une caméra sans repères géographiques vérifiés. Avec seulement quatre points, une faible erreur d'ajustement est attendue et ne valide pas l'exactitude sur le terrain. Celle-ci nécessite des repères indépendants et une vidéo réelle de la caméra visée.
