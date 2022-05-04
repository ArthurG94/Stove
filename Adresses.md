
## Ram
0x00 Compteur interne (s'incrément constamment)
0x01 Température d'ambiance (x2)
0x21 Etat du poile 	0 = désactivé
					1 = Démarrer
					2 = chargement de pellets
					3 = allumage
					4 = travail
					5 = Nettoyage du brasier
					6 = Nettoyage final
					7 = veille
					8 = alarme Pellet manquant
					9 = Alarme de défaillance d'allumage
					10 = alarmes non définies (à examiner)

0x31 Temps restant avant nettoyage (seconde)
0x32 Temps restant avant nettoyage (minute)

0x37 Vitesse du ventilateur des fumées
0x38 ??? 4eme bit alterne quand vis pellet

0x5A Température des fumées

0x65 Date : Seconde
0x66 Date : Jour de la semaine
0x67 Date : Heures
0x68 Date : Minutes
0x69 Date : Jour
0x6A Date : Mois
0x6B Date : Année

0x7E température ? Vitesse ventillateur ?

### Non vérifié
0x34 Puissance actuelle du poele
0x0D Quantité chargement grannulé en cours ????


## EEPROM

0x07 Consigne de vitesse ventilateur fumée  (Pendant refroidissement ?)
0x7D Consigne de température

### Non vérifié