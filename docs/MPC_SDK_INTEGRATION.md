# Intégration AKAI MPC — plan de livraison

## Machines visées

| Machine | Cible | Validation |
| --- | --- | --- |
| MPC Live III | Linux ARM64 | Compilation + test sur appareil requis |
| MPC Key 37 | Linux ARM32 | Compilation + test sur appareil requis |
| Force | Linux ARM32 | Vérifier firmware, découverte et chargement |

SDK officiel : https://github.com/inmusic-software/mpc-plugin-builder-sdk

## Contrat SDK

Le SDK utilise une bibliothèque partagée Linux compatible avec l'ABI VST 2.4, un manifeste `plugin.json` et une interface déclarative `ui.json`. Ne pas tenter de charger les binaires macOS ou Windows dans les MPC.

La chaîne officielle `tools/build.py` est prévue pour compiler les deux familles ARM. Le simulateur d'hôte et `tools/validate.py` doivent être exécutés avant les essais matériels. Suivre `GETTING_STARTED.md` et `docs/building.md` du SDK à la révision choisie.

## Intégration sans régression

1. Conserver le moteur C++ existant et ses tests de régression.
2. Créer un adaptateur d'instrument vers `framework/mpc_plugin.h` sans réécrire le DSP.
3. Faire correspondre note-on, note-off, vélocité, fréquence d'échantillonnage, buffers et événements MIDI.
4. Exposer les paramètres réels dans `plugin.json` et `ui.json` et mapper les contrôles matériels.
5. Ajouter des presets avec restauration de l'état et tests de changement de programme.
6. Générer deux paquets séparés (ARM32 et ARM64), avec manifeste, interface et bibliothèque.
7. Ajouter au CI une validation des paquets et un upload-artifact avec `if-no-files-found: error`.
8. Tester le chargement, l'audio, la polyphonie, les sauvegardes et le CPU sur MPC Live III, MPC Key 37 et Force.

## Critères de livraison

- Aucun chevauchement, texte français lisible et accents corrects sur toutes les pages.
- Pas de symboles substitués aux lettres.
- Aucun artefact présenté comme compatible avant compilation et vérification.
- Les builds Linux/macOS/Windows de démonstration ne prouvent pas la compatibilité ARM MPC.
- Pas de binaire ARM ni d'installation validés à ce stade.
