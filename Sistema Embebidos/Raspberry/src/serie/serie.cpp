/*
 * serie.cpp
 *
 *  Created on: 27 may. 2018
 *      Author: julian
 */
#include "serie.h"
//#include <errno.h>
#include <stdio.h>
#include <common/common.h>

#include <wiringSerial.h>

int fd=0;
char vectorRecibido[256];
int tamanoRecibido;
int disponibleRec=TRUE;

static char* checksum(char * p){
	int resultado;	//YO
	char tmp[3];	//YO
	char * pi= p;		//YO
	while (*p!='*' && *p != '\0' && (p - pi) < 100) {
				resultado = resultado ^ *p;
				p++;
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

			return tmp;
}

static void *eventoSerial(void *arg) {
	serie::CallBack_t pfuncion = (serie::CallBack_t) arg;
	while (true){
		int cant=serialDataAvail (fd);
		if(cant!=0 || disponibleRec==TRUE){
			tamanoRecibido=cant;
			int flagInit=FALSE,flagFin=FALSE;
			for (int i = 0; i < cant; i++) {
				char aux= serialGetchar(fd);
				if(aux==INICIO_DAT && flagFin==FALSE){
					flagInit=TRUE;
				}else{
					if(flagInit==TRUE){
						if(aux==FIN_DAT){
							//Terminó
							flagInit=FALSE;
							if (pfuncion != NULL)
								pfuncion(vectorRecibido,tamanoRecibido);

						}else{
							vectorRecibido[i]=aux;
							printf(" -> %3d", vectorRecibido[i]);
						}
					}
				}
			}
		}
	}

}

int serie::init(CallBack_t funcion) {
	if(fd!=0)
		return FALSE;

	if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0){
	    fprintf (stderr, "No pudo abrirse el puerto serial: %s\n", strerror (errno)) ;
	    return FALSE ;
	}else{
		if (wiringPiSetup () == -1){
		    fprintf (stdout, "No pudo iniciarse wiringPi: %s\n", strerror (errno)) ;
		    return FALSE ;
		}
	}
	pthread_t threadId;
	int err = pthread_create(&threadId, NULL, eventoSerial, &funcion);	//eventoSerial es donde se fija si hay un mensaje.
	if (err != 0) {
		printf("Error al crear Hilo.");
		fd=0;
		return FALSE;
	}
	return TRUE;
}


int serie::prepare_pack(char * vec, int tam){
	if(fd!=0)
		return FALSE;
	char vectorTransmit[tam+7];
//	vectorTransmit[0]=INICIO_DAT;
//	vectorTransmit[1]=SEPARADOR;
//
//	vectorTransmit[(tam-3)]=SEPARADOR;
//
//	vectorTransmit[tam]=SEPARADOR;
//	vectorTransmit[(tam+1)]=FIN_DAT;

	//hace CHecksum
	char * tmp =checksum(vec);
	//ojo '/0'
	sprintf((char*)vectorTransmit, "%c%c%s%c%02s%c%c",INICIO_DAT,SEPARADOR,vec,SEPARADOR,tmp,SEPARADOR,FIN_DAT);
//		vectorTransmit[(tam-2)]=tmp[0];//Parte alta checkSum
//		vectorTransmit[(tam-1)]=tmp[1];//Parte baja checkSum
	serialPrintf(fd, vectorTransmit);

	return TRUE;
}


