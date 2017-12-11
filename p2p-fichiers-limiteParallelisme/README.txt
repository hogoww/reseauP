Le serveur nécéssite une machine dédiée et se lance de la façon :
./a [num_port]
Les pairs demandent également une machine chacun, le tri étant fais par IP, et se lance :
./p [ip serveur xxx.xxx.xxx.xxx] [numport serv]

Note, Le dossier Seed dois etre présent pour que le programme pair soit utilisable, meme vide, Ainsi qu'un dossier "reception".

/******************************************************************/
Rajouter la limite permet d'éviter que le serveur n'ai plus de débit déscendant pour desservir chaque clients. Ceci prend également son sens pour se proteger d'attaques sur le serveur.
/******************************************************************/

Bugs connus.
Le refresh n'est pas parfait et n'est pas toujours complet de temps en temps.
