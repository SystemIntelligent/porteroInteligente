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
extern bool disponibleRec;
extern char vectorRecibido[];
using namespace std;
void procesarDato(void* vec, int cant);

Comunicacion myserie;
sqlite handlerSQlite("acceso.db");

int main() {

	char comand='1';
	char datos[1]={0xFF};
	//myserie.enviarDatos(comand,1,datos);
    handlerSQlite.init();
	cout << "\nSQLite: " << handlerSQlite.selectFirstItem("name","authorized_persons") << endl;

}



void procesarDato(void* vec, int cant){
	disponibleRec=TRUE;

	cout << "Hello SOAAA!!!" << endl;
}
