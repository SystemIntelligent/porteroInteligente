#include <SPI.h>
#include <MFRC522.h>
#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG

#ifdef  DEBUG
#define DELIMITER_CHARACTER '|'
#define START_CHARACTER 'S'
#define END_CHARACTER 'F'
#define SERIAL_PRINT(x,y)        Serial.print(x);Serial.println(y);
#endif

#ifndef DEBUG
#define DELIMITER_CHARACTER (int8_t)0xAE
#define START_CHARACTER (int8_t)0x91
#define END_CHARACTER (int8_t)0x92
#define SERIAL_PRINT(x,y)
#endif


#define PACKAGE_FORMAT "%c%c%s%c%s%c%c" //start character + delimiter character + payLoad + delimiter character + checksum + delimiter character + end character.
#define SIZE_PAYLOAD     40
#define SIZE_PACKAGE     SIZE_PAYLOAD + 7
#define SIZE_CHECKSUM    3


// ---------------------- Definicion de Pines -------------------------------//
#define RST_PIN          9           // Pin reset RFID.
#define SS_PIN           10          // Slave Select pin RFID.
#define BUZZER           2           // pin Buzzer.
#define PULSADOR         3           // pin Pulsador.
//---------------------------------------------------------------------------//

enum buzzerBeep {
  ONE_BEEP_SHORT = 0b01000000,
  TWO_BEEP_SHORT = 0b01010000,
  THREE_BEEP_SHORT = 0b01010100,
};


enum msgDisplay {
  MSG_WELCOME = 0,
  MSG_APPROACH_CARD,
  MSG_CARD_IN_FIELD,
  MSG_DING_DONG,
};

enum commands {
  OPEN_DOOR = 1,                 // comando recibido desde las raspberry.
  ACK_OPEN_DOOR,                 // ack enviado dede arduino a raspberry.
  BUTTON_PRESSED,                // comando enviado (cuando se pulsa el timbre) desde arduino a raspberry.
  ACK_BUTTON_PRESSED,            // ack recibido desde la raspberry.
  VALIDATE_CARD,                 // comando enviado (cuando se requiere validar una tarjeta) desde arduino a raspberry.
  CARD_VALID,                    // comando recibido desde la raspberry.
  CARD_NOT_VALID,                // comando recibido desde la raspberry.
};

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

LiquidCrystal_I2C lcd(0x3f, 16, 2); // Display Set 2 lineas por 16 caracteres.

bool  checkCardInField = false;
bool  refreshDisplay = false;
int   countAux0Timer = 0;  // contador auxiliar del timer usado para verificacion de tarjeta en campo.
int   countAux1Timer = 0;   // contador auxiliar del timer usado para refrescar display;


int   beepMode = 0;
int   msgNumber;

String receivedPackage = "";
bool packageComplete = false;

//------------------------------- Prototipos de funciones --------------------------------//

void  refreshBuzzer(void);
void  printDisplay(const char *line1, const char *line2);
void  printMsg(int msgNumber);
char  *calcChecksum(const char *data, int length);
char  *preparePackage(const char *payLoad, int length);
bool  compareChecksum(const char *checksumA, const char *checksumB);
char  *getChecksumFromReceivedPackage(const char *package, int length);
bool  validatePackage(const char *package, int length);
char  *disarmPackage(const char *package, int length);

//----------------------------------------------------------------------------------------//

