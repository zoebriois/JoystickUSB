Le programme est divisé en 3 parties:
  - le contrôle de la manette USB
    -> répertoire USB
    Commandes à exécuter:
      make
      sudo ./tutorat

    Ce qui est fait:
    - affichage des différentes informations sur le périphérique
    - sauvegarde des adresses des endpoints interruptions dans un tableau
    - si on reçoit un caractère ("o" ou "p") correspondant à un ordre sur la led L
      -> envoi sur l'endpoint LED_OUT_1


  - le programme concernant l'ATMega328p
    -> repertoire ATMega328p
    Commandes à executer:
      make
      make upload

      Ce qui est fait:


  - le programme concernant l'ATMega16u2
    -> répertoire ATMega16u2/PolytechLille/PAD
    Commandes à executer:
      (court-circuiter les lignes reset et masse de l’ATMega16u2)
      dfu-programmer atmega16u2 erase
      dfu-programmer atmega16u2 flash ATMega16u2.hex
      dfu-programmer atmega16u2 reset
      (enlever le court-circuit)


      Ce qui est fait:
      - configuration de 2 interfaces avec chacune 2 endpoints
      - si on reçoit quelque chose sur le port série qui correspond aux boutons du joystick, on découpe selon:
        - endpoint JOYSTICK_IN_1 = information "gros bouton" du joystick avec les 7 bits de points fort à 0
        - endpoint JOYSTICK_IN_2 = information des boutons D3 D4 D5 D6 avec les 4 bits de points fort à 0

      - si on reçoit quelque chose sur les endpoints out (leds), on reforme selon:
        - endpoint LED_OUT_1 = information sur la LED L = o ou p
        - endpoint LED_OUT_2 = information sur les LEDs 8 à 12
