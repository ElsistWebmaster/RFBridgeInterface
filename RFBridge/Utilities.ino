// *****************************************************************************
// Project    : MDP137A000
// Programmer : Sergio Bertana
// Date       : 14/02/2018
// *****************************************************************************
// Funzioni di utilità.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// DEFINIZIONE VARIABILI GLOBALI
// -----------------------------------------------------------------------------
// Definizione buffers ricezione dati RF.

unsigned char DProduce=0; //Data produce
unsigned char DConsume=0; //Data consume
unsigned char DBuffer[100]; //Data buffer
unsigned int DIndex[16]; //Data index
unsigned int DLength[16]; //Data length

// *****************************************************************************
// FUNZIONE CALCOLO SENSORI RICEVUTI
// *****************************************************************************
// Questa funzione ritorna il numero di sensori ricevuti.
// -----------------------------------------------------------------------------

unsigned char AcqSensor(void) 
{
    return(DProduce-DConsume);
}

// *****************************************************************************
// FUNZIONE MEMORIZZAZIONE FRAME
// *****************************************************************************
// Questa funzione esegue la memorizzazione del frame ricevuto.
// -----------------------------------------------------------------------------

bool DataProduce(unsigned char* Data, unsigned char Length) 
{
    // Definizione variabili locali.

    unsigned int POffset; //Produce offset

    // -------------------------------------------------------------------------
    // ESEGUO SALVATAGGIO DATI
    // -------------------------------------------------------------------------
    // Punto posizione buffer per salvataggio dati.

    if (DProduce >= sizeof(DIndex)/2) return(false);
    if (DProduce == 0) POffset=0; else POffset=DIndex[DProduce-1]+DLength[DProduce-1]; //Produce offset
    
    // Controllo se ancora spazio in buffer dati.

    if ((POffset+Length) >= sizeof(DBuffer)) return(false);

    // Salvo indice e lunghezza dati ricevuti.

    DIndex[DProduce]=POffset; //Data index
    DLength[DProduce]=Length; //Data length
    DProduce++; //Data produce

    // Trasferisco dati in buffer.

    for (unsigned int i=0; i<Length; i++)
    {
       DBuffer[POffset++]=*Data;
       Data++; //Data buffer pointer
    }
    return(true);
}

// *****************************************************************************
// FUNZIONE LETTURA FRAME
// *****************************************************************************
// Questa funzione esegue la lettura del frame ricevuto.
// -----------------------------------------------------------------------------

bool DataConsume(unsigned char* Data, unsigned char* Length) 
{
    // Definizione variabili locali.

    unsigned int COffset; //Consume offset

    // -------------------------------------------------------------------------
    // ESEGUO LETTURA DATI
    // -------------------------------------------------------------------------
    // Controllo se ho dati salvati.

    if (AcqSensor() == 0) return(false);

    // Punto posizione buffer per lettura dati.

    if (DConsume >= sizeof(DIndex)/2) return(false);
    *Length=DLength[DConsume]; //Ritorno lunghezza
    COffset=DIndex[DConsume]; //Consume offset
    DConsume++; //Data consume

    // Trasferisco dati in buffer.

    for (unsigned int i=0; i<*Length; i++)
    {
       if (COffset >= sizeof(DBuffer)) return(false);
       *Data=DBuffer[COffset++]; //Ritorno dato
       Data++; //Data buffer pointer
    }

    // Se consume uguale a produce ho acquisito tutti i sensori, resetto.

    if (DConsume == DProduce){DProduce=0; DConsume=0;}
    return(true);
}

// [End of file]
