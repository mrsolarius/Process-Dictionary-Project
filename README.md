# Rapport Process Dictonnary

* Introduction
* Conception
* Réalisation
* Conclusion

## Introduction

Le but de ce projet est de réaliser une application en ligne de commande permettant de stocker et de consulter un dictionnaire de données. Le dictionnaire permettra de stocker des données sous forme d'une chaine de caractère accessible depuis une clef représentée par un entier naturel. La particularité de ce projet vient de la nécessité de stoker ces données dans plusieurs processus selon la valeur de la clef.

Le programme comportera 4 fonctions destinées à l'utilisateur :

* Set : Permet de définir une valeur à un emplacement donnée
* Lookup : Permet de lire la valeur contenue à la clef indiquer
* Dump : Permet de lire toutes les valeurs contenues dans tous les processus
* Exit : Permet de quitter le programme

Les processus doivent être capables de communiquer par tube entre eux, afin de pouvoir se transférer les bonnes informations jusqu'à ce que l'un d'entre eux soit en capacité de la traiter et pour finalement la transférer au processus père (le contrôleur).

## Conception

Afin de répondre au besoin attendu par le cahier de charge (le sujet du TP), nous avons imaginé une structure applicative et de donnée permettant la communication entre les différents processus.

### Étude de la structure des données nécessaire à la transmission

Nous sommes partis du constat que les pipes de données transfèrent des tableaux d'octets comme le font les sockets TCP/IP (Transmission Control Protocol). Nous avons donc procédé de la même marnière que lorsque l'on souhaite créer un protocole applicatif utilisant les sockets.

Cela veut dire que nous organiserons le flux de communication en trame prédéfini nous permettant de structurer nos données de façon, à savoir quel comportement devra adopter chacun des processus.

Nous avons tout d'abord tenté d'identifier les flux de données qui passent par chacun des pipes. De cette façon nous en somme arrivés au constat suivant :

* Le contrôleur pose ses questions au premier de ses fils
* Les processus fils communiquent entre eux et ceux jusqu'à ce qu'ils soient en mesure de répondre.
* Lorsqu’ils peuvent répondre ils envoient leur réponse au contrôleur.

Ce constat implique deux types de trames :

* Une trame de question (Ask)
* Une trame de réponse ou d'acquittement (Acquittal)

Pour être en mesure de savoir quoi mettre dans ces trames, nous devons nous demander quelle information transite dans chacune d'entres elles.

Certaines données sont constantes et non, donc pas besoin d'être transmises comme le nombre total de nœud et l'identifiant du nœud courant.

#### Trames de questions

Les trames de questions sont envoyées par le contrôleur au premier nœud puis sont retransmises au nœud suivant tant que le nœud courant ne peut pas traiter la question ou que tous les nœuds ont reçu la demande

Pour la fonction **set** nous avons deux informations envoyées :

* La commande, ici set
* L'emplacement de stockage

Pour la fonction **lookup** nous avons aussi deux informations :

* La commande, ici lookup
* L'emplacement de récupération de la valeur

Pour la commande **dump** nous avons une seule information :

* La commande, ici dump

Pour la commande **exit** nous avons une seule information :

* La commande, ici exit

#### Tram d'acquittement

Les trames d'acquittement sont transmises par les nœuds vers le contrôleur pour confirmer une exécution de commande (que cela soit un succès ou qu'il y ait eu une erreur dans le traitement)

Lorsque la fonction **set** a été exécuté avec succès nous avons besoin de 3 informations :

* La commande, ici set
* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)

Lorsque la fonction **lookup** a été exécuté avec succès nous avons besoin de 5 informations pour la traiter :

* La commande, ici lookup
* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)
* La longueur de la chaine de caractère renvoyée
* La chaine de caractère trouver

Lorsque la fonction **dump** a été exécuté avec succès nous avons besoin de 3 informations :

* La commande, ici dump
* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)

Lorsque la fonction **exit** a été exécuté, cela veut dire que le processus est mort, donc nous n'utiliserons pas de trame pour l'acquittement. Cependant, il nous faudra attendre le signal de mort des processus grâce à la fonction wait null.

### Création du Protocole DDP (Data Dictionary Protocole)

