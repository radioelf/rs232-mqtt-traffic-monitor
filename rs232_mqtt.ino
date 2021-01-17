/*
Leemos el puerto serie y enviamos lo recibido a través de MQTT
los topic recibidos los enviamos por el puerto serie


1/300=   0,00333333-> 3333,33 microsegundos por bit
1/600=   0,00166666-> 1666,66 microsegundos por bit
1/1200=  0,00083333-> 833,33 microsegundos por bit
1/2400=  0,00041666-> 416.66 microsegundos por bit
1/4800=  0,00020833-> 208.33 microsegundos por bit
1/9600=  0,00010416-> 104,16 microsegundos por bit
1/19200= 0,00005208-> 52,08 microsegundos por bit
1/38400= 0,00002604-> 26,04 microsegundos por bit
1/57600= 0,00001736-> 17,35 microsegundos por bit
1/115200=0,00000868-> 8,68 microsegundos por bit
1/230400=0,00000434-> 4,34 microsegundos por bit
1/921600=0,00000108-> 1,08 microsegundos por bit

Trama de RX (depende de la configuración), a 115200Bps (máximo):
Máxima longitud trama uart (1-8-1-2) 1 bit start + 8 bits datos  + 1 bit paridad + 2 bit stop-> 12bits*8,68 = 104,16us
Minima longitud trama uart (1-5-1)   1 bit start + 5 bits datos + 1 bit stop-> 7bits*8,68 = 60,76us

Period máximo RX 128 caracteres 104,16us * 128 = 13,33248ms


Creative Commons License Disclaimer
UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING, LICENSOR OFFERS THE WORK AS-IS
AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK, EXPRESS, IMPLIED,
STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION, WARRANTIES OF TITLE, MERCHANTIBILITY,
FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS,
ACCURACY, OR THE PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME JURISDICTIONS
DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO SUCH EXCLUSION MAY NOT APPLY TO YOU.
EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU
ON ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES
ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK, EVEN IF LICENSOR HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES.
http://creativecommons.org/licenses/by-sa/3.0/

  Author: Radioelf  http://radioelf.blogspot.com.es/
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network-mqtt.
const char* ssid = "wifiusuario";
const char* password = "wificontrasenya";
const char* mqtt_server = "192.168.x.xxx";
#define mqtt_port 1883
#define MQTT_USER "usuario"
#define MQTT_PASSWORD "contrasenya"
#define MQTT_SERIAL_SUBSCRIBE "/serialdata"
#define MQTT_SERIAL_PUBLISH_CH "/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "/serialdata/rx"

#define bps 115200

String clientId = "ESP-";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Conectando a: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);                                                       // Connect to network
    while (WiFi.status() != WL_CONNECTED) {                                           
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    uint8_t mac[6];    
    WiFi.macAddress(mac);
    String Mac = String(String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16));
    clientId = clientId + Mac;                                                        // We created a unique id with the and part of the MAC
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("ID: ");
    Serial.println(clientId);
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
}

void reconnect() {
  while (!client.connected()) {                                                       
    Serial.println("Intentando la conexión MQTT...");
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.print("MQTT conectado a: ");
      Serial.println (clientId);
      String topic = (clientId + "/status");                                         // Once connected, it publishes the online status..
      client.publish((char*) topic.c_str(), "online");
      // we subscribe to /esp8266/serialdata/rx
      const String SubTopic = clientId  + String (MQTT_SERIAL_SUBSCRIBE) + "/#";
      client.subscribe(SubTopic.c_str(), 0);
      Serial.println("Subcrito  a topic: " + String (clientId + MQTT_SERIAL_SUBSCRIBE));
    } else {
      Serial.print("ERROR, rc=");
      Serial.print(client.state());
      Serial.println(" se reintentara en 5 segundos");
      delay(5000);
    }
  }
}

// RX topic to RS232
void callback(char* topic, byte *payload, unsigned int length) {
    if (String(topic) ==  String (clientId + MQTT_SERIAL_RECEIVER_CH)) {
      Serial.write(payload, length);
    } else {    
      Serial.println("Mensaje del broker");
      Serial.print("topic: ");
      Serial.print(topic);
      Serial.print("  payload: ");
      for (unsigned int i = 0; i < length; i++) {Serial.print((char)payload[i]);}
      Serial.println();
    }
}

void setup() {
  Serial.begin(bps);
  Serial.setTimeout(500);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}


void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  String topic = (clientId + MQTT_SERIAL_PUBLISH_CH);
  client.publish((char*) topic.c_str(), serialData);
}

void readSerial() {
static char buffer [128];
static uint8_t index = 0;
  if (Serial.available()){ 
    char c = Serial.read();  
    if (c != '\n'){                                   // NO end of line?
      if (index < 128){                               // Hardware buffer limit? (may be higher)
        buffer[index] = c; 
        index++;
      }
      else{
        buffer[index] = '\0';                         // Ending the chain
        index = 0;
        publishSerialData(buffer);
        return;
      }
    }
    else {                                            // If fine linea
      buffer[index] = '\0';
      index = 0;
      publishSerialData(buffer);
    }
  }
}

void loop() {
  client.loop();
  readSerial();
}
