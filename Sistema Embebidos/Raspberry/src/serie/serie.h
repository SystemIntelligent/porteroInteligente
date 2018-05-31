/*
 * serie.h
 *
 *  Created on: 27 may. 2018
 *      Author: julian
 */

#ifndef SERIE_SERIE_H_
#define SERIE_SERIE_H_

#define INICIO_DAT 0x91
#define FIN_DAT 0x92
#define SEPARADOR 0xAE
//exclusive or para hacer el checksum 2 BITS

class serie {

//private:

//	static int queryResult(void *nothing, int argc, char **argv, char **colName);

public:
	//tipo_devolución (*nombre)(tipo param1, tipo param2, …)
	typedef void (*CallBack_t)(void *,int);
	int init(CallBack_t funcion);
	int prepare_pack(char * vec, int tam);
};

#endif /* SERIE_SERIE_H_ */
