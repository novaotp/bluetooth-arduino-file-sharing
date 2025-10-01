
# Arduino program

## Getting started

1. Ouvrir le fichier `arduino-program.ino` avec l'Arduino IDE
2. Modifier les variables marquées d'un commentaire: `!!! CHANGE THIS !!!`
3. Téléverser le code sur l'*Arduino Uno*

## Documentation de la trame

| STX  | Adresse                    | Longueur         | Body     | Vérif                                         | STX  |
| ---- | -------------------------- | ---------------- | -------- | --------------------------------------------- | ---- |
| 0x02 | 4 bytes pour afficher l'IP | Longueur de body | 128 bytes | Vérification avec l'adresse, longueur et body | 0x03 |
