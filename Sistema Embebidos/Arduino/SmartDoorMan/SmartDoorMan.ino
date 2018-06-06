#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ---------------------- Definicion de Pines -------------------------------//
#define RST_PIN          9           // Pin reset RFID.
#define SS_PIN           10          // Slave Select pin RFID.
#define BUZZER           2           // pin Buzzer.
#define PULSADOR         3           // pin Pulsador.
#define SERVO_MOTOR      4           // servo motor signal.
//---------------------------------------------------------------------------//

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
LiquidCrystal_I2C lcd(0x3f, 16, 2); // Display Set 2 lineas por 16 caracteres.
SoftwareSerial SoftSerial(7, 8); // RX, TX


#define DEBUG

#ifdef  DEBUG
/*
#define DELIMITER_CHARACTER '|'
#define START_CHARACTER 'S'
#define END_CHARACTER 'F'
*/
#define DELIMITER_CHARACTER (int8_t)0xAE
#define START_CHARACTER (int8_t)0x91
#define END_CHARACTER (int8_t)0x92

#define SERIAL_PRINT(x,y)        SoftSerial.print(x);SoftSerial.println(y);
#endif

#ifndef DEBUG
#define DELIMITER_CHARACTER (int8_t)0xAE
#define START_CHARACTER (int8_t)0x91
#define END_CHARACTER (int8_t)0x92
#define SERIAL_PRINT(x,y)
#endif

#define ERROR_CODE              (-1)
#define ERROR_BAD_CHECKSUM      (-2)
#define CLOSE                    5
#define OPEN                     65

#define PACKAGE_FORMAT "%c%c%s%c%s%c%c" //start character + delimiter character + payLoad + delimiter character + checksum + delimiter character + end character.
#define SIZE_PAYLOAD     40
#define SIZE_PACKAGE     SIZE_PAYLOAD + 7
#define SIZE_CHECKSUM    3

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
  MSG_OPEN_DOOR,
  MSG_CLOSE_DOOR,
};

enum commands {
  OPEN_DOOR = 1,                 // comando recibido desde las raspberry.
  ACK_OPEN_DOOR,                 // ack enviado dede arduino a raspberry.
  BUTTON_PRESSED,                // comando enviado (cuando se pulsa el timbre) desde arduino a raspberry.
  ACK_BUTTON_PRESSED,            // ack recibido desde la raspberry.
  VALIDATE_CARD,                 // comando enviado (cuando se requiere validar una tarjeta) desde arduino a raspberry.
  CARD_VALID,                    // comando recibido desde la raspberry.
  CARD_NOT_VALID,                // comando recibido desde la raspberry.
  CLOSE_DOOR                     // comando recibido desde la raspberry.
};


bool   checkCardInField = false;
bool   refreshDisplay = false;
int    countAux0Timer = 0;    // contador auxiliar del timer usado para verificacion de tarjeta en campo.
int    countAux1Timer = 0;    // contador auxiliar del timer usado para refrescar display;
int    beepMode = 0;
int    msgNumber;
String receivedPackage = "";
bool   packageComplete = false;
int    servoPos = CLOSE;
bool   closeDoor = false;

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
int   disarmPayLoad(const char *payLoad, int length, char *data);
int   proccesPackage(String package, int length);

//----------------------------------------------------------------------------------------//

