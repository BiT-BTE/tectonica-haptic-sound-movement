/*

   i_mBODY LAB
   Vibran v: 1.0
   by: Joaku De Sotavento

*/



#include "WiFi.h"
#include <SPI.h>
#include <OSCMessage.h>

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
// Options
int update_rate = 16;

#include "vibrant_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h and don't publish it in any place
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

IPAddress local_IP(192, 168, 1, 102);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(0, 0, 0, 0); //optional
IPAddress secondaryDNS(0, 0, 0, 0); //optional

unsigned int localPort = 8890; // local port to listen for OSC packets


const int pot = A2;

const int drive1 = 13;
const int drive2 = 12;
const int drive3 = 27;
const int drive4 = 33;
const int drive5 = 32;

const int stopVib = 0;

const int periodOn = 100;
const int periodOff = 50;

const int longDelay = 10000;

// FOR THE INTERFACE
const int ledWIFI = 14;


// use first channel of 16 channels (started from zero)
// tjose channels are for the PWM on each motor
#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1
#define LEDC_CHANNEL_2     2
#define LEDC_CHANNEL_3     3
#define LEDC_CHANNEL_4     4


// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

int brightness = 0;    // how bright the LED is


// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}


///////////////////// delay sin delay

int period = 60;
unsigned long time_now = 0;


//////// cosas de los potenciometros

//int intensity = 13;

// the setup function runs once when you press reset or power the board
void setup()
{
  // FOR THE INTERFACE
  pinMode(ledWIFI, OUTPUT);

  // SERIAL COMMUNICATION
  Serial.begin(115200);
//  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
//    Serial.println("STA Failed to configure");
//  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  digitalWrite(ledWIFI, HIGH);

  Udp.begin(localPort);

  /////////////////////////// Yhis part is also for the PWM
  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcSetup(LEDC_CHANNEL_3, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcSetup(LEDC_CHANNEL_4, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);

  ledcAttachPin(drive1, LEDC_CHANNEL_0);
  ledcAttachPin(drive2, LEDC_CHANNEL_1);
  ledcAttachPin(drive3, LEDC_CHANNEL_2);
  ledcAttachPin(drive4, LEDC_CHANNEL_3);
  ledcAttachPin(drive5, LEDC_CHANNEL_4);
}

void ledtoggle(OSCMessage &msg) {
  switch (msg.getInt(0)) {
    case 0:
      Serial.println("Apagado");
      break;
    case 1:
      Serial.println("Prendido");
      vibrant();
      break;
  }
}

void receiveMessage() {
  OSCMessage inmsg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      inmsg.fill(Udp.read());
    }
    if (!inmsg.hasError()) {
      inmsg.dispatch("/led", ledtoggle);
    }
    //else { auto error = inmsg.getError(); }
  }
}

void loop() {
  receiveMessage();
  delay(update_rate);
}


void vibrant() {
  int intensity = map(analogRead(pot), 0, 4095, 0, 255);
  Serial.print("El potenciometro da: ");
  Serial.println(intensity);

  ledcAnalogWrite(LEDC_CHANNEL_0, intensity / 4.5);
  delay(periodOn);

  ledcAnalogWrite(LEDC_CHANNEL_0, intensity / 4.5);
  ledcAnalogWrite(LEDC_CHANNEL_1, intensity / 4);
  delay(periodOn);

  ledcAnalogWrite(LEDC_CHANNEL_0, stopVib);
  ledcAnalogWrite(LEDC_CHANNEL_1, intensity / 4);
  ledcAnalogWrite(LEDC_CHANNEL_2, intensity / 3);
  delay(periodOn);

  ledcAnalogWrite(LEDC_CHANNEL_1, stopVib);
  ledcAnalogWrite(LEDC_CHANNEL_2, intensity / 3);
  ledcAnalogWrite(LEDC_CHANNEL_3, intensity / 2);
  delay(periodOn);

  ledcAnalogWrite(LEDC_CHANNEL_2, stopVib);
  ledcAnalogWrite(LEDC_CHANNEL_3, intensity / 2);
  ledcAnalogWrite(LEDC_CHANNEL_4, intensity);
  delay(periodOn);


  ledcAnalogWrite(LEDC_CHANNEL_3, stopVib);
  ledcAnalogWrite(LEDC_CHANNEL_4, intensity / 1.5);
  delay(periodOn);

  ledcAnalogWrite(LEDC_CHANNEL_4, stopVib);
  delay(periodOn);
}