Nous avons identifié lors de la phase d'analyse que nous arions besoin de 2 trame une trame de question, une trame d'acquittement. Afin de répondre physique au besoin des données transférées qui sont multiples nous avons généralisé les attribuer (cela veut dire que les attribue non obligatoire à la transmission de certaines trames on était rapatrié)

#### Structure des trames

Ici, les trame sont montrées sous forme de tableau, on peut lire la longueur de celle ci en nombre de bits et nombre d'octets (bits). Il Faut considérer que l'échelles incrémente de la taille d'une ligne à chaque nouvelle ligne.

##### Trame de question (ASK)

```
Ask Frame
0                 1                 2                 3 (octer)
 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8  (bit)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       CMD       |        DATA (Unsigned Int)        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|    END FRAME    |
+-+-+-+-+-+-+-+-+-+
```

##### Trame d'acquittement (ACQUITTAL)

```
0                 1                 2                 3 (octer)
 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8  (bit)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       CMD       |     NODE ID     |    ERROR FLAG   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           DATA LENGTH             |  DATA (char *)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    END FRAME    |
+-+-+-+-+-+-+-+-+-+

/!\ DATA est de taille indeterminé char c'est un tableau de caractere
```

#### Composition des trames

##### ASK trame

Cette trame sera utilisée lorsqu’une donnée est envoyée depuis le contrôleur vers le processus 0

Ensuite en fonction de son contenue le processus 0 passera la trame au processus suivant ou exécutera un acquittement

S’il est en mesure d'y répondre lui-même

##### ACQUITAL trame

Cette trame sera utiliser par les processus pour renvoyer des données au contrôleur pour indiquer une fin de traitement ou une erreur d'exécution.

#### Détail des données transmises

##### Les commandes (CMD)

La commande indique ce que le processus devra faire

###### Note

Le numéro à la fin de la valeur du flag indique la trame dans laquelle la commande peut être transmise.

* → 1 Correspond à la trame ASK
* → 2 Correspond à la trame ACQUITTAL

Cela simplifiera l'identification du type de trame dans le programme

###### Exemple

0xA1 pourra être transmis uniquement par la trame ASK

0xD2 pourra être transmis uniquement par la trame ACQUITTAL

###### Valeur des commandes

* 0xA1 : Commande SET : Demande l'emplacement d'une donnée dans le processus à la clef indiquée dans DATA
* 0xA2 : Acquittement SET : Indique au contrôleur que le set à bien été traité
* 0xB1 : Commande LOOKUP : Demande la récupération d'une donnée dans les processus
* 0xB2 : Acquittement LOOKUP : Indique au contrôleur que le lookup à bien été traité
* 0xD1 : Commande DUMP : Demande l'affichage des données stoker dans le processus
* 0xD2 : Acquittement DUMP : Indique au contrôleur que le DUMP à bien été traité
* 0xE1 : Commande EXIT : Demande au processus de se suicider

##### Nous sommes très fiers d'avoir réussi à rendre ce projet fonctionnel.Identifiant du node (NODE_ID)

Le node id correspond à l'identifiant du processus, il est contenu entre 0 et 255 (il n'est pas possible d'avoirs plus de 255 fils pour utiliser ce protocole)

Les processus fils ont forcément un id qui se suivent et le premier nœud à obligatoirement l'id 0

Cet id permet au contrôleur de savoir qui a récupéré l'information qu'il a demandé parmi ses fils

sur les actions globales comme dump, cela permet aussi au contrôleur d'attendre que tous les processus aient fini le traitement avant d'en commencer un nouveau.

###### Exemple

```
0 1 2 3 4 5 6 OK
1 2 3 4 5 6 7 FAUX
0 1 3 4 5 6 7 FAUX
```

##### Drapeau d'erreur (ERROR FLAG)

Les drapeaux d'erreurs permettent d'indiquer au contrôleur que quelle que chose ne s’est pas bien passé ou au contraire si tout a bien fonctionné.

* 0x20 : Aucune erreur lors de l'exécution du processus fils
* 0x44 : Aucune donnée trouver
* 0x50 : Une erreur, c’est produit lors de l'exécution du processus

##### Données (DATA)

