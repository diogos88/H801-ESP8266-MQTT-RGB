#include "secrets.h"

#include <ESP8266WiFi.h> //tested with v 1.0.0
#include <PubSubClient.h> //tested with v2.6.0
#include <Ticker.h>

void LED_RED();
void LED_GREEN();
void LED_BLUE();
void LED_W1();
void LED_W2();
void change_LED();
int convertToInt(char upper, char lower);

#define PWM_VALUE 63
int gamma_table[PWM_VALUE + 1] = {
   0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
   11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
   61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
   279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
};

// RGB FET
#define redPIN    15
#define greenPIN  13
#define bluePIN   12

// W FET
#define w1PIN     14
#define w2PIN     4

// onbaord green LED D1
#define onboardGreenLEDPIN    5
// onbaord red LED D2
#define onboardRedLEDPIN   1

// note 
// TX GPIO2 @Serial1 (Serial1 ONE)
// RX GPIO3 @Serial1    

#define LEDoff digitalWrite(onboardGreenLEDPIN,HIGH)
#define LEDon digitalWrite(onboardGreenLEDPIN,LOW)

#define LED2off digitalWrite(onboardRedLEDPIN,HIGH)
#define LED2on digitalWrite(onboardRedLEDPIN,LOW)

int led_delay_red = 0;
int led_delay_green = 0;
int led_delay_blue = 0;
int led_delay_w1 = 0;
int led_delay_w2 = 0;
#define time_at_colour 1000 
unsigned long TIME_LED_RED = 0;
unsigned long TIME_LED_GREEN = 0;
unsigned long TIME_LED_BLUE = 0;
unsigned long TIME_LED_W1 = 0;
unsigned long TIME_LED_W2 = 0;
int RED, GREEN, BLUE, W1, W2;
int RED_A = 0;
int GREEN_A = 0;
int BLUE_A = 0;
int W1_A = 0;
int W2_A = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

Ticker ticker;

void Tick()
{
	if (millis() - TIME_LED_RED >= led_delay_red) {
		TIME_LED_RED = millis();
		LED_RED();
	}

	if (millis() - TIME_LED_GREEN >= led_delay_green) {
		TIME_LED_GREEN = millis();
		LED_GREEN();
	}

	if (millis() - TIME_LED_BLUE >= led_delay_blue) {
		TIME_LED_BLUE = millis();
		LED_BLUE();
	}

	if (millis() - TIME_LED_W1 >= led_delay_w1) {
		TIME_LED_W1 = millis();
		LED_W1();
	}

	if (millis() - TIME_LED_W2 >= led_delay_w2) {
		TIME_LED_W2 = millis();
		LED_W2();
	}
}

