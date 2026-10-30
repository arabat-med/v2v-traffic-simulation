# Fonds cartographiques et imagerie

Ces fichiers JSON sont embarqués dans les ressources Qt et utilisent le [format de fournisseur du plugin OSM de Qt 6.8](https://doc.qt.io/qt-6.8/location-plugin-osm.html). Les tuiles sont téléchargées à l'exécution ; aucune collection d'images satellite n'est embarquée.

| Choix de l'interface | Source | Couverture / résolution |
| --- | --- | --- |
| Carte routière | OpenStreetMap | Cartographie routière, zoom maximal 19 |
| Vue aérienne | IGN Géoplateforme, `ORTHOIMAGERY.ORTHOPHOTOS` | Photographies aériennes de la France, zoom maximal 19 |
| Satellite | EOxCloudless, `s2cloudless-2024_3857` | Mosaïque mondiale Sentinel-2 de 2024, résolution source 10 m, zoom limité à 14 |

Le fichier `terrain` du plugin fournit ici l'orthophotographie IGN ; le sélecteur de l'application l'identifie comme **Vue aérienne**, sans prétendre montrer du relief.

La date des photographies IGN dépend des zones et n'est pas déterminée par l'application. La mosaïque Sentinel-2 est une archive, pas une image en direct ; elle ne permet pas de voir les véhicules en temps réel. Les véhicules affichés viennent du moteur de simulation ou d'un fichier d'observations vidéo.

Les types `cycle`, `transit`, `night-transit` et `hiking` sont désactivés et ne figurent pas dans le sélecteur. Le préchargement des niveaux voisins est désactivé.

Attributions visibles dans la carte :

- Données routières © OpenStreetMap contributors, [ODbL](https://www.openstreetmap.org/copyright). Respecter la [politique des tuiles OSM](https://operations.osmfoundation.org/policies/tiles/).
- Photographies aériennes © IGN, [BD ORTHO](https://cartes.gouv.fr/rechercher-une-donnee/dataset/IGNF_BD-ORTHO), Licence Ouverte pour les données publiques concernées.
- **EOxCloudless https://cloudless.eox.at by EOX IT Services GmbH (Contains modified Copernicus Sentinel data 2024)**. Usage personnel et académique selon [les conditions et la licence EOX](https://cloudless.eox.at/documentation/license), qui prévoient CC BY-NC-SA 4.0 pour l'usage non commercial. Une utilisation commerciale nécessite les droits correspondants auprès du fournisseur.

La disponibilité dépend du réseau et des services tiers. Les métadonnées déclarent le fond demandé ; un test de démarrage Qt ne garantit pas que toutes ses tuiles ont été reçues. Les captures documentées sont contrôlées visuellement séparément.