Data est utilisée dans nos deux frame ASK et ACQUITTAL pour transmettre des données. Cependant, il n'est pas utiliser de la même façon dans les deux cas.

###### ASK Frame

Dans cette trame DATA correspond à un unsigned short qui correspond à la clé qui doit être enregistré.

Note : DATA n'est pas obligatoire pour les commandes EXIT, et DUMP.

###### ACQUITTAL Frame

DATA LENGHT : Ici DATA LENGTH correspond à la taille de la donnée contenue dans DATA.

DATA : Ici DATA correspond à une donnée sous forme d'un tableau de char

Note : DATA LENGHT et DATA ne sont pas obligatoire pour les trames SET et DUMP

##### Fin de trame (END FRAME)

La END FRAME permet de savoir quand es que le flux de donné se termine.

* 0x04 : END FRAME : Cette valeur est connue et sera toujours la même pour terminer la transition de données.

Elle correspond au caractère ASCII end of transmission (EOF)

### Découpage des entités et en fonctionnalité et définition des rôles

Dans ce projet, nous avons décidé de découper le plus possible notre code en fonctionnalité dans le but de le rendre le plus lisible possible. De cette façon, nous avons d'abord tenté d'identifier les grandes entités du code pour les séparer dans plusieurs fichiers c diffèrent.

#### Le contrôleur

Le contrôleur est une de nos entités il est le chef d'orchestre de notre application, car il permet à la fois l'initialisation le lancement des fils et la gestion du cycle de vie de l'application. Il permet aussi l'interface avec l'utilisateur.

Le contrôleur se découpe en plusieurs fonctionnalités :

* L'initialisation et libération des pipes
* Le lancement des nœuds enfants
* La gestion de l'interface utilisateur
* La gestion de l'envoi et de la réception des trames

##### Initialisation et libération des pipes

Au début du programme lors de l'initialisation, afin de permettre la communication entre les différents éléments de l'application notre contrôleur est chargé d'initialiser les connexions, cela implique de générer le bon nombre de pipes et fermer toutes ceux qui ne lui sont pas utiles.

De la même façon avant la fermeture de l'application le contrôleur est chargé de fermer tous les pipes, même ceux qui lui ont un jour été utile. Il doit aussi les libérer de la mémoire allouée à ces tableaux.

##### Lancement et fermeture des nœuds enfants

Lors de l'initialisation du programme le contrôleur est chargé de générer le nombre de processus fils indiqué par l'utilisateur. Ces processus ont besoin de plusieurs informations pour correctement être exécuté.

Ils ont besoin :

* d'un identifiant unique
* du nombre total de nœud présent dans le réseau de processus
* d'un pipe permettant l'écriture vers le contrôleur
* d'un pipe permettant la lecture depuis le frère précédent
* d'un pipe permettant l'écriture vers le frère suivant

Ces informations doivent donc être correctement reneigées par le contrôleur lors de l'initialisation.

L'ors de la fermeture du programme, le contrôleur doit correctement fermer les nœuds enfants pour ça il nous suffit d'attendre leur sucide avec la commande wait() executer dans une boucle for auttend de fois qu'il y a de noeud créer.

##### Gestion de l'interface utilisateur

Le contrôleur est chargé d'une partie de l'interface homme machine. Il se doit de permettre la sélection de commande. La sélection des clés du dictionnaire. Il se doit aussi de gérer l'affichage des résultats en provenance des fils, ainsi que des éventuelles erreurs renvoyées.

Cette interface se présente sous la forme d'une boucle while attendant la sélection de la commande de fermeture par l'utilisateur.

##### Gestion de l'envoi et de la réception des trames

###### Gestion de l'envoi des trames

Le contrôleur à pour mission de gérer l'envoie de trame de question (ASK) au premier fils afin que celui-là la propage à ces frères. Ces envois se font en fonction des demandes de l'utilisateur. Les trames sont générées via des structures de type AskFrame et encoder sous forme de tableau de unsigned char via notre entité DDP.c.

###### Gestion de la réception des trames

Le contrôleur s'attend après chaque trame envoyée à recevoir une trame d'acquittement de la part d’au moins un de ces fils. Cette trame doit être lue puis décoder avant d'être traité.

Le décodage d'une trame d'acquittement se fait via notre entité DPP.c cela nous renverra une structure de type AcquitalFrame.

