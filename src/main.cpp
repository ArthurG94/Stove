#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <Micronova.h>
#include <uri/UriBraces.h>
#include <SoftwareSerial.h>


ESP8266WebServer server(80);
Micronova micronova;

int additionalFanStatus = 0;

void setup()
{
	Serial.begin(9600);

	WiFiManager wifiManager;

	pinMode(13, OUTPUT); //D0 Ventillateur 
	pinMode(0, OUTPUT); //D3 3.3V constant 
	digitalWrite(0,HIGH);
	//wifiManager.resetSettings();

	if (!wifiManager.autoConnect("Poile à granule AP", "123456789")) {
		WiFi.hostname("poile_a_granule");
		ESP.reset();
		delay(1000);
	}


	server.on(UriBraces("/{}"), []() {
		
		String dataName = server.pathArg(0);
		bool error;
		char retry;
		
		if (server.method() == HTTP_GET) {
		
			int value;

			Serial.println("GET:"+dataName);

			if (dataName == "run") {
				value = micronova.getRun(&error, &retry);
			} else if (dataName == "temperatureSetpoint") {
				value = micronova.getTemperatureSetpoint(&error, &retry);
			} else if (dataName == "roomTemperature") {
				value = micronova.getRoomTemperature(&error, &retry);
			} else if (dataName == "fumeTemperature") {
				value = micronova.getFumeTemperature(&error, &retry);
			} else if (dataName == "state") {
				value = micronova.getState(&error, &retry);
			} else if (dataName == "timeBeforeCleanup") {
				value = micronova.getTimeBeforeCleanUp(&error, &retry);
			} else if (dataName == "flamePower") {
				value = micronova.getFlamePower(&error, &retry);
			}  else if (dataName == "externalFan") {
				value = additionalFanStatus;
			} else {
				server.send(503, "text/plain", "");
			}

			if(error){
				server.send(500, "text/plain", String((int)retry));
			}else{
				server.send(200, "text/plain", String(value));
			}

		} else if (server.method() == HTTP_POST) {
			char value = server.arg("plain").toInt();

			Serial.println("POST:"+dataName+" "+String((int)value));

			if (dataName == "temperatureSetpoint") {
				micronova.setTemperatureSetPoint(value, &error, &retry);
			}
			// else if (dataName == "state") {
			// 	micronova.setState(value, &error, &retry);
			// }
			else if (dataName == "additionalFan") {
				additionalFanStatus = value;
			} else if (dataName == "flamePower") {
				micronova.setFlamePower(value, &error, &retry);
			} else {
				server.send(503, "text/plain", "Wrong data name");
			}
		
			if(error){
				server.send(500, "text/plain", String((int)retry));
			}else{
				server.send(200, "text/plain", "");
			}
		} else {
			server.send(405, "text/plain", "Method Not Allowed");
		}
	});
	server.begin();

}


void loop() {
		server.handleClient();
	digitalWrite(13, additionalFanStatus);
}
