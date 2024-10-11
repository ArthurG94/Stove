#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <Micronova.h>
#include <uri/UriBraces.h>
#include <SoftwareSerial.h>

ESP8266WebServer server(80);
Micronova micronova;
WiFiClient espClient;

int additionalFanStatus = 1;

long oldMilli = 0;
char sec = 0;
char oldSec = 0;

bool error;
char retry;
char state;
int timeBeforeCleanUp;
char fumeTemperature;
char roomTemperature;
char temperatureSetPoint;
char flamePower;

void sendNoValue(bool error, byte retry)
{
	String json = "{\"error\":" + String(error) + ",\"retry\":" + String(retry) + "}";
	server.send(200, "application/json", json);
}
void sendBool(bool value, bool error, byte retry)
{
	String json = "{\"value\":" + String(value) + ",\"error\":" + String(error) + ",\"retry\":" + String(retry) + "}";
	server.send(200, "application/json", json);
}
void sendByte(byte value, bool error, byte retry)
{
	String json = "{\"value\":" + String((byte)value) + ",\"error\":" + String(error) + ",\"retry\":" + String(retry) + "}";
	server.send(200, "application/json", json);
}
void sendByteAndString(byte value, String str, bool error, byte retry)
{
	String json = "{\"value\":" + String((byte)value) + ",\"valueAsString\":\"" + str + "\",\"error\":" + String(error) + ",\"retry\":" + String(retry) + "}";
	server.send(200, "application/json", json);
}
void sendInt(int value, bool error, byte retry)
{
	String json = "{\"value\":" + String(value) + ",\"error\":" + String(error) + ",\"retry\":" + String(retry) + "}";
	server.send(200, "application/json", json);
}

void setup(){
	Serial.begin(9600);

	WiFiManager wifiManager;

	pinMode(13, OUTPUT); // D0 Ventillateur
	pinMode(0, OUTPUT);	 // D3 3.3V constant
	digitalWrite(0, HIGH);
	// wifiManager.resetSettings();

	if (!wifiManager.autoConnect("Poile à granule AP", "123456789"))
	{
		WiFi.hostname("poile_a_granule");
		ESP.reset();
		delay(1000);
	}

	server.on(UriBraces("/{}"), []()   {
		String dataName = server.pathArg(0);
		
		bool error;
		char retry;
		if (server.method() == HTTP_GET) {
			

			if(dataName == "temperature-set-point"){
				char value = micronova.getTemperatureSetPoint(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "room-temperature"){
				char value = micronova.getRoomTemperature(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "fume-temperature"){
				char value = micronova.getFumeTemperature(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "state"){
				char value = micronova.getState(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "run"){
				char value = micronova.getRun(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "flame-power"){
				char value = micronova.getFlamePower(&error, &retry);
				sendByte(value, error, retry);
			}
			else if(dataName == "time-before-clean-up"){
				char value = micronova.getTimeBeforeCleanUp(&error, &retry);
				sendInt(value, error, retry);
			} else {
				server.send(503, "text/plain", "Wrong data name");
			}
		} else if (server.method() == HTTP_POST){

			char value = server.arg("plain").toInt();
			
			if (dataName == "temperature-set-point"){
				micronova.setTemperatureSetPoint(value, &error, &retry);
				sendNoValue(error, retry);
			}
			else if (dataName == "state"){
				micronova.setState(value, &error, &retry);
				sendNoValue(error, retry);
			}
			else if (dataName == "flame-power"){
				micronova.setFlamePower(value, &error, &retry);
				sendNoValue(error, retry);
			}
			else if (dataName == "additional-fan-status"){
				additionalFanStatus = value;
				sendNoValue(error, retry);
			}
			else{
				server.send(503, "text/plain", "Wrong data name");
			}

		}  else {
			server.send(405, "text/plain", "Method Not Allowed");
		} });

	server.begin();
}

void loop(){

	if (millis() - oldMilli > 1000)	{ // top 1s
		oldMilli = millis();
		sec = (sec + 1) % 240;
	}

	if ((sec % 20) == 0 && sec != oldSec)	{ // 20s
		state = micronova.getState(&error, &retry);
		timeBeforeCleanUp = micronova.getTimeBeforeCleanUp(&error, &retry);
	}

	if ((sec % 60) == 0 && sec != oldSec)	{ // 1min
		fumeTemperature = micronova.getFumeTemperature(&error, &retry);
	}

	if ((sec % 120) == 0 && sec != oldSec)	{ // 2min
		roomTemperature = micronova.getRoomTemperature(&error, &retry);
		temperatureSetPoint = micronova.getTemperatureSetPoint(&error, &retry);
		flamePower = micronova.getFlamePower(&error, &retry);
	}

	oldSec = sec;

	digitalWrite(13, additionalFanStatus && (state >= 3 && state < 7 && fumeTemperature >= 80));

	server.handleClient();
}
