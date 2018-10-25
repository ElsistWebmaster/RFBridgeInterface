// *****************************************************************************
// Project    : MDP137A000
// Programmer : Sergio Bertana
// Date       : 14/02/2018
// *****************************************************************************
// Gestione dispositivo Sonoff 433 RF bridge.
// Il valore ritornato da millis esegue overlap dopo circa 50 giorni.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ESEGUO DEFINIZIONI GENERALI
// -----------------------------------------------------------------------------
// Includo definizioni generali.

#include <ESP8266WiFi.h>

// Definizione versione firmware.

#define FIRMWAREVERSION "MDP137A000"

// Definizione segnali hardware dispositivo.

#define BUTTON 0 //GPIO0, Button
#define BLUE_LED 13 //GPIO13 (MTCK), Blue led

// -----------------------------------------------------------------------------
// DEFINIZIONE COMANDI TELNET
// -----------------------------------------------------------------------------

#define STATUS_REQUEST 1 //Richiesta status
#define SENSOR_DATA 2 //Dati sensore
#define RF_DATA_SEND 3 //Invio dati RF
#define SYSTEM_PROMPT 100 //Invio prompt

// -----------------------------------------------------------------------------
// DEFINIZIONE CODICI RF
// -----------------------------------------------------------------------------
// Definizione codici RF.

#define RF_MESSAGE_SIZE         9
#define RF_MAX_MESSAGE_SIZE     (112+4)
#define RF_CODE_START           0xAA
#define RF_CODE_ACK             0xA0
#define RF_CODE_LEARN           0xA1
#define RF_CODE_LEARN_KO        0xA2
#define RF_CODE_LEARN_OK        0xA3
#define RF_CODE_RFIN            0xA4
#define RF_CODE_RFOUT           0xA5
#define RF_CODE_SNIFFING_ON     0xA6
#define RF_CODE_SNIFFING_OFF    0xA7
#define RF_CODE_RFOUT_NEW       0xA8
#define RF_CODE_LEARN_NEW       0xA9
#define RF_CODE_LEARN_KO_NEW    0xAA
#define RF_CODE_LEARN_OK_NEW    0xAB
#define RF_CODE_RFOUT_BUCKET    0xB0
#define RF_CODE_STOP            0x55

// Struttura messaggio Da/Verso sezione RF.

// AA A4 31 1A 01 A4 04 C4 D1 44 EE 55 
// Il frame inizia con 0xAA
// 1 byte di "Action" (0xA4 Rxd), (0xA5 TxD)
// 2 byte "TSyn"
// 2 byte "TLow"
// 2 byte "THigh"
// 3 byte "Data"
// Il frame termina con 0x55

// -----------------------------------------------------------------------------
// CONFIGURAZIONE RETE
// -----------------------------------------------------------------------------
// Configurazione WiFi.

const char* SSID=”Your SSID”; //Name of your network (SSID)
const char* Password=”Password”; //WiFi password
IPAddress IP(192,168,0,183); //Node static
IPAddress Gateway(192,168,0,1); //Gateway IP
IPAddress Subnet(255,255,255,0); //Subnet mask

// Definizione porta Server.

WiFiServer Server(1000); //Initialize the server library
WiFiClient Client; //Initialize the client library

// -----------------------------------------------------------------------------
// DEFINIZIONE VARIABILI GLOBALI
// -----------------------------------------------------------------------------

bool ButtonSts; //Button status
char TCPData[80]; //TCPData data buffer
unsigned long WiFiEr; //WiFi errors

// *****************************************************************************
// INIT PROGRAM
// *****************************************************************************
// Programma eseguito alla accensione.
// -----------------------------------------------------------------------------

void setup() 
{
    // -------------------------------------------------------------------------
    // IMPOSTAZIONE PIN GPIO
    // -------------------------------------------------------------------------
    // Impostazione funzioni su pin GPIO.

    pinMode(BLUE_LED, OUTPUT); //Imposto pin come output
    pinMode(BUTTON, INPUT); //Imposto pin come input   

    // Setto stato di default sui pin di GPIO.
          
    digitalWrite(BLUE_LED, LOW); //Accendo LED
    digitalWrite(BUTTON, HIGH); //Predispongo acquisizione pulsante

    // -------------------------------------------------------------------------
    // ATTIVAZIONE SERVER SERIALE
    // -------------------------------------------------------------------------
    // Attivo server seriale.

    Serial.begin(19200); //19200, n, 8

    // -------------------------------------------------------------------------
    // ATTIVAZIONE WIFI
    // -------------------------------------------------------------------------
    // Eseguo attivazione WiFi.

    WiFi.mode(WIFI_STA); //Imposto modalità station   
    delay(100); //Attesa 100 mS
    WiFi.begin(SSID, Password, IP); //Aggancio WiFi alla rete
    WiFi.config(IP, Gateway, Subnet); //Imposto parametri di rete
}

