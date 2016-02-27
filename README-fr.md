# Mantra

Une bibliothèque Entité-Composant-Système écrite en C++. Mantra est entièrement typée statiquement et se concentre sur les performances et la modularité. La bibliothèque est également construite dans l'optique du support du parallélisme. Néanmoins, cette fonctionnalité n'est pas encore implémentée.

Mantra est à un stade précoce de développement et certaines choses peuvent ne pas fonctionner correctement. N'hésitez pas à ouvrir une *issue* si vous rencontrez un problème.

## Installation

Mantra est une bibliothèque comportant uniquement des fichiers d'entête. Vous pouvez utiliser [CMake](http://www.cmake.org) pour l'installation, ou bien copier les fichiers de la bibliothèque là où vous le souhaitez. L'utilisation de Mantra dans un programme requiert le support du standard C++14, ainsi que les bibliothèques [Boost](http://www.boost.org).

## Documentation

Vous pouvez utiliser [Doxygen](http://www.stack.nl/~dimitri/doxygen/) pour générer la documentation de la bibliothèque. Une cible CMake nommée `doc` est disponible pour ceci.

## Licence

Mantra est distribué sous licence CeCILL-B (similaire à la licence MIT). Référez-vous au fichier LICENCE ou à http://www.cecill.info pour plus d'informations.
