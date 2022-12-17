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

// Kartendaten
const char* kartenID = "Vorbau_Relaiskarte";

WiFiClient espClient;
PubSubClient client(espClient);

// Connect to the WiFi
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
/////////////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////////////// Relais OFF
pcf8574.digitalWrite(P0, !LOW);
pcf8574.digitalWrite(P1, !LOW);
pcf8574.digitalWrite(P2, !LOW);
pcf8574.digitalWrite(P3, !LOW);
pcf8574.digitalWrite(P4, !LOW);
pcf8574.digitalWrite(P5, !LOW);
pcf8574.digitalWrite(P6, !LOW);
pcf8574.digitalWrite(P7, !LOW);
/////////////////////////////////////////////////////////////////////////// 

// MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

// Serielle Kommunikation starten
  Serial.begin(115200);

//*********************************************** Wifi Setup
wifi_setup();

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
      client.subscribe("RK_WiFi_003/IN/A");
      client.subscribe("RK_WiFi_003/IN/B");
      client.subscribe("RK_WiFi_003/IN/C");
      client.subscribe("RK_WiFi_003/IN/D");
      client.subscribe("RK_WiFi_003/IN/E");
      client.subscribe("RK_WiFi_003/IN/F");
      client.subscribe("RK_WiFi_003/IN/G");
      client.subscribe("RK_WiFi_003/IN/H");          
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


    if (strcmp(topic,"RK_WiFi_003/IN/A")==0) {

        // Kanal A
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_A -> AN");
                 pcf8574.digitalWrite(P0, !HIGH);
                 client.publish("RK_WiFi_003/OUT/A","on");
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_A -> AUS");
                 pcf8574.digitalWrite(P0, !LOW);
                 client.publish("RK_WiFi_003/OUT/A","off");
                delay(100);
              }
      } 



    if (strcmp(topic,"RK_WiFi_003/IN/B")==0) {

        // Kanal B
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_B -> AN");
                 pcf8574.digitalWrite(P1, !HIGH);
                 client.publish("RK_WiFi_003/OUT/B","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_B -> AUS");
                  pcf8574.digitalWrite(P1, !LOW);
                 client.publish("RK_WiFi_003/OUT/B","off");
                delay(100);
              }
      }      


    if (strcmp(topic,"RK_WiFi_003/IN/C")==0) {

        // Kanal C
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_C -> AN");
                  pcf8574.digitalWrite(P2, !HIGH);
                 client.publish("RK_WiFi_003/OUT/C","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 //digitalWrite(relais_A, !LOW);
                 Serial.println("relais_C -> AUS");
                 pcf8574.digitalWrite(P2, !LOW);
                 client.publish("RK_WiFi_003/OUT/C","off");
                delay(100);
              }
      }


    if (strcmp(topic,"RK_WiFi_003/IN/D")==0) {

        // Kanal D
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_D -> AN");
                 pcf8574.digitalWrite(P3, !HIGH);
                 client.publish("RK_WiFi_003/OUT/D","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_D -> AUS");
                 pcf8574.digitalWrite(P3, !LOW);
                 client.publish("RK_WiFi_003/OUT/D","off");
                delay(100);
              }
      }

    if (strcmp(topic,"RK_WiFi_003/IN/E")==0) {

        // Kanal D
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_E -> AN");
                 pcf8574.digitalWrite(P4, !HIGH);
                 client.publish("RK_WiFi_003/OUT/E","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_E -> AUS");
                 pcf8574.digitalWrite(P4, !LOW);
                 client.publish("RK_WiFi_003/OUT/E","off");
                delay(100);
              }
      }      

    if (strcmp(topic,"RK_WiFi_003/IN/F")==0) {

        // Kanal D
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_F -> AN");
                 pcf8574.digitalWrite(P5, !HIGH);
                 client.publish("RK_WiFi_003/OUT/F","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_F -> AUS");
                 pcf8574.digitalWrite(P5, !LOW);
                 client.publish("RK_WiFi_003/OUT/F","off");
                delay(100);
              }
      }    

    if (strcmp(topic,"RK_WiFi_003/IN/G")==0) {

        // Kanal D
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_G -> AN");
                 pcf8574.digitalWrite(P6, !HIGH);
                 client.publish("RK_WiFi_003/OUT/G","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_G -> AUS");
                 pcf8574.digitalWrite(P6, !LOW);
                 client.publish("RK_WiFi_003/OUT/G","off");
                delay(100);
              }
      } 

    if (strcmp(topic,"RK_WiFi_003/IN/H")==0) {

        // Kanal D
        if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                 Serial.println("relais_H -> AN");
                 pcf8574.digitalWrite(P7, !HIGH);
                 client.publish("RK_WiFi_003/OUT/H","on");                 
                delay(100);
              }

        if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                 Serial.println("relais_H -> AUS");
                 pcf8574.digitalWrite(P7, !LOW);
                 client.publish("RK_WiFi_003/OUT/H","off");
                delay(100);
              }
      } 
}