// *****************************************************************************
// MAIN PROGRAM LOOP
// *****************************************************************************
// Program loop eseguito ciclicamente.
// -----------------------------------------------------------------------------

void loop() 
{
    // -------------------------------------------------------------------------
    // DEFINIZIONE VARIABILI
    // -------------------------------------------------------------------------
    // Definizione variabili.

    static bool FirstLoop=true; //First loop
    static bool WiFiOn=false; //WiFi attivo
    static bool LEDStatus; //LED status
    static bool IsConnect; //Client is connect
    static unsigned char RFRxIDx; //RF Rx index
    static unsigned char TCPRxIDx; //TCP Rx index
    static unsigned char CType=SYSTEM_PROMPT; //Command type
    static unsigned char RFData[80]; //RF data buffer
    static unsigned long LEDTmBf; //LED time buffer (mS)
    static unsigned long SerialTimeBf; //Serial time buffer (mS)
    static unsigned long TCPTimeBf; //TCP time buffer (mS)

    // -------------------------------------------------------------------------
    // INIZIALIZZAZIONI
    // -------------------------------------------------------------------------
    // Eseguo inizializzazioni.

    if (FirstLoop)
    {
        FirstLoop=false; //First loop
        LEDTmBf=millis(); //LED time buffer (mS)
    }

    // -------------------------------------------------------------------------
    // CONFIGURAZIONE WIFI
    // -------------------------------------------------------------------------
    // Eseguo attesa connessione alla rete.

    if (WiFi.status() != WL_CONNECTED)
    {
        if ((millis()-LEDTmBf) > 100)
        {
            LEDTmBf=millis(); //LED time buffer (mS)
            (LEDStatus)?LEDStatus=false:LEDStatus=true;
            digitalWrite(BLUE_LED, LEDStatus);
        }
        WiFiOn=false; //WiFi attivo
        return;
    }

    // Inizializzo server.

    if (!WiFiOn) 
    {
      // Inizializzo server.

        WiFiOn=true; //WiFi attivo
        IsConnect=false; //Client is connect
        RFRxIDx=0; //RF Rx index
        LEDTmBf=millis(); //LED time buffer (mS)
        TCPTimeBf=millis(); //TCP time buffer (mS)
        SerialTimeBf=millis(); //Serial time buffer (mS)
        Server.begin(); //Aggancio WiFi alla rete
    }

    // Errore connessione se non ricevo dati dopo timeout reinizializzo.

    if ((millis()-TCPTimeBf) > 60000)
    {
        WiFiOn=false; //WiFi attivo
        WiFiEr++; //WiFi errors
        Client.stop();
        Server.stop();
        WiFi.begin(); //Aggancio WiFi alla rete
    }

    // -------------------------------------------------------------------------
    // GESTIONE LAMPEGGIO LED
    // -------------------------------------------------------------------------
    // Viene eseguito il lampeggio del led.

    if ((millis()-LEDTmBf) > 500)
    {
        LEDTmBf=millis(); //LED time buffer (mS)
        (LEDStatus)?LEDStatus=false:LEDStatus=true;
        digitalWrite(BLUE_LED, LEDStatus);
    }

    // -------------------------------------------------------------------------
    // ACQUISIZIONE PULSANTE
    // -------------------------------------------------------------------------
    // Eseguo acquisizione pulsante, lo stato và invertito.

    ButtonSts=!digitalRead(BUTTON); //Button status

    // -------------------------------------------------------------------------
    // ACQUISIZIONE DATI DA RICEVITORE RF
    // -------------------------------------------------------------------------
    // Eseguo controllo se pausa ricezione dati e carico dati ricevuti.

    if ((RFRxIDx != 0) && ((millis()-SerialTimeBf) > 100))
    {
        DataProduce(&RFData[0], RFRxIDx);
        RFRxIDx=0; //RF Rx index
    }

    // Eseguo controllo se dati da seriale.

    while (Serial.available() > 0)
    {
        SerialTimeBf=millis(); //Serial time buffer (mS)
        if (RFRxIDx < sizeof(RFData)) RFData[RFRxIDx++]=Serial.read(); //Leggo carattere dal buffer seriale
    }

    // -------------------------------------------------------------------------
    // CONTROLLO CONNESSIONE CLIENT
    // -------------------------------------------------------------------------
    // Controllo se Client connesso.

    if (!IsConnect)
    {
        Client=Server.available(); //Client
        if (Client)
        {
            IsConnect=true; //Client is connect
            CType=SYSTEM_PROMPT; //Command type
            TCPRxIDx=0; //TCP Rx index
            Client.print("Elsist Sonoff firmware: "FIRMWAREVERSION"\r\nWelcome...\r\n");
        }
    }
    else
    {
        if (!Client.connected()) IsConnect=false; //Client is connect
    }

    // -------------------------------------------------------------------------
    // GESTIONE COMANDI
    // -------------------------------------------------------------------------
    // Gestione comandi ricevuti.

    if (!IsConnect) return; //Nessun client connesso
 
    // Eseguo selezione comando ricevuto.

    switch (CType)
    {
        case SYSTEM_PROMPT: Client.print("> "); CType=0; TCPRxIDx=0;  break;
        case STATUS_REQUEST: if (StatusTCPData(false)) CType=SYSTEM_PROMPT; return; //Richiesta status
        case SENSOR_DATA: if (SensorData(false)) CType=SYSTEM_PROMPT; return; //Lettura dati sensore
        case RF_DATA_SEND: if (RFOut(false)) CType=SYSTEM_PROMPT; return; //Invio dati RF
     }
 
    // -------------------------------------------------------------------------
    // GESTIONE PROTOCOLLO
    // -------------------------------------------------------------------------
    // available, get the number of bytes (characters) available for reading.
    // This is data that's already arrived and stored in the receive buffer
    // (which holds 64 bytes).
    // -------------------------------------------------------------------------
     // Controllo se caratteri ricevuti dal client. 

    while (Client.available() > 0)
    {
        // Eseguo lettura caratteri e li salvo in buffer.

        if (TCPRxIDx < (sizeof(TCPData)-1)) TCPData[TCPRxIDx++]=Client.read(); //Carattere letto

        // Eseguo controllo se ricevuto <CR>.

        if (TCPData[TCPRxIDx-1] == 13)
        {
            TCPData[TCPRxIDx]=0; //Codice tappo
            TCPTimeBf=millis(); //Command time buffer (mS)
            Client.print(TCPData);

            // Eseguo controllo comando ricevuto.

            if (strstr(TCPData, "Status") != NULL) {CType=STATUS_REQUEST; StatusTCPData(true);}
            if (strstr(TCPData, "Sensor") != NULL) {CType=SENSOR_DATA; SensorData(true);}
            if (strstr(TCPData, "RFOut") != NULL) {CType=RF_DATA_SEND; RFOut(true);}

            // Se comando non riconosciuto output prompt.

            if (!CType)
            {
                Client.print("Command mismatch...\r\n");
                CType=SYSTEM_PROMPT; //Command type
            }
        }
    }
}

