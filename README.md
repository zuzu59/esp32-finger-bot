# esp32-remote-finger
Doigt qui appuie sur un bouton à distance

zf250624.2215, zf250626.1418

## Buts
Appuyer sur un bouton à distance !<br>
Donc de simplement faire un petit robot qui avec un servo moteur appuie sur un bouton.

![](https://raw.githubusercontent.com/zuzu59/esp32-finger-bot/refs/heads/master/images/ordinateur.jpeg)


## Problématiques
Parfois il est impossible d'allumer certains appareils électriques à distance. Ce petit robot permet d'y remédier à très faible coût.


## Moyens
J'ai utilisé pour ce projet un esp32-c3 super mini, qui va faire la connexion entre le servo moteur et Internet.<br>
Un petit serveur WEB sur le esp32-c3 qui va permettre, via une page WEB de piloter le servo moteur.


https://grabcad.com/library/esp32-c3-supermini-1

https://fr.aliexpress.com/item/1005006170575141.html

https://github.com/zuzu59/esp32-c3-pinout

![](https://raw.githubusercontent.com/zuzu59/esp32-finger-bot/refs/heads/master/images/driver_detail.jpeg)

et un vieux servo moteur de récupération

![](https://raw.githubusercontent.com/zuzu59/esp32-finger-bot/refs/heads/master/images/servo_detail.jpeg)


## Utilisation
Il suffit avec son browser WEB de se connecter sur l'adresse ip du esp-32:

http://adrs_ip

![](https://raw.githubusercontent.com/zuzu59/esp32-finger-bot/refs/heads/master/images/interfcae_web.png)




## Sources
https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561