Cependant, comme lors d'un dump le contrôleur s'attend à recevoir plusieurs trames d'acquittement et que ces trames peuvent être concaténées dans le même buffer de lecture. Autrement dit, le contrôleur peut lire dans son pipe le contenu de plusieurs trame en même temps.

En conséquent : le contrôleur sera aussi chargé de séparer les trames afin de correctement les comptées et les convertir en structures.

Le comptage de ces trames sera traduit par une variable global incrémenté à chaque réception de trame et réinitialisé à chaque fin d'interaction avec l'utilisateur.

#### Le node

Le node et un processus enfant, son rôle est de gérer le stockage et la transmition des données ainsi qu'une petite partie interface utisateur. Le node à besoin des données en provenance de son frère précédent pour lancer un traitement. Sa boucle principale restera active tant qu’il n'aura pas reçu de commande d'exit.

Le node se découpe en plusieurs fonctionnalités :

* La réception et l'envoie de trames
* La gestion de l'interface home machine
* Il stocke les données

##### Déroulement du programe du node

Le point d'entrée du node et la lecture de la trame transmise par son précédent frère, cettre trame est traitée par notre entité DDP.c qui se charge de la convertir en une structure de AskFrame. Cette trame est traitée différemment en fonction de la commande envoyée.

###### Si on reçoit un SET

Si la clef envoyée par la trame correspond à une cles traitable par le node (si clef%totalnode = idnode) Dans se cas on peut traiter la commande sinon on envoie la trame reçue au frère suivant

Si on peut traiter la valeur, on va demander à l'utilisateur de renseigner une chaine de caractère.

On va ensuite enregistrer cette chaine de charachter avec la clef reçue dans la trame.

Une fois fait il nous reste plus qu'à envoyer au contrôleur une trame d'aquitement.

###### Si on reçoit un LOOKUP

Si la clef envoyée par la trame correspond à une cles traitable par le node (si clef%totalnode = idnode) Dans se cas on peut traiter la commande sinon on envoie la trame reçue au frère suivant

Si on peut traiter la valeur on va tenter de la récupérer dans la BDD.

Si la valeur est null on renvera une trame d'acquittement contenant l'erreur 44 NOT FOUND au controlleur

Si la valeur est correctement trouvée on envoie une trame d'aquitement contenant cette valeur au controleur.

###### Si on reçoit un DUMP

On affiche le contenu de notre BDD

Si notre identifiant est déférent du nombre de noeud moins un, dans ce cas on envoi la trame au prochain noeud.

On renvoie une trame d'aquitement au contrôleur.

###### Si on reçoit un EXIT

On envoie la trame reçu au nœud suivant

On quitte la boucle principale

On quitte le programme enfant avec un exit(0). Cela veut dire que le fils se suicide.

#### Librairie DDP

DDP et une entité qui s'utilise comme une librairie elle a comme rôle d'encoder et de décoder les trames du protocole DDP. Elle utilise le même système de variable global pour les erreurs que errno ce qui simplifie le débogage lors de son utilisation.

Elle définit des structures permettent une utilisation simplifiée du protocole. Il y a la structure AskFrame pour les trames de questions, et la structure Acquittal frame pour les trames d'acquittements

Les constantes des commandes et des erreurs y sont stockés sous forme d'énumération.

Afin d'éviter les erreurs,et comme notre contrôleur et nos fils s'appuieront dessus. Cette librairie est critique pour le fonctionnement correct de notre application. Pour éviter le plus possible les erreurs, nous mettrons en place des tests unitaire pour vérifier que le résultat correspond bien à celui de la spécification attendue définie plus tôt.

## Réalisation

Dans cette partie, nous détaillerons un peu plus notre démarche lors de la création du code. Ainsi que les éventuels problèmes auquel nous avons été confrontés.

### Structuration du projet

Notre projet et découper dans 3 dossiers chaque dossier représente une partie du TP :

* project_0 : Création de N fils
* project_1 : Création de N fils puis mise en place d'un cycle de communication par tube
* test_table : Test des fonctions de stockage fourmis dans se tp
* process_dictionary : Le projet complet

Chacun de ces dossiers contient un Makefile sous la forme suivante :

