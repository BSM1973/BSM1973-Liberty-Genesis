# LIBERTY GENESIS

Instrument virtuel en développement par Liberty Music Software, destiné à explorer les possibilités des plateformes Akai MPC 3, MPC Key 37, MPC Live III et Force.

## Objectif

Concevoir un instrument musical original, avec un moteur sonore performant, une interface adaptée aux écrans tactiles et un navigateur de presets.

## Feuille de route

1. Vérifier les SDK, formats de plugins et contraintes de déploiement réellement pris en charge par chaque appareil.
2. Définir l'architecture du moteur audio, des voix et des modulations.
3. Réaliser un premier prototype autonome testable.
4. Développer l'interface et la gestion des presets.
5. Tester la compatibilité sur les appareils cibles avant toute annonce de prise en charge.

## Cibles matérielles

- MPC Live III : cible ARM 64 bits, à valider avec le SDK officiel et sur le matériel.
- MPC Key 37 : cible ARM 32 bits, à valider avec le SDK officiel et sur le matériel.
- Force : cible ARM 32 bits, à valider séparément (OS et chargeur de plugins).

Le moteur DSP C++ reste commun aux plateformes. Les formats de plugins, interfaces hôtes, dépendances SDK et paquets d'installation sont spécifiques aux cibles. Les builds CI actuels (macOS, Windows, Linux de bureau) ne sont pas des binaires MPC. Aucun support natif ne doit être annoncé avant test sur appareil.

## État

Phase de conception. Aucun binaire compatible MPC ou Force n'est encore validé.

© Liberty Music Software.
