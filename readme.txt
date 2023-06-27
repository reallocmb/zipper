D'une certaine manière, le truc de la force brute ne fonctionne pas vraiment. Je ne sais pas exactement ce que c'est.
Je dirais lors de la présentation que c'est la seule manière logique d'interroger le mot de passe avec [libzip].
Donc pour utiliser la fonction zip_fopen_index_encrypted. Je suis vraiment désolé mais je n'ai pas vraiment compris ce que c'est.
Peut-être que cela a quelque chose à voir avec la norme de cryptage. Si vous le découvrez demain, je serais heureux si vous pouviez me le dire également.


Compilation du programme :

make /* le programme sera compilé et vous trouverez l'exécutable dans ./bin/zipper */

make install /* le programme sera installé maintenant vous pouvez l'exécuter dans le terminal comme ceci [zipper -o test.zip] */

make remove /* désinstalle à nouveau le programme */

Caractéristiques:

->options -h --help

exemple:
fermeture éclair - h

sortir:
-h, --help Afficher cette aide
-o, --open Ouvrir un fichier zip pour le parcourir
-b, --bruteforce Essayer de forcer brutalement le mot de passe
-d, --dictionary [FILE] Essayez de forcer brutalement le mot de passe avec un dictionnaire
-p, --password [MOT DE PASSE] Utiliser ce mot de passe
-e, --extract [FICHIER] Extraire ce fichier
-i, --include [FILE] Inclure ce fichier

-> options -o --open

exemples:
zipper -o test.zip

sortir:
Actions:
extraire le fichier ou le répertoire(e), visiter le répertoire(v), revenir en arrière(b), ouvrir le fichier(o), quitter(q)
./
1. répertoire1/
2. répertoire2/
3.fichier1
4.fichier2

exemples:
avec l'entrée e2, vous pouvez exporter dir2 le dossier complet
ou vous pouvez exporter avec l'exemple e4 le fichier2

avec v[index] vous pouvez visiter le répertoire et le contenu de celui-ci apparaît
avec b tu peux revenir en arrière

avec o vous pouvez afficher le contenu du fichier dans le terminal


-> option-b

exemples:
zipper -b pass.zip /* pass.zip est chiffré */

-> option-d

exemple:
zipper -b -d dictionary pass.zip /* parcourt le fichier du dictionnaire et essaie les mots de passe qu'il contient */

-> options -e

exemples:
zipper -e extract test.zip /* extrait l'archive zip complète dans l'emplacement du dossier d'extraction */

-> option-i
zipper -i src test.zip /* ajoute le dossier src complet dans le test.zip */