```makefile
CC = gcc //On indique le compilateur
SRCS = project_0.c //on indique toutes les sources.c
OBJS = $(SRCS:.c=.o) //on les convertie en .o
EXEC=projet_0 //Fichier de sortie

all: run

run:$(EXEC)
   ./$(EXEC) 5

$(EXEC): $(OBJS)
   $(CC) $(OBJS) -o $(EXEC)

clean:
   @echo "suppression de fichier temporaires et des builds"
   rm *.o
   rm $(EXEC)
```

Ce Makefile est le même de partout sauf sur la variable srcs et exec. Sur le projet process dictionary il y a dans le Makefile deux commandes en plus pour les tests unitaire `test_DDP`et `test_controler`

### Problèmes rencontrés 

Lors de la réalisation de ce projet, nous avons été confrontés à de nombreuse erreur et problème en tous genres. Ici, nous détaillerons certain des compliqués à résoudre.

#### Boucle infinie dans les fils et blocage processus

Au début du projet, nous avions très souvent à faire à des boucles infies qui restaient bloquer dans le cercle de fils. Cela était du au fait que les processus fils continuaient à envoyer à leurs frères sans condition. Plusieurs problèmes nous ramenaient à cette erreur.

##### Erreur de modulo

Lorsque l'on envoie une commande de set ou de lookup les nœuds ne s'identifiait pas toujours comme étant traitable par leur soin, car le test de l'id du nœud était faux. Il se passait donc la trame de frère en frère sans jamais s'arrêter. 

Cela venait du test qui permet de savoir si le nœud peut traiter la trame, le voir ci après:

```c
if(totalNod%askFrame->val == nodeID){...}
```

Le souci, était que l'on faisait un test en regardant le nombre total de node modulo la valeur reçu or cela ne renvoyait presque jamais une valeur identifiant l'un de nos nodes.

La solution à ce problème était donc juste d'inverser les valeur autour du modulo comme ceci : 

```c
if(askFrame->val%totalNode == nodeID){...}
```

##### Erreur de test

Lorsque l'on envoie une commande de dump au fils le dump s'exécutait de manière indéfinie car la limitation de quand les nodes devaient arrêter de retransmettre la trame à leur frère était erroné. En effet, ici le test était le suivant pour retransmettre la trame : 

```c
if(askFrame->val != totalNode){
    sendNextNode(frame,pipeWrite);
}
```

Or une ask frame ne contient pas dans sa valeur le nombre totale de noeud mais plutôt une valeur indéterminée qui traine sur la mémoire. Lorsque cette valeur ne corespond à aucun id de node alors on entrait dans une boucle infinie. Pour corriger ce probleme il sufit de tester que l'id du noeud courant soit différent du nombre total de nœud -1, voici à quoi ressemble le correctif  : 

```c
if(totalNode-1 != nodeID){
    sendNextNode(frame,pipeWrite);
}
```

##### Erreur de blocage du processus

Lors du lancement des nœuds où nous ne fermions pas les bons pipes ce qui avait pour conséquence de bloquer le contrôleur, car il attendait une réponse dans son read. Le programme restait donc bloqué après l'envoie de n'importe quelle commande. Voici le code qui fermait les mauvaises pipes :

```c
void closePipes(int * pipeCtr, int * pipeRead, int * pipeWrite){
    //Avant de commencer notre traitement on ferme les lecture/ecriture qui ne nous serve pas sur nos différent pipes
    //On ferme en lecture le pipe du controller
    close(pipeCtr[1]);
    //On ferme en ecriture le pipe de lecture
    close(pipeRead[0]);
    //On ferme en lecture le pipe d'écriture
    close(pipeWrite[1]);
}
```

Ici, nous avions inversé les numéros de lecture et d'écriture des pipes. Le correctif de cette erreur était donc tout simplement de mettre des 0 à la place de 1 et des 1 à la place des 0.

```c
void closePipes(int * pipeCtr, int * pipeRead, int * pipeWrite){
    //Avant de commencer notre traitement on ferme les lecture/ecriture qui ne nous serve pas sur nos différent pipes
    //On ferme en lecture le pipe du controller
    close(pipeCtr[0]);
    //On ferme en ecriture le pipe de lecture
    close(pipeRead[1]);
    //On ferme en lecture le pipe d'écriture
    close(pipeWrite[0]);
}
```

