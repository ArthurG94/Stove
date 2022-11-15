#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#ifndef MICRONOVA_DEF
#define MICRONOVA_DEV ;

class Micronova
{

public:
	Micronova();

	char readRAM(char address, bool *error, char *retry);
	char readEEPROM(char address, bool *error, char *retry);
	void writeRAM(char address, char value, bool *error, char *retry);
	void writeEEPROM(char address, char value, bool *error, char *retry);

	void setTemperatureSetPoint(char temp, bool *error, char *retry);
	void setState(char state, bool *error, char *retry);
	void setFlamePower(char power, bool *error, char *retry);
	char getTemperatureSetPoint(bool *error, char *retry);
	char getRoomTemperature(bool *error, char *retry);
	char getFumeTemperature(bool *error, char *retry);
	char getState(bool *error, char *retry);
	char getRun(bool *error, char *retry);
	char getFlamePower(bool *error, char *retry);
	int getTimeBeforeCleanUp(bool *error, char *retry);

	enum StateEnum
	{
		STATE_OFF = 0,
		STATE_STARTING = 1,
		STATE_PELLET_LOADING = 2,
		STATE_IGNITION = 3,
		STATE_WORK = 4,
		STATE_BRAZIER_CLEANING = 5,
		STATE_FINAL_CLEANING = 6,
		STATE_STANDBY = 7,
		STATE_PELLET_MISSING_ALARM = 8,
		STATE_IGNITION_FAILURE_ALARM = 9,
		STATE_UNKNOWN_ALARM = 10
	};

	String stateString[11] = {"OFF", "STARTING", "PELLET_LOADING", "IGNITION", "WORK", "BRAZIER_CLEANING", "FINAL_CLEANING", "STANDBY", "PELLET_MISSING_ALARM", "IGNITION_FAILURE_ALARM", "UNKNOWN_ALARM"};

private:
	int const RETRY_COUNT = 10;
	SoftwareSerial serial;
	void createWriteBuffer(char *buffer, char type, char address, char value);
	void createReadBuffer(char *buffer, char type, char address);
	void get_ReadRAM_Buffer(char *buffer, char address);
	void get_ReadEEPROM_Buffer(char *buffer, char address);
	void get_WriteRAM_Buffer(char *buffer, char address, char value);
	void get_WriteEEPROM_Buffer(char *buffer, char address, char value);
	int writeAndRead(char *writeBuffer, char writeSize, char *readBuffer, char readSize);
};

#endif
