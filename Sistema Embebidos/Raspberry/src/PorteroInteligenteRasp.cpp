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
#include <pthread.h>

using namespace std;

static Comunicacion myserie;
static sqlite handlerSQlite("acceso.db");
void eventoMain(int comando, int tam, void *vec);

bool openDoorFirebases = false;
bool closeDoorFirebases = true;
bool openMicFirebases = false;

void *firebases(void *arg) {

	FILE *ptr_file;
	char buffer[100];
	char oldBuffer[100];
	char cmd[150];
	char data[100];

	while (true) {
		system("python /home/pi/iot/firebases.py > retFirebases.out");
		ptr_file = fopen("retFirebases.out", "r");

		if (ptr_file) {

			fgets(buffer, sizeof(buffer), ptr_file);

			if (strcmp(buffer, oldBuffer) != 0) {
				strncpy(oldBuffer, buffer, sizeof(oldBuffer));

				if (buffer[0] == '1') {
					cout <<"\n> Firebases: Abrir Puerta" << endl;
					buffer[0] = '0';
					snprintf(cmd, sizeof(cmd),"python /home/pi/iot/firebases_sender.py %s", buffer);
					system(cmd);
					strncpy(data, "trash", sizeof(data));
					myserie.enviarDatos(OPEN_DOOR + 48, 2, data);
				}
				if (buffer[2] == '1') {
					cout << "\n> Firebases: Cerrar Puerta" << endl;
					buffer[2] = '0';
					snprintf(cmd, sizeof(cmd),"python /home/pi/iot/firebases_sender.py %s", buffer);
					system(cmd);
					strncpy(data, "trash", sizeof(data));
					myserie.enviarDatos(CLOSE_DOOR + 48, 2, data);
				}
				if (buffer[4] == '1') {
					cout << "\n> Firebases: abrir mic" << endl;
					buffer[4] = '0';
					snprintf(cmd, sizeof(cmd),"python /home/pi/iot/firebases_sender.py %s", buffer);
					system(cmd);
				}

				fclose(ptr_file);
			}
		}
		usleep(10000);
	}
}

int main() {
	myserie.init(eventoMain);
	handlerSQlite.init();
	pthread_t threadId;
	int err = pthread_create(&threadId, NULL, firebases, NULL); //eventoSerial es donde se fija si hay un mensaje.
	if (err != 0) {
		printf("Error al crear hilo firebases.");
		return -1;
	}

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
