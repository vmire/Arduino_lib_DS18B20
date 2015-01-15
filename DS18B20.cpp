#include "Arduino.h"
#include "DS18B20.h"

DS18B20::DS18B20(OneWire* ow){
	_oneWire = ow;
	_nb = 0;
}

byte DS18B20::init(byte resolution){
	_resolution = resolution;
	_oneWire->reset_search();						 // On reset la recherche sur le bus 1-Wire
	for(_nb=0 ; _oneWire->search(_addresses[_nb]) ; _nb++){
		Serial.println();
		Serial.print("Addr ");
		for(int j=0;j<8;j++) {
			Serial.print(_addresses[_nb][j], HEX);
			Serial.print(" ");
		}
		Serial.println();
		if (OneWire::crc8(_addresses[_nb], 7) != _addresses[_nb][7]){	// Vérifie que l'adresse a été correctement reçue
			Serial.println("CRC Error sur l'adresse");
		}
		
		if (_addresses[_nb][0] != TYPE_DS18B20){			// Vérifie qu'il s'agit bien d'un DS18B20
			Serial.println("Le module n'est pas un DS18B20");
		}
	}
	if(_nb > 0){
		//Definition de la résolution
		for(byte j=0;j<_nb;j++){
			setResolution(j, resolution);
		}
		_oneWire->reset();
	}
	return _nb;
}

void DS18B20::setResolution(byte sensorIdx, byte resolution){
	if(resolution<9 || resolution>12) resolution = 12;
	byte resConf = ((resolution-9)<<5) | 0x1F;	//resolution sur les bits 6 et 7 du registre de configuration
	_oneWire->reset();
	_oneWire->select(_addresses[sensorIdx]);
	_oneWire->write(WRITESCRATCH);
	_oneWire->write(0);				//1st byte : TH
	_oneWire->write(0);				//2nd byte : TL
	_oneWire->write(resConf); 			//3rd byte : Configuration register
	_oneWire->reset();
	_oneWire->write(COPYSCRATCH);			//Enregistrement dans l'eeprom des valeurs transmises
}

void DS18B20::startConversion(){
	//Lancement de la mesure sur tous les capteurs
	for(byte j=0;j<_nb;j++){
		_oneWire->reset();
		_oneWire->select(_addresses[j]);
		_oneWire->write(STARTCONVO);
	}
	_convStartTime = millis();
}

/**
 * Attente que la dernière conversion soit terminée
 */
void DS18B20::waitForConversion(){
	int convDelay = (750>>(12-_resolution))+1;
	long tmp = millis()-_convStartTime;
	if(convDelay > tmp) delay(convDelay - tmp);
}

/**
 * Lecture de la température en degres celcius
 */
float DS18B20::readTemperature(byte sensorIdx){
	//Commande de lecture du scratchpad
	_oneWire->reset();
	_oneWire->select(_addresses[sensorIdx]);
	_oneWire->write(READSCRATCH);
	//Lecture du scratchpad
	byte tempLSB = _oneWire->read();	//0:scratchpad byte 0 : temp LSB
	byte tempMSB = _oneWire->read();	//1:scratchpad byte 1 : temp MSB
	/*
	_oneWire->read();			//2:High alarm temp
	_oneWire->read();			//3:Low	alarm temp
	byte conf = oneWire->read();		//4:Config
	_oneWire->read();			//5:internal use & crc
	_oneWire->read();			//6:store for crc
	_oneWire->read();			//7:store for crc
	_oneWire->read();			//8:scratchpad CRC
	_oneWire->reset();
	*/
	//conversion en celsius
	//int16_t rawTemperature = ((tempMSB << 8) + tempLSB) * 0.0625;
	int16_t rawTemperature = (((int16_t)tempMSB) << 8) | tempLSB;
	float temp = (float)rawTemperature * 0.0625;
	return temp;
}

byte DS18B20::getNb(){
	return _nb;
}

/**
 * Extrait l'adresse d'un capteur sous forme de string et la met dans un char*
 * nécessite un char[13] minimum
 */
void DS18B20::getAddrAsString(byte sensorIdx, char* output){
	//Sur les 8 bytes de l'adresse, le numéro de série est sur les 6 bytes du milieu (1:code famille, 8:crc)
	uint8_t k=0;
	for(uint8_t i=1;i<7;i++){
		byte tmp = _addresses[sensorIdx][i];
		output[k] = (int)(tmp>>4) + 0x30;
		if(output[k] > 0x39) output[k] +=7;
		k++;
		output[k] = (int)(tmp & 0x0f) + 0x30;
		if(output[k] > 0x39) output[k] +=7;
		k++;
	}
	output[12] = '\0';
}

