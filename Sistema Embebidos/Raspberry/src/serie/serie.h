/*
 * serie.h
 *
 *  Created on: 27 may. 2018
 *      Author: julian
 */
#include <stdio.h>
#include <common/common.h>
#include <wiringSerial.h>
#ifndef SERIE_SERIE_H_
#define SERIE_SERIE_H_
static volatile bool disponibleRec=true;
static pthread_mutex_t mutexSerie;
class serie {

public:
#define INICIO_DAT	0x91
#define FIN_DAT 	0x92
#define SEPARADOR	0xAE
#define SIZE_CHECKSUM    3
#define ERROR_CODE      (-1)

	//tipo_devolución (*nombre)(tipo param1, tipo param2, …)
	serie();
	typedef void (*CallBack_t)(void *,int);
	typedef struct {
		CallBack_t pfuncion;
		int fd;
	}argThread;
	argThread argumentosTh;
	static char* checksum(const char * p, int length);
	int init(CallBack_t funcion, int velocidad);
	int prepare_pack(char * vec, int tam);
protected:
	bool  validatePackage(const char *package, int length);
	bool  compareChecksum(const char *checksumA, const char *checksumB);
	static void* eventoSerial(void *arg);
};

#endif /* SERIE_SERIE_H_ */