#### Mauvais compte d'acquittement pour les dump

Lors de nos premiers tests de dump nous étions étonnée de l'écart entre le nombre d'acquittements compter et le nombre total de nœud. Le nombre d'acquittements était bien inférieur au nombre total de noeud. C'était particulièrement visible sur un grand nombre de nœuds entre 120 et 2048. 

Cela avait pour effet de bloquer notre programme. Parce que pour attendre l'acquittement, nous parcourons une boucle while tant que le nombre de trames reçu est inférieur au nombre total de nœud.

```c
while(countAcquittal<nbNodes){
   readAcquittal(pipeCtrlRead,nbNodes);
}
```

Après un affichage de la trame entrante dans le readAcquittal() nous nous sommes rendu compte que le buffer lu sur le pipe renvoyait plusieurs trames concomitantes. Or notre fonction decodeAquittalFrame() n'est pas en mesure d'identifier plusieurs trames sur le même tableau de caractère. Comme cela entre dans le cadre du traitement de lecture du contrôleur nous avons décidé de créer une nouvelle fonction deserialization() ayant pour but de sepérarer les trames afin de les compter correctement.

Dans ce but nous avons itéré sur le buffer afin de le décalé petit à petit pour découper la trame grace à la fonction strncpy() qui copy un string dans un autre avec une longueure definis, ici on utilise le l'addition de pointeur pour générer un décalage, et donc copier une partie tronquée du tableau

par exemple pour la chaine suivante qui contient deux trames d'aquittement: `{0xd2,0xe8,0x20,0x04,0xd2,0xe9,0x20,0x04}`

Nous allons nous déplacer de la façon suivante : 

* 1 : str:{0xd2} start:0 len:1 
* 2 : str:{0xd2,0xe8} start:0 len:2 
* [...] 
* 4 : str:{0xd2,0xe8,0x20,0x04} start:0 len:4  Ici la fonction decode acquittal ne renverra pas d'erreur car str est une trame correcte start sera egale à start + len donc on aura : 
* 5 : str:{0xd2} start:4 len:1
* 6 : str:{0xd2,0xe9} start:4 len:2 

Ce qui permet de compter la trame suivante :

```c
for (int i = 0, len = 1, start = 0; i < bufferSize; ++i, ++len) {
   unsigned char *str;
   //On alloue de la memoire au string de travail
   str = malloc(sizeof(unsigned char)*len+1);
   strncpy((char *)str, (char *)frame + start, len);
   acquittalFrame = decodeAcquittalFrame(str,len);
   if (acquittalFrame->cmd != 0xff) {
       // ici si j'avais eu le temps j'aurais préféré stoker toute les structure d'acquittement dans une
       // liste chainer ou un tableau pour éventuellement afin de les renvoyer et de les traiter ailleurs plus tard
       //
       // en attendant je copie la structure trouver dans la structure final renvoyer par la fonction
       // la copie est obligatoire car acquittalFrame et free() à chaque itération
       memcpy(acquittalFinal, acquittalFrame, sizeof(AcquittalFrame));
       start += len;
       len = 1;
       //On incrémente la variable global qui permet de savoir combien de trame on était lu depuis le dernier rest
       countAcquittal++;
   }
   //On liber toutes nos valeur
   free(str);
   //surtous le aquitalframe pour eviter d'avoir des information résiduel à la prochaine itération
   free(acquittalFrame);
}
```

De cette façon, nous sommes en mesure de correctement compter les trames. Le souci, c’est que maintenant il nous arrivait de compter plusieurs fois la même node lorsque beaucoup de processus étaient lancé simultanément.

On a d’abord pensé que cela venait du faite que les nodes s'étaient mit à envoyer plusieurs fois les mêmes trames, mais après test impossible de faire cette observation. On s’est donc mis en tete de faire un tableau contenant tous les id des nodes déjà reçus pour eviter de compter les doublons. Mais encore une fois on ne comptait pas suffisament de nœuds.