// *****************************************************************************
// GESTIONE COMANDO RICHIESTA STATO
// *****************************************************************************
// Ritorna lo stato del dispositivo.
// -----------------------------------------------------------------------------

bool StatusTCPData(bool Init) 
{
    // -------------------------------------------------------------------------
    // DEFINIZIONE VARIABILI
    // -------------------------------------------------------------------------
    // Definizione variabili.

    char OBuffer[80]; //Output buffer
    static unsigned char CaseNr; //Program case
    static long RSSIValue; //Valore Rssi
   
    // -------------------------------------------------------------------------
    // GESTIONE INIZIALIZZAZIONE
    // -------------------------------------------------------------------------
    // Gestione inizializzazione.

    if (Init) {CaseNr=0; return(false);}

    // -------------------------------------------------------------------------
    // GESTIONE CASES COMANDO
    // -------------------------------------------------------------------------
    // Gestione cases comando.

    switch (CaseNr++)
    {
        case 0: RSSIValue=WiFi.RSSI(); return(false);
        case 1: sprintf(OBuffer, "RSSI value: %ld\r\n", RSSIValue); Client.print(OBuffer); return(false);
        case 2: sprintf(OBuffer, "WiFi errors: %ld\r\n", WiFiEr); Client.print(OBuffer); return(false);
        case 3: sprintf(OBuffer, "Button is: %s\r\n", (!ButtonSts)?"Off":"On"); Client.print(OBuffer); return(false);
        case 4: sprintf(OBuffer, "Sensor acquired: %d\r\n", AcqSensor()); Client.print(OBuffer); return(false);
        default: return(true);
    }
}

