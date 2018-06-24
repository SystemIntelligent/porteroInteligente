/*
 * common.h
 *
 *  Created on: 28 may. 2018
 *      Author: lpastor
 */

#ifndef SRC_COMMON_COMMON_H_
#define SRC_COMMON_COMMON_H_
using namespace std;

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <sqlite3.h>

#define TRUE	1
#define FALSE	0

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

typedef void (*CallBackMain_t)(int,int,void *);
#endif /* SRC_COMMON_COMMON_H_ */


