//============================================================================
// Name        : PorteroInteligenteRasp.cpp
// Author      : julian
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <serie/serie.h>
#include <common/common.h>

#include <wiringSerial.h>
using namespace std;
void procesarDato(void* vec, int cant);
int main() {
	serie myserie;
	int fd=myserie.init(procesarDato);
	char vec[]={'H','o','l','a','J','u','u'};

	myserie.prepare_pack(vec,7);
while (true){
	cout << "!!!Hello soa" << fd<<" "<<endl; // prints !!!Hello World!!!
	usleep(4000000);

}
	cout << "!!!Hello soa" << fd<<" "<<endl; // prints !!!Hello World!!!
	return 0;
}

void procesarDato(void* vec, int cant){
	cout << "Hello SOAAA!!!" << endl;
}
