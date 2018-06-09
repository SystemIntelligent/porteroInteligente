/*
 * serie.cpp
 *
 *  Created on: 27 may. 2018
 *      Author: julian
 */
#include "serie.h"
int fd = 0;
char tmp[3];	//YO
char vectorRecibido[TAMANO_MAXIMO];
int tamanoRecibido;

char* serie::checksum(const char * p, int length) {
	int resultado=0;	//YO

	for (int idx = 0; idx < length; idx++) {
		resultado = resultado ^ p[idx];
	}
	if (resultado <= 15) {
		if (resultado <= 9) {
			tmp[0] = '0';
			tmp[1] = resultado + 48;
			tmp[2] = '\0';
		} else {
			tmp[0] = '0';
			tmp[1] = resultado + 87;
			tmp[2] = '\0';

		}
	} else {

		int resto;
		int cociente;
		char numHexa[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
				'a', 'b', 'c', 'd', 'e', 'f' };

		resto = resultado % 16;
		cociente = resultado / 16;
		tmp[2] = '\0';
		tmp[1] = numHexa[resto];
		tmp[0] = numHexa[cociente];

	}
//		printf("%c ",tmp[0]);
//		printf("%c ",tmp[1]);
	return tmp;
}

void* serie::eventoSerial(void *arg) {
	argThread *argTh = (argThread*) arg;
//		volatile int fileDescriptor = argTh->fd;
		volatile CallBack_t pfuncion = argTh->pfuncion;
	while (true) {
		int cant = serialDataAvail(fd);

		if (cant > 0 ) {
			tamanoRecibido = cant;
			int flagInit = FALSE;
			int i;
			while(cant>0){
					char aux = serialGetchar(fd);
					if (aux == INICIO_DAT) {
						flagInit = TRUE;
						printf("Recibio: ");
						i=0;
					} else {
						if (flagInit == TRUE) {
							if (aux == FIN_DAT) {
								//Terminó
								flagInit = FALSE;
								puts("");
								if (pfuncion != NULL){
									pfuncion(vectorRecibido, i);
								}
							} else {
								vectorRecibido[i] = aux;
								printf(" %02x", vectorRecibido[i]);
								i++;
							}
						}
					}
					cant--;
			}
		}

		usleep(5000);
	}

}

serie::serie(){
}

int serie::init(CallBack_t funcion, int velocidad) {
	if (fd != 0)
		return FALSE;

	if ((fd = serialOpen("/dev/ttyAMA0", velocidad)) < 0) {
		fprintf(stderr, "No pudo abrirse el puerto serial: %s\n",
				strerror(errno));
		return FALSE;
	} else {
		if (wiringPiSetup() == -1) {
			fprintf(stdout, "No pudo iniciarse wiringPi: %s\n",
					strerror(errno));
			return FALSE;
		}
	}
	argumentosTh.fd=fd;
	argumentosTh.pfuncion=funcion;
	pthread_t threadId;
	int err = pthread_create(&threadId, NULL, eventoSerial, &argumentosTh);//eventoSerial es donde se fija si hay un mensaje.
	if (err != 0) {
		printf("Error al crear Hilo.");
		fd = 0;
		return FALSE;
	}
	return fd;
}

int serie::prepare_pack(char * vec, int tam) {
	if (fd == 0)
		return FALSE;
	char vectorTransmit[tam + 7];

	//hace CHecksum
	char * tmp = checksum(vec, tam);
	//ojo '/0'
	sprintf((char*) vectorTransmit, "%c%c%s%c%s%c%c", INICIO_DAT, SEPARADOR,
			vec, SEPARADOR, tmp, SEPARADOR, FIN_DAT);

	serialPrintf(fd, vectorTransmit);
//	cout<<"Envioo"<<endl;
// visualización de lo enviado.
//	int i=0;
//	for(i=0;i<(tam + 7);i++){
//		printf("%x ",vectorTransmit[i]);
//	}
//	puts(" ");
	return TRUE;
}