void setup_wifi() {

	delay(10);
	// We start by connecting to a WiFi network
	Serial1.println();
	Serial1.print("Connecting to ");
	Serial1.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial1.print(".");
	}

	Serial1.println("");
	Serial1.println("WiFi connected");
	Serial1.println("IP address: ");
	Serial1.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial1.print("Message arrived [");
	Serial1.print(topic);
	Serial1.print("] ");
	for (int i = 0; i < length; i++) {
		Serial1.print((char)payload[i]);
	}
	Serial1.println();

	if ((String(topic).indexOf(mqtt_rgb_topic) != -1))
	{
		long r = convertToInt((char)payload[0], (char)payload[1]); //red
		long g = convertToInt((char)payload[2], payload[3]); //green
		long b = convertToInt((char)payload[4], (char)payload[5]); //blue

		int red = map(r, 0, 255, 0, PWM_VALUE);
		red = constrain(red, 0, PWM_VALUE);
		int green = map(g, 0, 255, 0, PWM_VALUE);
		green = constrain(green, 0, PWM_VALUE);
		int blue = map(b, 0, 255, 0, PWM_VALUE);
		blue = constrain(blue, 0, PWM_VALUE);

		RED = gamma_table[red];
		GREEN = gamma_table[green];
		BLUE = gamma_table[blue];
		W1 = 0;
		W2 = 0;
	}
	else if ((String(topic).indexOf(mqtt_w1_topic) != -1))
	{
		long w = convertToInt(payload[0], payload[1]); //w1
		int w1 = map(w, 0, 255, 0, 1023);
		w1 = constrain(w1, 0, 1023);

		RED = 0;
		GREEN = 0;
		BLUE = 0;
		W1 = w1;
		W2 = 0;
	}
	else if ((String(topic).indexOf(mqtt_w2_topic) != -1))
	{
		long w = convertToInt(payload[0], payload[1]); //w2
		int w2 = map(w, 0, 255, 0, 1023);
		w2 = constrain(w2, 0, 1023);

		RED = 0;
		GREEN = 0;
		BLUE = 0;
		W1 = 0;
		W2 = w2;
	}

	change_LED();
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial1.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
			Serial1.println("connected");
			// Once connected, publish an announcement...
			client.publish(mqtt_command_topic, "Device Connected");
			// ... and resubscribe
			client.subscribe(strcat(mqtt_command_topic, mqtt_rgb_topic));
			client.subscribe(strcat(mqtt_command_topic, mqtt_w1_topic));
			client.subscribe(strcat(mqtt_command_topic, mqtt_w2_topic));
		}
		else {
			Serial1.print("failed, rc=");
			Serial1.print(client.state());
			Serial1.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void change_LED()
{
	int diff_red = abs(RED - RED_A);
	if (diff_red > 0)
	{
		led_delay_red = time_at_colour / abs(RED - RED_A);
	}
	else
	{
		led_delay_red = time_at_colour / 1023;
	}

	int diff_green = abs(GREEN - GREEN_A);
	if (diff_green > 0)
	{
		led_delay_green = time_at_colour / abs(GREEN - GREEN_A);
	}
	else
	{
		led_delay_green = time_at_colour / 1023;
	}

	int diff_blue = abs(BLUE - BLUE_A);
	if (diff_blue > 0)
	{
		led_delay_blue = time_at_colour / abs(BLUE - BLUE_A);
	}
	else
	{
		led_delay_blue = time_at_colour / 1023;
	}

	int diff_w1 = abs(W1 - W1_A);
	if (diff_w1 > 0)
	{
		led_delay_w1 = time_at_colour / abs(W1 - W1_A);
	}
	else
	{
		led_delay_w1 = time_at_colour / 1023;
	}

	int diff_w2 = abs(W2 - W2_A);
	if (diff_w2 > 0)
	{
		led_delay_w2 = time_at_colour / abs(W2 - W2_A);
	}
	else
	{
		led_delay_w2 = time_at_colour / 1023;
	}
}

void LED_RED()
{
	if (RED != RED_A) {
		if (RED_A > RED) RED_A = RED_A - 1;
		if (RED_A < RED) RED_A++;
		analogWrite(redPIN, RED_A);
	}
}

void LED_GREEN()
{
	if (GREEN != GREEN_A) {
		if (GREEN_A > GREEN) GREEN_A = GREEN_A - 1;
		if (GREEN_A < GREEN) GREEN_A++;
		analogWrite(greenPIN, GREEN_A);
	}
}

void LED_BLUE()
{
	if (BLUE != BLUE_A) {
		if (BLUE_A > BLUE) BLUE_A = BLUE_A - 1;
		if (BLUE_A < BLUE) BLUE_A++;
		analogWrite(bluePIN, BLUE_A);
	}
}

void LED_W1()
{
	if (W1 != W1_A) {
		if (W1_A > W1) W1_A = W1_A - 1;
		if (W1_A < W1) W1_A++;
		analogWrite(w1PIN, W1_A);
	}
}

void LED_W2()
{
	if (W2 != W2_A) {
		if (W2_A > W2) W2_A = W2_A - 1;
		if (W2_A < W2) W2_A++;
		analogWrite(w2PIN, W2_A);
	}
}

int convertToInt(char upper, char lower)
{
	int uVal = (int)upper;
	int lVal = (int)lower;
	uVal = uVal > 64 ? uVal - 55 : uVal - 48;
	uVal = uVal << 4;
	lVal = lVal > 64 ? lVal - 55 : lVal - 48;
	return uVal + lVal;
}

void setup()
{
	pinMode(onboardGreenLEDPIN, OUTPUT);
	pinMode(onboardRedLEDPIN, OUTPUT);

	pinMode(redPIN, OUTPUT);
	pinMode(greenPIN, OUTPUT);
	pinMode(bluePIN, OUTPUT);
	pinMode(w1PIN, OUTPUT);
	pinMode(w2PIN, OUTPUT);

	// Setup console
	Serial1.begin(115200);
	delay(10);
	Serial1.println();
	Serial1.println();

	setup_wifi();
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);

	LEDon;

	while (WiFi.status() != WL_CONNECTED) {

		LED2off;
		delay(500);
		Serial1.print(".");
		LED2on;
	}

	Serial1.println("");

	Serial1.println("WiFi connected");
	Serial1.println("IP address: ");
	Serial1.println(WiFi.localIP());

	Serial1.println("");

	ticker.attach(0.01, Tick);
}

void loop()
{
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}