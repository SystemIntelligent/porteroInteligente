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

Comunicacion myserie;
sqlite handlerSQlite("acceso.db");

int main() {
	handlerSQlite.init();
	cout << "\nSQLite: " << handlerSQlite.selectFirstItem("name","authorized_persons") << endl;
	char comand='1';
	char datos[1]={0xFF};

	while(true){
	myserie.enviarDatos(comand,1,datos);
	usleep(2000000);
	}

}
