#include "Micronova.h"

Micronova::Micronova() : serial(5, 4)
{

	this->serial.begin(1200, SWSERIAL_8N2);
	this->serial.flush();
	this->serial.setTimeout(20);
	pinMode(2, OUTPUT);
}

/* -------------------------------------------------------------------------- */
/*                                   Private                                  */
/* -------------------------------------------------------------------------- */

void Micronova::createWriteBuffer(char *buffer, char type, char address, char value)
{
	char cs = (type + address + value) & 0xFF;
	buffer[0] = type;
	buffer[1] = address;
	buffer[2] = value;
	buffer[3] = cs;
}

void Micronova::createReadBuffer(char *buffer, char type, char address)
{
	buffer[0] = type;
	buffer[1] = address / 256;
	buffer[2] = address % 256;
}

void Micronova::get_ReadRAM_Buffer(char *buffer, char address)
{
	this->createReadBuffer(buffer, 0x00, address);
}
void Micronova::get_ReadEEPROM_Buffer(char *buffer, char address)
{
	this->createReadBuffer(buffer, 0x20, address);
}

void Micronova::get_WriteRAM_Buffer(char *buffer, char address, char value)
{
	this->createWriteBuffer(buffer, 0x80, address, value);
}
void Micronova::get_WriteEEPROM_Buffer(char *buffer, char address, char value)
{
	this->createWriteBuffer(buffer, 0xA0, address, value);
}

int Micronova::writeAndRead(char *writeBuffer, char writeSize, char *readBuffer, char readSize)
{

	digitalWrite(2, HIGH);
	this->serial.write(writeBuffer, writeSize);
	digitalWrite(2, LOW);
	delay(10);
	return this->serial.readBytes(readBuffer, readSize);
}

char Micronova::readRAM(char address, bool *error, char *retry)
{
	char writeBuffer[3];
	char readBuffer[4];
	memset(writeBuffer, 0, sizeof(writeBuffer));
	memset(readBuffer, 0, sizeof(readBuffer));

	this->get_ReadRAM_Buffer(writeBuffer, address);

	// On tente plusieur fois la lecture à moins que l'on ai reçu des données
	for (byte i = 0; i < this->RETRY_COUNT; i++)
	{
		*retry = i;
		if (this->writeAndRead(writeBuffer, 3, readBuffer, 2) > 0)
		{
			char value = readBuffer[1];
			char cs = (0x00 + address + value) & 0xFF;
			if (readBuffer[0] == cs)
			{
				*error = false;
				return value;
			}
		}
		*retry = i;
	}
	*error = true;
	return 0xFF;
}
char Micronova::readEEPROM(char address, bool *error, char *retry)
{
	char writeBuffer[3];
	char readBuffer[4];
	memset(writeBuffer, 0, sizeof(writeBuffer));
	memset(readBuffer, 0, sizeof(readBuffer));

	this->get_ReadEEPROM_Buffer(writeBuffer, address);

	// On tente plusieur fois la lecture à moins que l'on ai reçu des données
	for (byte i = 0; i < this->RETRY_COUNT; i++)
	{
		*retry = i;
		if (this->writeAndRead(writeBuffer, 3, readBuffer, 2) > 0)
		{
			char value = readBuffer[1];
			char cs = (0x20 + address + value) & 0xFF;
			if (readBuffer[0] == cs)
			{
				*error = false;
				return value;
			}
		}
	}
	*error = true;
	return 0xFF;
}

void Micronova::writeRAM(char address, char value, bool *error, char *retry)
{
	char writeBuffer[4];
	char readBuffer[2];
	memset(writeBuffer, 0, sizeof(writeBuffer));
	memset(readBuffer, 0, sizeof(readBuffer));

	this->get_WriteRAM_Buffer(writeBuffer, address, value);

	// On tente plusieur fois l'écriture à moins que l'on ai reçu des données
	for (byte i = 0; i < this->RETRY_COUNT; i++)
	{
		*retry = i;
		if (this->writeAndRead(writeBuffer, 4, readBuffer, 2) > 0)
		{
			if (readBuffer[0] == address && readBuffer[1] == value)
			{
				*error = true;
				return;
			}
		}
	}
	*error = false;
}
void Micronova::writeEEPROM(char address, char value, bool *error, char *retry)
{
	char writeBuffer[4];
	char readBuffer[2];
	memset(writeBuffer, 0, sizeof(writeBuffer));
	memset(readBuffer, 0, sizeof(readBuffer));

	this->get_WriteEEPROM_Buffer(writeBuffer, address, value);

	// On tente plusieur fois l'écriture à moins que l'on ai reçu des données
	for (byte i = 0; i < this->RETRY_COUNT; i++)
	{
		*retry = i;
		if (this->writeAndRead(writeBuffer, 4, readBuffer, 2) > 0)
		{
			if (readBuffer[0] == address && readBuffer[1] == value)
			{
				*error = true;
				return;
			}
		}
	}
	*error = false;
}

/* -------------------------------------------------------------------------- */
/*                                   Public                                   */
/* -------------------------------------------------------------------------- */

char Micronova::getRun(bool *error, char *retry)
{
	return this->readRAM(0, error, retry);
}

char Micronova::getTemperatureSetPoint(bool *error, char *retry)
{
	return this->readEEPROM(0x7D, error, retry) / 2;
}

char Micronova::getRoomTemperature(bool *error, char *retry)
{
	return this->readRAM(0x01, error, retry) / 2;
}
char Micronova::getFumeTemperature(bool *error, char *retry)
{
	return this->readRAM(0x5A, error, retry);
}
char Micronova::getState(bool *error, char *retry)
{
	return this->readRAM(0x21, error, retry);
}
char Micronova::getFlamePower(bool *error, char *retry)
{
	return this->readEEPROM(0x7F, error, retry);
}
int Micronova::getTimeBeforeCleanUp(bool *error, char *retry)
{
	bool r1Error = false;
	bool r2Error = false;
	char r1Retry = 0;
	char r2Retry = 0;

	char second = this->readRAM(0x31, &r1Error, &r1Retry);
	char minute = this->readRAM(0x32, &r2Error, &r2Retry);

	*error = r1Error || r2Error;
	*retry = r1Retry + r2Retry;
	return minute * 60 + second;
}

void Micronova::setTemperatureSetPoint(char temp, bool *error, char *retry)
{
	this->writeEEPROM(0x7D, temp * 2, error, retry);
}

void Micronova::setState(char state, bool *error, char *retry)
{
	if (state >= 0 && state < this->stateString->length())
	{
		this->writeRAM(0x21, state, error, retry);
	}
	else
	{
		*error = 1;
	}
}

void Micronova::setFlamePower(char power, bool *error, char *retry)
{
	this->writeEEPROM(0x7F, power, error, retry);
}
