
#include <SPI.h>
#include <MFRC522.h>
#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------------- Definicion de Pines -------------------------------//
#define RST_PIN         9           // Pin reset RFID.
#define SS_PIN          10          // Slave Select pin RFID.
#define BUZZER          2           // pin Buzzer.
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

void refreshBuzzer(void);
void printDisplay(const char *line1, const char *line2);
void printMsg(int msgNumber);

void setup() {

  pinMode(BUZZER, OUTPUT);
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

}

void loop() {

  if (checkCardInField == true) {
    checkCardInField = false;
    if ( rfid.PICC_IsNewCardPresent()) {
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

  if (refreshDisplay == true) {
    refreshDisplay = false;
    printMsg(msgNumber);
  }
}

//----------------------------------------------------------------------------//
void printMsg(int msgNumber) {
  static int lastMsgNumber = 99;

  if (lastMsgNumber != msgNumber) {
    lastMsgNumber = msgNumber;
    switch (msgNumber) {
      case 0: printDisplay("Portero", "    Inteligente");
        break;
      case 1: printDisplay("Acerque su      ", "     Tarjeta ...");
        break;
      case 2: printDisplay("Tarjeta         ", "     en campo   ");
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
//----------------------------------------------------------------------------//