Finalement, on a fini par se rappeler que notre protocole retournait l'identifiant des nœuds sur un octet pouvant avoir comme valeur max 255 comme pour des soucis ce calcule de la longueur de chaine via strlen nous avions décidé d'ajouter 1 à tous les octets variables pour correctement estimer la taille de la chaine. Au final notre nombre maximum de nœud se limite à 254 si l'on demande la génération de plus de noeud cela fonctione mais les valeurs d'identifications des noeuds renvoyer sont tronqués d'où les doublons observés.

Pour résoudre définitivement cela nous avons ajouté une condition à notre main empéchant la création de plus de 254 nodes.

#### Erreur lors de la réception EBADCMD

Lors de la réception lorsque le programme envoyait le buffer de la trame à la fonction decodeAcquittalFrame() de temps en temps et de façon aléatoire la fonction renvoyait l'erreur suivante dans erreur_DDP(): 

`controller.c::readAcquittal() call decodeAcquittalFrame() : CMD not match with frame` 

Cette erreur était très inattendue d'autant plus que tous les tests unitaires qui ont été faits sur les fonctions de DDP.c fonctionnaient correctement dans toutes les situations. 

En observant le contenu de la trame envoyé vers la fonction, decodeAcuittalFrame() on pouvait voir des caractères initialisés après la première trame qui devait probablement trainer dans la mémoire à l'emplacement indiquer par le tableau. Pour régler ce problème lors de l'allocation mémoire de la trame nous avons utilisé la fonction memset(), qui nous permet de définir l’ensemble des bytes du tableau à 0 pour éviter ce genre d'erreurs. 

Le code fonctionnel ressemble à ça : 

```c
int bytes = 4096;
unsigned char frame[bytes];
//On utilise memset pour nettoyer les valeurs indésirables de notre tableau qui pourrais trainer dans la mémoire
//On le defines donc entièrement à 0
memset(frame, 0, bytes);
```

### Test unitaire

Afin d'améliorer la qualité et fiabilité de notre code particulièrement sur le protocole DDP dont la spécification était entièrement rédigée en amont de la programmation. Nous avons décidé d'utiliser le TDD(Test Driving Devloppement) pour la conception et la réalisation des fonctions de DDP.c.

Cela veut dire que nous avons commencé par rédiger des tests unitaires. Puis une fois les tests rédigés nous avons développé les fonctions jusqu'à ce que le code soit en mesure de passer tous les tests unitaire. 

Comme nous n'avions jamais fait de test unitaire en c et que nous n'avons pas eu le temps de nous pencher sur les librairies existantes. Nous avons juste recréé quelle que fonctions importantes afin de savoir si nos tests passent .Une fonction pour afficher le résultat du test une fonction pour résumer le nombre de tests passés et une fonction pour passer les tests.

Ensuite, nous avons créé autant de tests unitaires que nécessaire. La seule contrainte étant que les tests devaient renvoyer un booléen true si le test passe false s’il ne passe pas.

Nous avons aussi fait d'autres formes de tests avec printf lorsque les bugs auxquelles nous faisions fasse le nécessité..

Nous avons aussi commencé à faire des tests unitaires après avoir développé le contrôleur notamment pour corriger les erreurs de DUMP plus simplement.

Si vous souhaitez lancer les tests unitaire de DPP (testDDP.c) : make test_DDP

Si vous souhaitez lancer le test unitaire du contrôleur (testController.c) : make test_CONTROLLER

# Conclusion

L'objectif final de ce projet était de nous faire découvrir et utiliser les fonctions avancées de programmation système sur Linux en C. D’établir notre propre moyen de communication entre différent processus afin de nous faire réfléchir sur la façon d'organiser les données d'une application ainsi que leur cycle de vies.

Il nous aura fallu étudier attentivement la question afin de concevoir une solution à la fois faisable techniquement et au plus proche des attendus. C'est au moyen de nos différentes réflexions et analyses que nous avons pu réaliser les différentes étapes de ce projet. 

Ce travail, nous a permis de comprendre l'importance de correctement structurer son code et de nommer les variables de façon pertinente. Pour résoudre des problématiques complexe.

Nous avons beaucoup apprécié travailler sur ce projet, car il nous a accordé une grande liberté sur la manière d'aborder le problème qui nous était posé en n'imposant pas les façons de communiquer par les pipes.