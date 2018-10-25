Questo progetto illustra come utilizzare un PLC con programmazione IEC-61131 per gestire un ricevitore RF433 Mhz per ricevere i dati da sensori domotici RF433. Il progetto vuole essere una alternativa ai tanti progetti analoghi basati su Arduino o Raspberry. Per una spiegazione dettagliata rimando a [questo articolo](https://support.elsist.biz/articoli/gestione-domotica-con-sensori-in-rf433/).

# RFBridgeInterface

Questo blocco funzione gestisce la decodifica del protocollo HS1527, EV1527, RT1527 o FP1527 in uscita da un ricevitore RF tipo MX-05V come quello visibile nella foto.

![MX-05V receiver](https://github.com/ElsistWebmaster/Home-automation-RF433/blob/master/Images/MX-05V-RF-Receiver.jpg?raw=true)

# CPU SlimLine

Il ricevitore RF viene connesso ad un ingresso digitale di un modulo CPU della serie SlimLine che tramite il FB **HS1527Decoding** ne gestisce la decodifica ritornando il codice acquisito. Come si vede dalla foto sotto il modulo viene alimentato dallo stesso modulo CPU ed è riportato anche il semplice programma FBD di gestione.

![MX-05V connected to CPU SlimLine](https://github.com/ElsistWebmaster/Home-automation-RF433/blob/master/Images/MX-05V-RF-To-CPU.jpg?raw=true)

# Sensori

Sul mercato esistono moltissimi sensori wireless che possono essere acquisiti, ecco alcuni dei dispositivi compatibili che possono essere reperiti facilmente dal mercato ad un costo inferiore alla decina di euro.

![Thin remote controller](https://github.com/ElsistWebmaster/Home-automation-RF433/blob/master/Images/Thin-Remote-Controller.jpg?raw=true)
![Wireless door detector](https://github.com/ElsistWebmaster/Home-automation-RF433/blob/master/Images/CD100S-Door-Detector.jpg?raw=true)
![Wireless PIR detector](https://github.com/ElsistWebmaster/Home-automation-RF433/blob/master/Images/CT60-PIR-Sensor.jpg?raw=true)

