#include <Arduino.h>
#include <SPI.h>
#include "PCF8574.h"
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>  
#include <OneWire.h>

////////////////////////////////////////////////////////////////////// PCF8574 Adresse
PCF8574 pcf8574(0x20);
//PCF8574 pcf8574_2(0x21);
/*
PINOUT Wemos Expander
D1 SCL
D2 SDA
Pinout -> https://bre.is/9H5nddhG
*/

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void wifi_setup                 ();
void callback                   (char* topic, byte* payload, unsigned int length);
void reconnect                  ();
void temp_messen                ();
void mqbstt_callback_aufrufen   ();

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_mqtt_callback = 0; // Spannung Messen
unsigned long interval_mqtt_callback = 100; 

unsigned long previousMillis_abfrage_tuerklingel = 0; // Spannung Messen
unsigned long interval_abfrage_tuerklingel = 500; 

/////////////////////////////////////////////////////////////////////////// PIN zuweisen
int tuerklingel =  D7;

/////////////////////////////////////////////////////////////////////////// Kartendaten
const char* kartenID = "Vorbau_Relaiskarte";

WiFiClient espClient;
PubSubClient client(espClient);

/////////////////////////////////////////////////////////////////////////// Connect to the WiFi
const char* ssid = "GuggenbergerLinux";
const char* password = "Isabelle2014samira";
const char* mqtt_server = "192.168.150.1";
//const char* mqtt_topic_a = "192.168.178.222";


/////////////////////////////////////////////////////////////////////////// SETUP - Wifi
void wifi_setup() {
// Verbindung zum WiFI aufbauen

  Serial.print("Verbindung zu SSID -> ");
  Serial.println(ssid);

  IPAddress ip(192, 168, 5, 27);
	IPAddress dns(192, 168, 1, 1);  
	IPAddress subnet(255, 255, 0, 0);
	IPAddress gateway(192, 168, 1, 1);
	WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Erfolgreich mit dem WiFi verbunden!");
  Serial.print("DHCP Adresse der Relaikarte : ");
  Serial.println(WiFi.localIP());
  Serial.print("ID der Relaiskarte: ");
  Serial.println(kartenID);
}


//////////////////////////////////////////////////////////////////////////////////   SETUP
void setup() {

/////////////////////////////////////////////////////////////////////////// Konfig Relais
  pcf8574.pinMode(P0, OUTPUT);
  pcf8574.pinMode(P1, OUTPUT);
  pcf8574.pinMode(P2, OUTPUT);
  pcf8574.pinMode(P3, OUTPUT);
  pcf8574.pinMode(P4, OUTPUT);
  pcf8574.pinMode(P5, OUTPUT);
  pcf8574.pinMode(P6, OUTPUT);
  pcf8574.pinMode(P7, OUTPUT);        
  pcf8574.begin();


/////////////////////////////////////////////////////////////////////////// Relais OFF
pcf8574.digitalWrite(P0, !LOW);
pcf8574.digitalWrite(P1, !LOW);
pcf8574.digitalWrite(P2, !LOW);
pcf8574.digitalWrite(P3, !LOW);
pcf8574.digitalWrite(P4, !LOW);
pcf8574.digitalWrite(P5, !LOW);
pcf8574.digitalWrite(P6, !LOW);
pcf8574.digitalWrite(P7, !LOW);

// Sturmschutzschalter init
pinMode(tuerklingel, INPUT);


// MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

// Serielle Kommunikation starten
  Serial.begin(115200);

//*********************************************** Wifi Setup
wifi_setup();

}

/////////////////////////////////////////////////////////////////////////// Türklingel abfragen 
void abfragen_tuerklingel() {

  // Schalter abfragen
  	while( digitalRead(tuerklingel) == 1 ) //while the button is pressed
      {
        //blink
        Serial.println("Türklingel gedrückt");
        client.publish("Haustuer/taster", "1");
        delay(2000);
        client.publish("Haustuer/taster", "0");
      } 

}

/////////////////////////////////////////////////////////////////////////// mqtt Callback aufrufen
void mqtt_callback_aufrufen() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

//////////////////////////////////////////////////////////////////////////////////   LOOP
void loop() {

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ mqtt Callback aufrufen
  if (millis() - previousMillis_mqtt_callback> interval_mqtt_callback) {
      previousMillis_mqtt_callback= millis(); 
      // Prüfen der Panelenspannung
      Serial.println("mqtt Callback aufrufen");
      mqtt_callback_aufrufen();
    } 

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Tuerklingel abfragen
  if (millis() - previousMillis_abfrage_tuerklingel> interval_abfrage_tuerklingel) {
      previousMillis_abfrage_tuerklingel= millis(); 
      // Prüfen der Panelenspannung
      Serial.println("Tuerklingel abfragen");
      abfragen_tuerklingel();
    }     
  
}

//////////////////////////////////////////////////////////////////////////////////   reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Verbindung zu mqtt IP: ");
    Serial.print(mqtt_server);
    Serial.println("");
    // Create a random client ID
    String clientId = "RK-WiFi-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Verbunden ....");
      // ... and resubscribe
   
      client.subscribe("Vorbau/Relaiskarte/IN/1");     
      client.subscribe("Vorbau/Relaiskarte/IN/2");     
      client.subscribe("Vorbau/Relaiskarte/IN/3");     
      client.subscribe("Vorbau/Relaiskarte/IN/4");     
      client.subscribe("Vorbau/Relaiskarte/IN/5");     
      client.subscribe("Vorbau/Relaiskarte/IN/6");     
      client.subscribe("Vorbau/Relaiskarte/IN/7");     
      client.subscribe("Vorbau/Relaiskarte/IN/8");     
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////   callback
void callback(char* topic, byte* payload, unsigned int length) {


    if (strcmp(topic,"Vorbau/Relaiskarte/IN/1")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 1 -> AN");
                 pcf8574.digitalWrite(P0, !HIGH);
                 //client.publish("Vorbau/Relaiskarte/IN/1","on");
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 1 -> AUS");
                 pcf8574.digitalWrite(P0, !LOW);
                 //client.publish("Vorbau/Relaiskarte/IN/1","off");
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/2")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 2 -> AN");
                 pcf8574.digitalWrite(P1, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 2 -> AUS");
                 pcf8574.digitalWrite(P1, !LOW);
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/3")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 3 -> AN");
                 pcf8574.digitalWrite(P2, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 3 -> AUS");
                 pcf8574.digitalWrite(P2, !LOW);
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/4")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 4 -> AN");
                 pcf8574.digitalWrite(P3, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 4 -> AUS");
                 pcf8574.digitalWrite(P3, !LOW);
                delay(100);
              }
      }       
          if (strcmp(topic,"Vorbau/Relaiskarte/IN/5")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 5 -> AN");
                 pcf8574.digitalWrite(P4, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 5 -> AUS");
                 pcf8574.digitalWrite(P4, !LOW);
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/6")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 6 -> AN");
                 pcf8574.digitalWrite(P5, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 6 -> AUS");
                 pcf8574.digitalWrite(P5, !LOW);
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/7")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 7 -> AN");
                 pcf8574.digitalWrite(P6, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 7 -> AUS");
                 pcf8574.digitalWrite(P6, !LOW);
                delay(100);
              }
      } 

          if (strcmp(topic,"Vorbau/Relaiskarte/IN/8")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("Vorbau_Relais Kanal 8 -> AN");
                 pcf8574.digitalWrite(P7, !HIGH);
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("Vorbau_Relais Kanal 8 -> AUS");
                 pcf8574.digitalWrite(P7, !LOW);
                delay(100);
              }
      } 
}