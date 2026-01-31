# Chess
Découverte du fonctionnement des IA jouant aux échecs.
Cet outil n'est pas du machine learning, seulement un arbre dedécision basé sur une fonction  d'évaluation.

Certains outils comme XLaunch peuvent être nécessaires sur Windows.
 - g++ -O3 -march=native main.cpp board.cpp brain.cpp -o app -lsfml-graphics -lsfml-window -lsfml-system
 
Le fichier uci sert à faire jouer l'IA automatiquement contre d'autre sur le logiciel Arena.
 - x86_64-w64-mingw32-g++ -O3 uci.cpp board.cpp brain.cpp -o mon_moteur_uci.exe -static

