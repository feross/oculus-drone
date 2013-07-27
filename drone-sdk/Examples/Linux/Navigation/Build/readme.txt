-Procédure de téléchargement de l'application
Télécharger la package SDK à l'url : https://projects.ardrone.org/wiki/ardrone-api. 

-Pré-requis
Installer les packages : libsdl1.2-dev ou supérieur, gtk-2.0 ou supérieur, gcc version 4.3.3 ou supérieur. 

-Procédure d'installation sous linux
   - Extraire les fichiers en tapant la commande : tar -xjvf ARDrone_API-x.x.x.tar.bz2
   - Aller dans ARDroneLib/Soft/Build :
     Éditer le fichier config.makefile et changer la ligne "BUILD_LINUX_EXAMPLES=no" par "BUILD_LINUX_EXAMPLES=yes" 
     Taper make, les binaires sont copiés dans ARDroneLib/Version. 

-Procédure de modification de l'application ?
   - Éditer le code source dans Examples/Linux/Navigation/Sources. Le programme Navigation utilise le point d'entrée principale de la librarie ardrone. Il est customiser par les fonctions déclarer dans le fichier common/mobile_main.c. Le répertoire /UI contient les fichiers qui gère les périphériques de contrôle tel que le gamepad ou la wiimote. Le répertoire /ihm gère l'affichage du panneau de contrôle. Le répertoire /navdata_client gère la réception des données de navigation provenant du Drone [Navdata]. Le répertoire /utils contient des fonctions d'affichage pour le débogage.

   - Pour recompiler l'application, aller dans ARDroneLib/Soft/Build et taper make.

-Explication de l'application installée et des différents réglages
Le premier panneau de contrôle indique l'état du Drone :
 * Bouton start. Vert : Décollage. Rouge : Attérissage.
 * Mykonos global state : Valeur en hexadécimale détaillée dans l'enum def_ardrone_state_mask_t du fichier ARDroneLib/Soft/Common/config.h. 
 * Mykonos control state: Indique si le Drone est en phase de décollage, en vol, en phase d'attérissage, au sol. Voir enum CONTROL_STATE dans le fichier ARDroneLib/Soft/Common/config.h.     
 * Mykonos vision state : Détect le déplacement du Drone.
 * Le deuxième panneau de contrôle permet d'affiner les gains des moteurs. Le bouton "Send control gains" envoi les nouveaux paramètres au Drone.
 * Le bouton "Set configuration params" : Fait apparaître le menu de configuration du Drone. Le champ texte correspondant au label "Parameters" indique le paramètre à changer et le champ texte du label "Value" indique ça nouvelle valeur. Pour récupérer les noms des paramètres editer le fichier ARDroneLib/Soft/Common/config_key.h. Le bouton "Set configuration to Toy" envoi la nouvelle config au Drone.
Exemple pour activer la vidéo :
Parameter: "GENERAL:video_enable" Value: TRUE 
 * Le bouton "Flat trim": Envoi une demande de calibrage du Drone au sol.  
 * Le panneau "Camera detect" : Choix le l'algo de vision du Drone par : le menu "Select camera detect" qui permet de choisir dans l'ordre (Détection tag-3D via la caméra "horizontal" ou "vertical" et enfin "shell" pour la détection de carène. Pour la détection de carène on peux choisir la couleur avec le menu "change enemy color" et le type de carène "Flight without shell". Le menu "Number of detected tags" indique le nombre maximum de détection de carène.   
* Le panneau "Led animation" : Permet de jouer faire clignoter les leds en choisissant une animation, une periode et une fréquence.
* Le menu "Select anim" permet de faire jouer une figure automatique au Drone.

- Explication des instruments de mesure
* VISION image : Permet de visualiser la caméra du Drone. Le bouton "Zapper" permet de changer de caméra (Horizontal, vertical ou les deux). Le bouton "RAW capture" permet de sauvegarder la video sur le Drone. 