void setup() {

  pinMode(BUZZER, OUTPUT);
  pinMode(PULSADOR, INPUT_PULLUP);
  Serial.begin(9600);

  lcd.init();                           // inicializacion del display
  lcd.backlight();                      // backLigth on.

  SPI.begin();
  rfid.PCD_Init();

  for (byte idx = 0; idx < 6; idx++) {  // Se setea la key que traen por defecto
    key.keyByte[idx] = 0xFF;            // de las tarjetas midfire.
  }
  Timer1.initialize(50000);             // se ajusto el timer para tener una interrupcion cada 50 mS.
  Timer1.attachInterrupt(ISR_TIMER);    // Iterrupt Request Service del Timer.

  printMsg(MSG_WELCOME);
  SERIAL_PRINT(F("\n> Inicialización finalizada."),"");
  attachInterrupt(digitalPinToInterrupt(PULSADOR), ISR_HW, LOW);
  receivedPackage.reserve(200);
  char msg[] = "ho ho ho i have a machine gun!!!";

  char pakage[SIZE_PACKAGE];
  char payLoad[SIZE_PAYLOAD];
  SERIAL_PRINT(F("\npayLoad: "), msg);
  strcpy(pakage, preparePackage(msg, strlen(msg)));
  SERIAL_PRINT(F("Package: "), pakage);

  /*
    Serial.print(F("Checksum is Valid ?: "));
    Serial.println(validatePackage(pakage, strlen(pakage)));
    strcpy(payLoad, disarmPackage(pakage, strlen(pakage)));

    Serial.print(F("\npayLoad Disarmed: "));
    Serial.println(payLoad);
  */
}

void loop() {

  if (packageComplete) {
    packageComplete = false;
    SERIAL_PRINT(F("\n> Recieved Package: "), receivedPackage);
    char  recievedPack[SIZE_PACKAGE];
    char  payLoad[SIZE_PAYLOAD];
    receivedPackage.toCharArray(recievedPack, sizeof(recievedPack));
    if (validatePackage(recievedPack, strlen(recievedPack))) {
      SERIAL_PRINT(F("> Checksum valid!!"), "");
      strcpy(payLoad, disarmPackage(recievedPack, strlen(recievedPack)));
      SERIAL_PRINT(F("> PayLoad Disarmed: "), payLoad);
    }
    else {
      SERIAL_PRINT(F("> Error bad Checksum !!"), "");
    }
  }

  if (refreshDisplay == true) {
    printMsg(msgNumber);
    refreshDisplay = false;
  }

  if (checkCardInField == true) {
    checkCardInField = false;
    if (rfid.PICC_IsNewCardPresent()) {
      SERIAL_PRINT(F("\n> Tarjeta en Campo"), "");
      msgNumber = MSG_CARD_IN_FIELD;
      beepMode = ONE_BEEP_SHORT;
      if ( rfid.PICC_ReadCardSerial()) {
        rfid.PICC_DumpDetailsToSerial(&(rfid.uid));
      }
    }
    else {
      msgNumber = MSG_APPROACH_CARD;
    }
  }


}

