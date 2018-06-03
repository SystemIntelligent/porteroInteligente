
#include <SPI.h>
#include <MFRC522.h>
#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#ifndef DELIMITER_CHARACTER
#define DELIMITER_CHARACTER (int8_t)0xAE
//#define DELIMITER_CHARACTER '|'
#endif

#ifndef START_CHARACTER
#define START_CHARACTER (int8_t)0x91
//#define START_CHARACTER 'S'
#endif

#ifndef END_CHARACTER
#define END_CHARACTER (int8_t)0x92
//#define END_CHARACTER 'F'
#endif

#ifndef PACKAGE_FORMAT
#define PACKAGE_FORMAT "%c%c%s%c%s%c%c" //start character + delimiter character + payLoad + delimiter character + checksum + delimiter character + end character.
#endif

#define SIZE_PACKAGE     250
#define SIZE_CHECKSUM    3


// ---------------------- Definicion de Pines -------------------------------//
#define RST_PIN         9           // Pin reset RFID.
#define SS_PIN          10          // Slave Select pin RFID.
#define BUZZER          2           // pin Buzzer.
#define PULSADOR        3           // pin Pulsador.
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

String inputString = "";
bool stringComplete = false;

//------------------------------- Prototipos de funciones --------------------------------//

void  refreshBuzzer(void);
void  printDisplay(const char *line1, const char *line2);
void  printMsg(int msgNumber);
char* calcChecksum(const char *data, int length);
char* preparePackage(const char *payLoad, int length);
bool  compareChecksum(const char *checksumA, const char *checksumB);
char* getChecksumFromReceivedPackage(const char *package, int length);
bool  validatePackage(const char *package, int length);

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
  Serial.println(F("> Inicializacion finalizada"));
  delay(1500);
  attachInterrupt(digitalPinToInterrupt(PULSADOR), ISR_HW, LOW);
  inputString.reserve(200);
  //char temp[] = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
  char temp[] = "HolaJuu";
  
  char temp1[250];
  strcpy(temp1, preparePackage(temp, sizeof(temp)));
  Serial.println(temp1);
  for(int i = 0; i < strlen(temp1); i++){
    Serial.print(temp1[i],16);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println(getChecksumFromReceivedPackage(temp1, strlen(temp1)));
  Serial.println(validatePackage(temp1,strlen(temp1)));
}

void loop() {

  if (stringComplete) {
    stringComplete = false;
    Serial.print("--- string recibido: ");
    Serial.println(inputString);
  }

  if (refreshDisplay == true) {
    printMsg(msgNumber);
    refreshDisplay = false;
  }

  if (checkCardInField == true) {
    checkCardInField = false;
    if (rfid.PICC_IsNewCardPresent()) {
      Serial.println(F("\n> Tarjeta en Campo"));
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
    if (blockCount > 4) {
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
  if (countAux0Timer == 15) {
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
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    if (inChar == START_CHARACTER && state == 0) {
      state = 1;
      inputString = inChar;
    }

    else if (inChar == START_CHARACTER && state == 1) {
      state = 1;
      inputString = inChar;
    }

    else if (inChar != END_CHARACTER && state == 1) {
      inputString += inChar;
    }

    else if (inChar == END_CHARACTER && state == 1) {
      inputString += inChar;
      state = 0;
      stringComplete = true;
    }
  }
}
//--------------------------------------------------------------------------//
char* calcChecksum(const char *data, int length) {

  for(int i = 0; i < length; i++){
    Serial.print(data[i],16);
    Serial.print(" ");
  }
  Serial.println("");
  
  static char checksum[SIZE_CHECKSUM];
  unsigned int result = 0;

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
  for(int i = 0; i < 3; i++){
    Serial.print(checksum[i],16);
    Serial.print(" ");
  }
  Serial.println("");
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

  for (int idx = 0; idx < 3; idx++) {
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

