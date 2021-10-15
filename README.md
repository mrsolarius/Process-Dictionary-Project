# Rapport Process Dictonnary

* Introduction
* Conception
* La structure des données 
* La réalisation
* Conclusion

## Introduction

Le but de ce projet est de réaliser une application en ligne de commande permettant de stocker et de consulter un dictionnaire de données. Le dictionnaire permettra de stocker des données sous forme d'une chaine de caractère accessible depuis une clef représenter par un entier naturel. La particularité de ce projet vient de la nécessiter de stoker ces données dans plusieurs processus selon la valeur de la clef.

Le programme comportera 4 fonctions destiner à l'utilisateur : 

* Set : Permet de définir une valeur à un emplacement donnée
* Lookup : Permet de lire la valeur contenue à la clef indiquer
* Dump : Permet de lire toutes les valeurs contenues dans tous les processus
* Exit : Permet de quitter le programme 

Les processus doivent être capables de communiquer par tube entre eux afin de pouvoir se transférer les bonnes informations jusqu'à ce que l'un d'entre eux soit en capacité de la traiter et pour finalement la transférée au processus pair (le contrôleur).

## Conception

Afin de répondre au besoin attendu par le cahier des charges (le sujet du TP), nous avons imaginé une structure de donnée permettant la communication entre les différents processus. 

Nous sommes parties du constat que les pipes de donnée transfèrent des tableaux d'octets de la même manière que les sockets TCP/IP (Transmission Control Protocol). Nous avons donc procédé de la même marnière que lorsque l'on souhaite créer un protocole applicatif. 

Cela veut dire que nous organiserons le flux de communication en trame prédéfini nous permettant de structurer nos données de façon à savoirs quel comportement devra adopter chacun des processus.

### Étude de la structure des données nécessaire à la transition

Nous avons tous d'abord tenter d'identifier les flux de données qui passent par chacun des pipes. De cette façon nous en somme arrive au constat suivant : 

* Le contrôleur pose ses questions au premier de ses fils


* Les processus fils communiquent entre eux et ceux jusqu'à ce qu'il soit en mesure de répondre.
* Lorsqu’ils peuvent répondre ils envoient leur réponse au contrôleur.

Ce constat implique deux types de trame :

* Une trame de question
* Une trame d'acquittement

Pour être en mesure de savoir quoi mettre dans ces trames, nous devons nous demander quelle information transite dans chacune d'entre elle.

#### Trame de question

Les trame de question sont envoyé par le contrôler au premier nœud puis sont retransmis à tous les deux tant que le nœud courant ne peut pas traiter la question

Pour la fonction set nous avons deux informations envoyer :

* La commande, ici set
* L'emplacement de stockage

Pour la fonction lookup nous avons aussi deux informations :

* La commande, ici lookup
* L'emplacement de récupération de la valeur

Pour la commande dump nous avons toujours deux informations :  

* La commande, ici dump
* Le nombre de nœuds du système (pour éviter de faire une boucle infinie ou la commande dump ne s'arrête jamais)

Pour la commande exit nous avons une seule commande :  

* La commande, ici exit

#### Tram d'acquittement

Les trame d'acquittement sont transmises par les nœuds vers le contrôler pour confirmer une exécution de commande (que cela soit un succès ou qu'il y ait eu une erreur dans le traitement)

Lorsque la fonction set a été exécuté avec succès nous avons besoin de 3 informations :

* La commande, ici set 
* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)

Lorsque la fonction lookup a été exécuté avec succès nous avons besoin de 5 informations pour la traiter :

* La commande, ici lookup 


* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)


* La longueur de la chaine de caractère renvoyée
* La chaine de caractère trouver

Lorsque la fonction dump a été exécuté avec succès nous avons besoin de 3 informations :

* La commande, ici dump
* Le nœud qui a exécuté la commande (afin de savoir qui nous répond)
* Un drapeau d'erreur (afin de savoir si l'opération a réussi ou pas)

Lorsque la fonction exit a été exécuté, cela veut dire que le processus est mort, donc nous n'utiliserons pas de trame pour l'acquittement.

### Conception du protocole DDP (Data Dictionary Protocole)

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

**Note** : 

Le numéro à la fin de la valeur du flag indique la trame dans laquelle la commande peut être transmise.

*  → 1 Correspond à la trame ASK


*  → 2 Correspond à la trame ACQUITTAL

Cela simplifiera l'identification du type de trame dans le programme

 **Exemple** :

 0xA1 pourra être transmis uniquement par la trame ASK

 0xD2 pourra être transmis uniquement par la trame ACQUITTAL

**Valeur des commandes :**

*  0xA1 : Commande SET : Demande l'emplacement d'une donnée dans le processus à la clef indiquée dans DATA
*  0xA2 : Acquittement SET : Indique au contrôleur que le set à bien été traité
*  0xB1 : Commande LOOKUP : Demande la récupération d'une donnée dans les processus
*  0xB2 : Acquittement LOOKUP : Indique au contrôleur que le lookup à bien été traité
*  0xD1 : Commande DUMP : Demande l'affichage des données stoker dans le processus
*  0xD2 : Acquittement DUMP : Indique au contrôleur que le DUMP à bien été traité
*  0xE1 : Commande EXIT : Demande au processus de se suicider

##### Identifiant du node (NODE ID)

Le node id correspond à l'identifiant du processus, il est contenu entre 0 et 255 (il n'est pas possible d'avoirs plus de 255 fils pour utiliser ce protocole)

 Les processus fils ont forcément un id qui se suivent et le premier nœud à obligatoirement l'id 0

 Cet id permet au contrôleur de savoir qui a récupéré l'information qu'il a demandé parmi ses fils

 sur les actions globales comme dump, cela permet aussi au contrôleur d'attendre que tous les processus aient fini le traitement.  Avant d'en commencer un nouveau.

**Exemple :**

```
0 1 2 3 4 5 6 OK
1 2 3 4 5 6 7 FAUX
0 1 3 4 5 6 7 FAUX
```

 

##### Drapeau d'erreur (ERROR FLAG)

Les drapeaux d'erreurs permettent d'indiquer au contrôleur que quelle que chose ne s’est pas bien passé ou au contraire si tout a bien fonctionné.

*  0x20 : Aucune erreur lors de l'exécution du processus fils
*   0x44 : Aucune donnée trouver
*   0x50 : Une erreur, c’est produit lors de l'exécution du processus

##### Données (DATA) 

 Data est utilisée dans nos deux frame ASK et ACQUITTAL pour transmettre des données. Cependant, il n'est pas utiliser de la même façon dans les deux cas.

######  ASK Frame :

 Dans cette trame DATA correspond à un unsigned short qui correspond à la clé qui doit être enregistré.

Note : DATA n'est pas obligatoire pour les commandes EXIT, et DUMP.

######  ACQUITTAL Frame :

 DATA LENGHT : Ici DATA LENGTH correspond à la taille de la donnée contenue dans DATA.

 DATA : Ici DATA correspond à une donnée sous forme d'un tableau de char

Note : DATA LENGHT et DATA ne sont pas obligatoire pour les trames SET et DUMP

##### Fin de trame (END FRAME)

 La END FRAME permet de savoir quand es que le flux de donné se termine.

*   0x04 : END FRAME : Cette valeur est connue et sera toujours la même pour terminer la transition de données.

 Elle correspond au caractère ASCII end of transmission (EOF)
