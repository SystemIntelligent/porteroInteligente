/*
 * Comunicacion.h
 *
 *  Created on: 4 jun. 2018
 *      Author: julian
 */
#include <serie/serie.h>
#include <common/common.h>

#ifndef COMUNICACION_COMUNICACION_H_
#define COMUNICACION_COMUNICACION_H_

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
