# Carte de démonstration

`demo.osm` est un extrait routier réel autour de Bourtzwiller, à Mulhouse. Il est dérivé du fichier local `map2.osm` et embarqué dans l'application pour permettre un premier lancement sans téléchargement de données.

- 1 887 nœuds routiers et 404 chemins OSM retenus.
- Environ 229 Ko ; 3 757 arêtes orientées avec les règles du parseur actuel.
- Types conservés : routes automobiles et voies de desserte acceptées par `RoadGraph`.
- Métadonnées de comptes contributeurs, objets non routiers et tags non utilisés retirés de cet extrait.
- Extrait préparé le 30 octobre 2026 ; la date initiale de téléchargement de `map2.osm` n'est pas consignée.

## Provenance et licence des données

Données © OpenStreetMap et ses contributeurs, sous [Open Database License 1.0](https://opendatacommons.org/licenses/odbl/1-0/).

Voir [OpenStreetMap — droits d'auteur et attribution](https://www.openstreetmap.org/copyright). La licence des données est distincte de celle du code du projet.

Les fichiers d'origine `map.osm`, `map2.osm` et `map_large.osm` restent des données locales facultatives. Ils ne sont pas nécessaires à la construction et ne figurent pas dans l'archive de sources préparée pour publication. Pour charger un autre extrait :

```bash
./build/linux-release/appfinalproduct --map /chemin/vers/carte.osm
```

Le fond cartographique affiché par Qt Location est distinct de ce réseau routier local et peut nécessiter Internet.
