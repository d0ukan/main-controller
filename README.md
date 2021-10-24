# main-controller

Main source controller with RF - GPS project

working with **13.56 MHz** rf tags

must be required Libs : 
- MFRC522
- RF24

### to use with rf unique , need to change
```
rfid.uid.uidByte[0] == 186 &&
rfid.uid.uidByte[1] == 117 &&
rfid.uid.uidByte[2] == 102 &&
rfid.uid.uidByte[3] == 103
```


>***made by flurex***
