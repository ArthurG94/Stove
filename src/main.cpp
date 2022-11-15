#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include <Micronova.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

#define MQTT_SERVER "192.168.1.101"
#define MQTT_PORT 1883

Micronova micronova;
WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char *topic, byte *payload, unsigned int length)
{
	payload[length] = '\0'; 
	
	int value = String((char *)payload).toInt();
	
	if (String(topic) == "stove/temperatureSetPoint" && temperatureSetPoint != value)
	{
		Serial.println(topic);
		Serial.println(value);
		micronova.setTemperatureSetPoint(value, &error, &retry);
	}
	if (String(topic) == "stove/flamePower" && flamePower != value)
	{
		Serial.println(topic);
		Serial.println(value);
		micronova.setFlamePower(value, &error, &retry);
	}
	if (String(topic) == "stove/additionalFan")
	{
		Serial.println(topic);
		Serial.println(value);
		additionalFanStatus = value;
	}
}

void mqtt_publish(String topic, int value)
{
	client.publish(topic.c_str(), String(value).c_str());
}

void setup()
{
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

	client.setServer(MQTT_SERVER, MQTT_PORT);
	client.setCallback(callback); // Déclaration de la fonction de souscription
}

// setTemperatureSetPoint
// setState
// setFlamePower

void loop()
{

	if (!client.connected())
	{
		if (!client.connect("stove"))
		{
			delay(1000);
		}
		else
		{
			client.subscribe("stove/temperatureSetPoint");
			client.subscribe("stove/flamePower");
			client.subscribe("stove/additionalFan");
		}
	}
	else
	{

		if (millis() - oldMilli > 1000)
		{ // top 1s
			oldMilli = millis();
			sec = (sec + 1) % 240;
		}

		if ((sec % 20) == 0 && sec != oldSec)
		{ // 20s
			state = micronova.getState(&error, &retry);
			mqtt_publish("stove/state", state);
			timeBeforeCleanUp = micronova.getTimeBeforeCleanUp(&error, &retry);
			mqtt_publish("stove/timeBeforeCleanUp", timeBeforeCleanUp);
			mqtt_publish("stove/additionalFan", additionalFanStatus);
		}

		if ((sec % 60) == 0 && sec != oldSec)
		{ // 1min
			fumeTemperature = micronova.getFumeTemperature(&error, &retry);
			mqtt_publish("stove/fumeTemperature", fumeTemperature);
		}

		if ((sec % 120) == 0 && sec != oldSec)
		{ // 2min
			roomTemperature = micronova.getRoomTemperature(&error, &retry);
			mqtt_publish("stove/roomTemperature", roomTemperature);
			temperatureSetPoint = micronova.getTemperatureSetPoint(&error, &retry);
			mqtt_publish("stove/temperatureSetPoint", temperatureSetPoint);
			flamePower = micronova.getFlamePower(&error, &retry);
			mqtt_publish("stove/flamePower", flamePower);
		}

		oldSec = sec;
	}

	digitalWrite(13, additionalFanStatus && (state >= 3 && state < 7 && fumeTemperature >= 80));
	client.loop();
}