//----------------------------------------------------------------------------//
void printMsg(int msgNumber) {
  static int lastMsgNumber = 99;
  static bool block = false;
  static int blockCount = 0;


  if (block == true) {
    blockCount++;
    if (blockCount > 3) {
      blockCount = 0;
      block = false;
    }
  }

  if ((lastMsgNumber != msgNumber) && block == false) {
    lastMsgNumber = msgNumber;
    block = true;
    blockCount = 0;
    switch (msgNumber) {
      case MSG_WELCOME: printDisplay("Portero", "    Inteligente");
        break;
      case MSG_APPROACH_CARD: printDisplay("Acerque su      ", "     Tarjeta ...");
        break;
      case MSG_CARD_IN_FIELD: printDisplay("Tarjeta         ", "     en campo!! ");
        break;
      case MSG_DING_DONG: printDisplay("  Ding         ", "     Dong ...   ");
        break;

    }
  }
}
//----------------------------------------------------------------------------//
void printDisplay(const char *line1, const char *line2) {
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
//----------------------------------------------------------------------------//
void refreshBuzzer(void) {
  beepMode =  beepMode << 1;
  uint8_t result = (beepMode) & 0b10000000;
  if (result)
    digitalWrite(BUZZER, HIGH);
  else
    digitalWrite(BUZZER, LOW);
}
//-------------------------- Pulsador Handler -------------------------------//
void ISR_HW() {
  msgNumber = MSG_DING_DONG;
  refreshDisplay = true;
}
//--------------------------- Timer Handler ---------------------------------//
void ISR_TIMER(void) {
  refreshBuzzer();

  countAux0Timer++;
  if (countAux0Timer == 7) {
    countAux0Timer = 0;
    checkCardInField = true;
  }

  countAux1Timer++;
  if (countAux1Timer == 5) {
    countAux1Timer = 0;
    refreshDisplay = true;
  }
}

//-------------------------- Evento Serial Handler -------------------------//
void serialEvent() {

  static int state = 0;
  if (Serial.available()) {
    char inChar = (char)Serial.read();

    if (inChar == START_CHARACTER && state == 0) {
      state = 1;
      receivedPackage = inChar;
    }

    else if (inChar == START_CHARACTER && state == 1) {
      state = 1;
      receivedPackage = inChar;
    }

    else if (inChar != END_CHARACTER && state == 1) {
      receivedPackage += inChar;
    }

    else if (inChar == END_CHARACTER && state == 1) {
      receivedPackage += inChar;
      state = 0;
      packageComplete = true;
    }
  }
}
//--------------------------------------------------------------------------//
char* calcChecksum(const char *data, int length) {

  static char checksum[SIZE_CHECKSUM];
  int result = 0;

  for (int idx = 0; idx < length; idx ++) {
    result = result ^ data[idx];
  }

  if (result <= 15) {
    if (result <= 9) {
      checksum[0] = '0';
      checksum[1] = result + 48;
      checksum[2] = '\0';
    } else {
      checksum[0] = '0';
      checksum[1] = result + 87;
      checksum[2] = '\0';

    }
  } else {

    int rest;
    int quotient;
    char numHexa[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    rest = result % 16;
    quotient = result / 16;
    checksum[2] = '\0';
    checksum[1] = numHexa[rest];
    checksum[0] = numHexa[quotient];
  }

  return checksum;


}
//--------------------------------------------------------------------------//
char *preparePackage(const char *payLoad, int length) {

  char checksum[SIZE_CHECKSUM];
  memcpy(checksum, calcChecksum(payLoad, length), 3 * sizeof(char));

  static char package[SIZE_PACKAGE];
  snprintf(package, sizeof(package), PACKAGE_FORMAT, START_CHARACTER, DELIMITER_CHARACTER, payLoad, DELIMITER_CHARACTER, checksum, DELIMITER_CHARACTER, END_CHARACTER);
  return package;
}
//--------------------------------------------------------------------------//
bool compareChecksum(const char *checksumA, const char *checksumB) {
  bool result = true;

  for (int idx = 0; idx < SIZE_CHECKSUM; idx++) {
    if (checksumA[idx] != checksumB[idx]) {
      result = false;
    }
  }
  return result;
}
//--------------------------------------------------------------------------//
char *getChecksumFromReceivedPackage(const char *package, int length) {
  static char checksum[SIZE_CHECKSUM];
  byte countDelimiterCharacter = 0;

  int  index = 0;
  bool findOK = false;
  for (; index < length; index++) {
    if (package[index] == END_CHARACTER) {
      findOK = true;
      break;
    }
  }

  if (!findOK) {
    Serial.println("out");
    return NULL;
  }

  index -= 3;
  for (int idx = 0; idx < SIZE_CHECKSUM - 1; idx++) {
    checksum[idx] = package[index + idx];
  }
  checksum[2] = '\0';
  return checksum;
}
//--------------------------------------------------------------------------//
bool validatePackage(const char *package, int length) {

  return compareChecksum(getChecksumFromReceivedPackage(package, length), calcChecksum(package + 2, length - 7));
  // + 2 para saltearme el start_character y el -7 es (caracter de inicio + primer delimitador + 1 delimitador antes del checksum +  2 caracteres del checksum + ultimo delimitador
  // + caracter de fin.

}
//--------------------------------------------------------------------------//
char *disarmPackage(const char *package, int length) {
  static char payLoad[SIZE_PAYLOAD];
  memcpy(payLoad, package + 2, (length - 7 )*sizeof(char)); // +3 para salterame el caracter inicial(1) + primer delimitador(1); -7 para no copiar caracter de inicio
  // de paquete(1), primer delimitador(1), delimitador antes del checksum(1) el checksum(2) el limitador despues del checksum(1) y el caracter de fin de paquete(1).
  return payLoad;
}



