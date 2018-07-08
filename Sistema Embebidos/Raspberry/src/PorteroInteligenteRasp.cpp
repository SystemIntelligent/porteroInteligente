//============================================================================
// Name        : PorteroInteligenteRasp.cpp
// Author      : julian
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <serie/serie.h>
#include <comunicacion/Comunicacion.h>
#include <common/common.h>
#include <sqlite/sqlite.h>
using namespace std;

static Comunicacion myserie;
static sqlite handlerSQlite("acceso.db");
void eventoMain(int comando, int tam, void *vec);

int main() {
	myserie.init(eventoMain);
	handlerSQlite.init();

	while (true) {

		usleep(2000000);
	}

}

void eventoMain(int comando, int tam, void *vec) {

	cout << "> Command: " << comando << endl;
	cout << "> Data: ";
	char *vecChar = (char*) vec;
	for (int idx = 0; idx < tam; idx++) {
		printf("%c", vecChar[idx]);
	}
	cout << "\n";

	if (comando == ACK_OPEN_DOOR) {
		//HOLA
	} else if (comando == BUTTON_PRESSED) {
		//RELLENO
	} else if (comando == VALIDATE_CARD) {
		char data[30];
		memcpy(data, vec, 2 * sizeof(uint8_t));
		data[2] = '\0';
		char *returnCode = handlerSQlite.selectItem("card_id", data, "Name",
				"authorized_persons");
		if (returnCode != NULL) {
			strcpy(data, returnCode);
			cout << "Acceso Permitido: " << returnCode << "\n" << endl;
			myserie.enviarDatos(CARD_VALID + 48, strlen(data), data);
		} else if (returnCode == NULL) {
			cout << "Acceso Denegado: Tarjeta invalida" << "\n" << endl;
			strncpy(data, "trash", sizeof(data));
			myserie.enviarDatos(CARD_NOT_VALID + 48, 2, data);
		}

	}

}
