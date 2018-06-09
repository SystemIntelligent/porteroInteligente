/*
 * Comunicacion.h
 *
 *  Created on: 4 jun. 2018
 *      Author: julian
 */
#include <serie/serie.h>
#ifndef COMUNICACION_COMUNICACION_H_
#define COMUNICACION_COMUNICACION_H_
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
static char vectorRec[TAMANO_MAXIMO];

typedef struct {
						CallBackMain_t pfuncion;
			}argument;

class Comunicacion : public serie {
protected:
#define VELOCIDAD 9600
	static int commando;
	int commandoRec=999;
	static int disarmPayLoad(const char *payLoad, int length, char *data);
	static char *disarmPackage(const char *package, int length);
	static char* getChecksumFromReceivedPackage(const char *package, int length);
	static bool validatePackage(const char *package, int length);
	static bool compareChecksum(const char *checksumA, const char *checksumB);

public:
	static argument argFuncionn;
	Comunicacion();
	void init(CallBackMain_t funcion);

	static void procesarEntrada(void* vec ,int tam);
	static bool getHasNewMensege();
	static void changeNewMensege();
	static int getData(char *data, int* length);
	void enviarDatos(char comando,int tam,char *dato);
//	void enviar(char* vec,int tam);
	virtual ~Comunicacion();
};

#endif /* COMUNICACION_COMUNICACION_H_ */
