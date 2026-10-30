# Captures réelles de l'application

## Capture disponible

[01-interface-qt6-qml.png](01-interface-qt6-qml.png) est un rendu réel de la fenêtre Qt Quick, produit le 30 octobre 2026 par l'option `--screenshot` du programme.

Conditions : Ubuntu 24.04 x86_64, Qt 6.8.3, rendu logiciel en mode `offscreen`, carte embarquée de Bourtzwiller, 100 véhicules, facteur affiché 1×. Les cercles et connexions sont activés. Le fond cartographique a été chargé pendant cette exécution. La scène montre un modèle simulé ; les compteurs ne sont pas des mesures de performance.

La capture native mesure 1 400 × 900 pixels. Les véhicules, liens et valeurs affichés n'ont pas été retouchés. L'attribution OpenStreetMap est conservée.

## Reproduire un rendu

Après la construction décrite dans le [README principal](../../README.md#installation-compilation-et-exécution), depuis la racine :

```bash
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/linux-release/appfinalproduct --vehicles 100 \
  --screenshot docs/images/01-interface-qt6-qml.png
```

Le programme démarre, attend huit secondes, enregistre le rendu réel de la fenêtre et quitte. Les positions et rayons peuvent différer d'une exécution à l'autre. Le chargement du fond cartographique dépend du réseau et du cache.

## Autres écrans et vidéo

Les vues suivantes restent à capturer manuellement après vérification :

| Emplacement prévu | Vue |
| --- | --- |
| `docs/images/02-graphe-portee.png` | Détail de liens et cercles réellement présents |
| `docs/images/03-mobilite-sans-cercles.png` | Véhicules et liens après désactivation de `RADIUS CIRCLES` |
| `docs/images/04-statistiques-reseau.png` | Compteurs visibles après un recalcul, en pause |

L'application n'enregistre pas de vidéo. Un enregistreur d'écran externe peut montrer démarrage, pause, reprise et affichage des couches. Aucun résultat vidéo de simulation ou de Computer Vision n'est actuellement fourni.