void setup() {
  pinMode(4, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(PULSADOR, INPUT_PULLUP);
  SoftSerial.begin(9600);
  Serial.begin(9600);

  lcd.init();                           // inicializacion del display
  lcd.backlight();                      // backLigth on.

  SPI.begin();
  rfid.PCD_Init();

  for (byte idx = 0; idx < 6; idx++) {  // Se setea la key que traen por defecto
    key.keyByte[idx] = 0xFF;            // de las tarjetas midfire.
  }


  noInterrupts ();        // deshabilitar todas las interrupciones
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 65487;          // temporizador de precarga 16MHz / 8 / 2Mhz.
  TCCR1B = (1 << CS11);   // 8 prescaler.
  TIMSK1 = (1 << TOIE1);  // habilitar la interrupción del desbordamiento del temporizador.
  interrupts ();          // activar todas las interrupciones.


  printMsg(MSG_WELCOME);
  SERIAL_PRINT(F("\n> Inicialización finalizada."), "");

  attachInterrupt(digitalPinToInterrupt(PULSADOR), ISR_HW, FALLING);
  receivedPackage.reserve(200);
  char msg[] = "01|Abrir Puerta";
  char msg1[] = "08|Cerrar Puerta";
  
  char pakage[SIZE_PACKAGE];
  char payLoad[SIZE_PAYLOAD];
  SERIAL_PRINT(F("\npayLoad: "), msg);
  strcpy(pakage, preparePackage(msg, strlen(msg)));
  SERIAL_PRINT(F("Package: "), pakage);

  SERIAL_PRINT(F("\npayLoad: "), msg1);
  strcpy(pakage, preparePackage(msg1, strlen(msg1)));
  SERIAL_PRINT(F("Package: "), pakage);

}

void loop() {

  if (closeDoor == true) {
    closeDoor = false;
    servoPos = CLOSE;
    printMsg(MSG_CLOSE_DOOR);
  }

  if (packageComplete) {
    packageComplete = false;
    int retCmd = proccesPackage(receivedPackage, receivedPackage.length());

    if (retCmd == ERROR_CODE) {
      SERIAL_PRINT("> Error unknown command !!", "");
    }
    else if (retCmd == ERROR_BAD_CHECKSUM) {
      SERIAL_PRINT("> Error bad checksum!!", "");
    }
    else {
      switch (retCmd) {
        case OPEN_DOOR:
          SERIAL_PRINT(F("> Command Open Door"), "");
          servoPos = OPEN;
          printMsg(MSG_OPEN_DOOR);
          break;

        case CLOSE_DOOR:
          SERIAL_PRINT(F("> Command Close Door"), "");
          servoPos = CLOSE;
          printMsg(MSG_CLOSE_DOOR);
          break;
        
        case CARD_VALID:
          SERIAL_PRINT(F("> Command Card Valid"), "");
          break;

        case CARD_NOT_VALID:
          SERIAL_PRINT(F("> Command Card not Valid"), "");
          break;

        case ACK_BUTTON_PRESSED:
          SERIAL_PRINT(F("> Command ACK button pressed"), "");
          break;

        case ACK_OPEN_DOOR:
          SERIAL_PRINT(F("> Command ACK Open Door"), "");
          break;
      }
    }
  }

  if (refreshDisplay == true) {
    printMsg(msgNumber);
    refreshDisplay = false;
  }

  if (checkCardInField == true) {
    checkCardInField = false;
    if (rfid.PICC_IsNewCardPresent()) {
      SERIAL_PRINT(F("\n> Tarjeta sobre el Lector"), "");
      msgNumber = MSG_CARD_IN_FIELD;
      beepMode = ONE_BEEP_SHORT;
      if ( rfid.PICC_ReadCardSerial()) {
        //rfid.PICC_DumpDetailsToSerial(&(rfid.uid));
        char cmd[] = "05";
        char payLoad[10];
        char pakage[50];
        snprintf(payLoad,sizeof(payLoad),"%s%c%s",cmd,(char)DELIMITER_CHARACTER,"04");
        SERIAL_PRINT(F("payLoad: "),pakage);
        strcpy(pakage, preparePackage(payLoad, strlen(payLoad)));
        SERIAL_PRINT(F("package: "),pakage);
        Serial.println(pakage); 
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
      case MSG_WELCOME:
        printDisplay("Portero", "    Inteligente");
        break;
      case MSG_APPROACH_CARD:
        printDisplay("Acerque su     ", "    Tarjeta ... ");
        break;
      case MSG_CARD_IN_FIELD:
        printDisplay("Tarjeta sobre  ", "      el lector ");
        break;
      case MSG_DING_DONG:
        printDisplay("  Ding         ", "     Dong ...   ");
        break;
      case MSG_OPEN_DOOR:
        printDisplay("Abriendo       ", "    Puerta ...  ");
        break;
      case MSG_CLOSE_DOOR:
        printDisplay("Cerrando       ", "    Puerta ...  ");
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
ISR (TIMER1_OVF_vect) {

  static int countRefresh = 0;
  static unsigned long countCloseDoor = 0;

  countRefresh++;
  countCloseDoor++;

  servoRefresh(servoPos);
  /*
    if (servoPos == OPEN) {            // Activo retardo de cierre de cerradura.
      countCloseDoor++;
      if (countCloseDoor >= 500000) {
        countCloseDoor = 0;
        closeDoor = true;
      }
    }
  */

  if (countRefresh >= 2083) {         // Tengo una interrupción de timer cada (1 / (16Mhz / 8)) * 48 = 24 uS ---> 24 uS * 2083 = 50 mS.
    countRefresh = 0;
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

  TCNT1 = 65487;                   // (1 / (16Mhz / 8)) * 48 = 24 us * 127 = 3mS ---> período de la señal PWM requerida por el servo motor.

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
  checksum[2] = '\0';
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
  payLoad[length - 7] = '\0';
  return payLoad;
}
//--------------------------------------------------------------------------//
int disarmPayLoad(const char *payLoad, int length, char *data) {

  char command[3];
  memcpy(command, payLoad, 2 * sizeof(char));
  command[2] = '\0';

  int cmd = atoi(command);
  if (cmd > 0 && cmd < 9) {
    //if (cmd == 6) {  // unico comando proveniente desde la raspberry con datos;
    memcpy (data, payLoad + 3, (length - 3)*sizeof(char));
    data[length - 3] = '\0';
    // }
    return cmd;
  }
  return ERROR_CODE; //retCode error;
}
//--------------------------------------------------------------------------//
int proccesPackage(String package, int length) {

  SERIAL_PRINT(F("\n> Recieved Package: "), package);
  char  recievedPack[SIZE_PACKAGE];
  char  payLoad[SIZE_PAYLOAD];
  package.toCharArray(recievedPack, sizeof(recievedPack));
  if (validatePackage(recievedPack, strlen(recievedPack))) {
    SERIAL_PRINT(F("> Checksum valid!!"), "");
    strcpy(payLoad, disarmPackage(recievedPack, strlen(recievedPack)));
    SERIAL_PRINT(F("> PayLoad Disarmed: "), payLoad);
    char data[50];
    int command = disarmPayLoad(payLoad, strlen(payLoad), data);
    if (command != -1) {
      SERIAL_PRINT(F("> Command: "), command);
      SERIAL_PRINT(F("> Data: "), data);
      return command;
    }
    else {
      return ERROR_CODE;
    }
  }
  else {
    return ERROR_BAD_CHECKSUM;
  }
}
//--------------------------------------------------------------------------//
void servoRefresh(int servoPos) {
  static int servoLast = 99;
  static int countServo = 0;
  countServo++;

  static bool block = false;
  static unsigned long blockCount = 0;


  if (servoLast != servoPos) {
    blockCount++;
    if (blockCount > 50000) {
      servoLast = servoPos;
      blockCount = 0;
      digitalWrite (SERVO_MOTOR, LOW);
    }
  }

  if (servoLast != servoPos) {
    if (countServo >= 128) {
      countServo = 0;
    }

    else if (countServo <= servoPos) {
      digitalWrite (SERVO_MOTOR, HIGH);
    }
    else
    {
      digitalWrite (SERVO_MOTOR, LOW);
    }
  }
}

