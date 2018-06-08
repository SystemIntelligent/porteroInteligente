/*
 * Comunicacion.cpp
 *
 *  Created on: 4 jun. 2018
 *      Author: julian
 */

#include <comunicacion/Comunicacion.h>

char * Comunicacion::getChecksumFromReceivedPackage(const char *package, int length) {
  static char checksum[SIZE_CHECKSUM];
  int index = 3;
  for (int idx = 0; idx < SIZE_CHECKSUM - 1; idx++) {
    checksum[idx] = package[length-index + idx];
  }
  checksum[2] = '\0';
  return checksum;
}

bool Comunicacion::compareChecksum(const char *checksumA, const char *checksumB) {
  bool result = true;

  for (int idx = 0; idx < SIZE_CHECKSUM; idx++) {
    if (checksumA[idx] != checksumB[idx]) {
      result = false;
    }
  }
  return result;
}

/*
 *	Desarmo la carga util del paquete, devuelvo el comando
 *	y copio en data los datos que viene el paquete.
 */
int Comunicacion::disarmPayLoad(const char *payLoad, int length, char *data) {

  char command[3];
  memcpy(command, payLoad, 2 * sizeof(char));
  command[2] = '\0';

  int cmd = atoi(command);
  if (cmd > 0 && cmd < 9) {
    //if (cmd == 6) {  // unico comando proveniente desde la raspberry con datos;
    memcpy (data, payLoad + 3, (length - 3)*sizeof(char));
    data[length - 3] = '\0';
    // }
    return cmd;
  }
  return ERROR_CODE; //retCode error;
}

/*
 * Me quedo con la carga util del paquete.
 * Mi paquete no tiene los caracteres de inicio, y fin.
 */
char *Comunicacion::disarmPackage(const char *package, int length) {
  static char payLoad[TAMANO_MAXIMO];
  memcpy(payLoad, package + 1, (length - 5 )*sizeof(char)); // +1 primer delimitador(1); -5 para no copiar
  // primer delimitador(1), delimitador antes del checksum(1) el checksum(2) el limitador despues del checksum(1) y el caracter de fin de paquete(1).
  payLoad[length - 5] = '\0';
  return payLoad;
}
bool Comunicacion::validatePackage(const char *package, int length) {

  return compareChecksum(getChecksumFromReceivedPackage(package, length), serie::checksum(package + 1, length - 5));
  // + 1 para saltearme el delimitador start_character y el -5 es (primer delimitador + 1 delimitador antes del checksum +  2 caracteres del checksum + ultimo delimitador
  // + caracter de fin.

}




Comunicacion::Comunicacion():serie() {
	serie::init(procesarEntrada,VELOCIDAD);
}

void Comunicacion::procesarEntrada(void* vec,int tam){
	char* vector=(char*) vec;
	char  payLoad[TAMANO_MAXIMO];
	char comando;
//	cout<<"RECIBOooo:"<<endl;
	//valido si el checkSum es correcto
	if(Comunicacion::validatePackage(vector,tam)==true){
		// El checksum es valido
		cout<<"> Checksum valido!!"<<endl;
		strcpy(payLoad, disarmPackage(vector, tam));
		cout<<"> PayLoad Disarmed: "<<endl;
		char data[50];
		int command = disarmPayLoad(payLoad, strlen(payLoad), data);
		if (command != -1) {
			cout<<"> Command: "<<command<<endl;
			cout<<"> Data: "<< data<<endl;
			if(comando==ACK_OPEN_DOOR){
				//HOLA
			}else if (comando==BUTTON_PRESSED) {
				//RELLENO
			}else if (comando==VALIDATE_CARD) {
				//RELLENAR
			}
		}
		else {
		  cout << "Comando invalido"<<endl;
		}
	}else cout << "CheckSum invalido"<<endl;
}

void Comunicacion::enviarDatos(char comando,int tam,char *dato){
	static char vec[TAMANO_MAXIMO];
	sprintf((char*) vec, "%c%c%s",comando, SEPARADOR, dato);

	serie::prepare_pack(vec,sizeof(vec));
}
Comunicacion::~Comunicacion() {
	// TODO Auto-generated destructor stub
}