// *****************************************************************************
// GESTIONE COMANDO DATI SENSORE
// *****************************************************************************
// Gestisce il comando di lettura sensore.
// -----------------------------------------------------------------------------

bool SensorData(bool Init) 
{
    // -------------------------------------------------------------------------
    // DEFINIZIONE VARIABILI
    // -------------------------------------------------------------------------
    // Definizione variabili.

    unsigned char Length; //Data length
    unsigned char Data[80]; //Data buffer
    char OBuffer[80]; //Output buffer

    // -------------------------------------------------------------------------
    // INIZIALIZZAZIONE COMANDO
    // -------------------------------------------------------------------------
    // Eseguo inizializzazione comando.

    if (Init) return(false);

    // -------------------------------------------------------------------------
    // NESSUN SENSORE ACQUISITO
    // -------------------------------------------------------------------------
    // Se nessun sensore acquisito ritorno messaggio.

    if (AcqSensor() == 0)
    {
        sprintf(OBuffer, "No sensor\r\n");
        Client.print(OBuffer);
        return(true);
    }

    // -------------------------------------------------------------------------
    // RITORNO DATI SENSORE
    // -------------------------------------------------------------------------
    // Ritorno dati sensore.

    DataConsume(Data, &Length);
    sprintf(OBuffer, "Sensor:");

    // Eseguo loop ritorno dati sensore.

    for (unsigned char i=0; i<Length; i++) sprintf(OBuffer+strlen(OBuffer), "%02X", Data[i]);
    sprintf(OBuffer+strlen(OBuffer), "\r\n");
    Client.print(OBuffer);
    return(true);
 }

// *****************************************************************************
// GESTIONE COMANDO INVIO DATI RF
// *****************************************************************************
// Gestisce il comando di invio dati RF.
// A seguito del comando vi sono i bytes di dato da trasmettere (Sempre 12).
// Esempio: RFOut AAA52242011803703B3EE855<CR>
// 2 bytes RF_CODE_START (0xAA), RF_CODE_RFOUT (0xA5)
// 9 bytes di dato
// 1 byte RF_CODE_STOP (0x55)
//
// Ecco la legenda dei dati inviati alla sezione RF.
// Start frame 0xAA
// Action (1 bytes) 0xA5
// Sync time (2 bytes)
// Low level time (2 bytes)
// High level time (2 bytes)
// Data (3 bytes)
// Fine frame 0x55
//
// Note:
// Gli ultimi 3 bytes del dato sono il codice ricevuto dal ricevitore RF
// collegato allo SlimLine (Vedi progetto HS1527Decoding). Ecco alcuni esempi:
// AAA4 2242012203703B3EE1 55 -> 3882721
// AAA4 2242011803703B3EE2 55 -> 3882722
// AAA4 2238012203703B3EE4 55 -> 3882724
// -----------------------------------------------------------------------------

bool RFOut(bool Init) 
{
    // -------------------------------------------------------------------------
    // DEFINIZIONE VARIABILI
    // -------------------------------------------------------------------------
    // Definizione variabili.

    int i; //Auxiliary char
    char Data; //Data out
    static char RFCode[32]; //RF code data buffer

    // -------------------------------------------------------------------------
    // INIZIALIZZAZIONE COMANDO
    // -------------------------------------------------------------------------
    // Eseguo inizializzazione comando. Nel buffer di ricezione TCP a seguito
    // del comando vi sono i bytes di dato da trasmettere (Sempre 12).

    if (Init)
    {
        memset(&RFCode, 0, sizeof(RFCode));
        memcpy(&RFCode, &TCPData[6], 12*2);
        return(false);
    }

    // -------------------------------------------------------------------------
    // INVIO CODICE RF
    // -------------------------------------------------------------------------
    // Eseguo invio codice verso sezione RF.
       
    Serial.println(); //Invia <CR><LF>
 
    for (i=0; i<12; i++)
    {
      sscanf(&RFCode[i*2], "%02x", &Data);
      Serial.write(Data);
    }

    Serial.flush(); //Eseguo flush dati
    Serial.println(); //Invia <CR><LF>
    return(true);
}

// [End of file]
