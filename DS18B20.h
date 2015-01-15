#ifndef DS18B20_h
#define DS18B20_h

#include "Arduino.h"
#include <OneWire.h>

// Code famille 1-Wire du modèle de capteur
#define TYPE_DS18S20 0x10
#define TYPE_DS18B20 0x28
#define TYPE_DS1822  0x22
// OneWire commands
#define SKIP_ROM        0xCC  // Skip ROM command to address all devices on the bus simultaneously
#define STARTCONVO      0x44  // Initiate temperature conversion
#define READSCRATCH     0xBE  // Read scratchpad
#define WRITESCRATCH    0x4E  // Write to scratchpad
#define COPYSCRATCH     0x48  // Copy scratchpad to EEPROM

#define NB_MAX_DS18B20	6	//nb maximum de capteurs sur la ligne

class DS18B20{

public:
	DS18B20(OneWire* ow);
	
	byte init(byte resolution);
	void startConversion();
	float readTemperature(byte sensorIdx);
	void waitForConversion();
	byte getNb();
	void getAddrAsString(byte sensorIdx, char* output);
	
protected:
	void setResolution(byte sensorIdx, byte resolution);
	
private:
	OneWire* _oneWire;
	byte _nb;				//Nombre de capteurs ds18b20
	byte _resolution;			//resolution
	long _convStartTime;			//timestamp de démarrage de la dernière conversion
	byte _addresses [NB_MAX_DS18B20][8];	//Adresses des capteurs (6 max)
	float _temperatures [NB_MAX_DS18B20];	//Températures relevée à la précédente mesure

};

#endif
