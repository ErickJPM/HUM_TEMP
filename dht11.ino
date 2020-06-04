#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


////////////////punto de acceso wifi ///////////////////

#define WLAN_SSID  "Erick"
#define WLAN_PASS  "erick199"

//////////////////////ADAFRUIT.io configuracion ///////////

#define AIO_SERVER  "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define IO_USERNAME  "Erick99"
#define IO_KEY       "aio_ArKa63UDLjA9YFGLdZlgOgMxR1j8"  

//temperature libraries//
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2
#define ledPin 16


#define DHTTYPE DHT11
//

//INICIALIZAR SENSOR
DHT_Unified dht(DHTPIN, DHTTYPE);

DHT dht2(DHTPIN, DHTTYPE);

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client,AIO_SERVER,AIO_SERVERPORT,IO_USERNAME,IO_KEY);

Adafruit_MQTT_Publish temp_value= Adafruit_MQTT_Publish(&mqtt,IO_USERNAME "/feeds/TEMPERATURE",MQTT_QOS_1); 
Adafruit_MQTT_Publish humidity= Adafruit_MQTT_Publish(&mqtt,IO_USERNAME "/feeds/HUMIDITY",MQTT_QOS_1); 

Adafruit_MQTT_Subscribe ledcontrol = Adafruit_MQTT_Subscribe(&mqtt,IO_USERNAME "/feeds/LED_CONTROL",MQTT_QOS_1);
uint16_t ledvalue=0;

///notificacion
const char* resource = "https://maker.ifttt.com/trigger/notificacion/with/key/bHOg8QsihwicLNLQFA9dD5";
const char* resource2 = "https://maker.ifttt.com/trigger/email/with/key/bHOg8QsihwicLNLQFA9dD5";

const char* server = "maker.ifttt.com";

void MQTT_connect();
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print("conectando a...");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID,WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Wifi conectado Direccion IP");
  Serial.println("IP ADRESS: ");
  Serial.println(WiFi.localIP());

  //SETUP MQTT SUBSCRIPTION FOR ONOFF FEED//
  mqtt.subscribe(&ledcontrol);

  dht.begin();
  dht2.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

}
int previous=0;
int diferencia;
void loop() {
  MQTT_connect();
  float humedad = dht2.readHumidity();
  Serial.println("humedad:");
  Serial.println(humedad);
  int actual=millis();
  
  if(actual-previous>=10000){
    Serial.println("10 segundos");
    diferencia= actual-previous;
    previous=previous+10000;
  }
  if (diferencia<10000){
    primero();
  }
  else if(diferencia>=10000){
    primero();
    segundo();
    humidity.publish(humedad);
    diferencia=0;
  }
}



long long ledvalue;
////led
void primero(){
  Adafruit_MQTT_Subscribe *subscription;
  while((subscription=mqtt.readSubscription(200))){
    if (subscription==&ledcontrol){
      Serial.println("OBTENIENDO VALOR DE INTENSIDAD DEL LED" );
      ledvalue= atoi((char *)ledcontrol.lastread);
      Serial.println("valor de la intensidad:");
      Serial.println(ledvalue*2.55); 
      analogWrite(ledPin,ledvalue*2.55);
    }
  }
}


///temperatura
void segundo(){
  //publicar temperatura
  //delay(100);
  //sensors_event_t event;
  //dht.temperature().getEvent(&event);
  float event= dht2.readTemperature();
  temp_value.publish(event);
  
  Serial.println("Temperatura");
  Serial.println("°C");
  Serial.println(event);
  if(event>26){
    makeIFTTTRequestEmail();
    
  }

  
  if(event>25){
    makeIFTTTRequestNot();
  }
  
  if(isnan(event)){
    Serial.println("Error leyendo datos de temperatura");
  }
  else{
    Serial.println("Temperatura");
    Serial.println("°C");
    Serial.println(event);

    if(!temp_value.publish(event)){
      Serial.println("Error Publicando");
    }
    else{
      Serial.println("OK!");
    }
  }
  
}





void MQTT_connect() {
  int8_t ret;

  if(mqtt.connected()) {
    return;
  }

  Serial.print("Conectando a MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect())!=0){
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando conexion en 5 segundos...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    Serial.println("Retry "+ retries);
    if (retries == 0) {
      while(1);
    }
  }
  
  Serial.println("MQTT CONECTADO!");
}

void makeIFTTTRequestNot() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
     Serial.println("Failed to connect, going back to sleep");
  }
  Serial.print("Request resource: "); 
  Serial.println(resource);
  client.print(String("GET ") + resource + 
                  " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" + 
                  "Connection: close\r\n\r\n");

  
                  
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
     Serial.println("No response, going back to sleep");
  }
  
  Serial.println("\nclosing connection");
  client.stop();
}



void makeIFTTTRequestEmail() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
     Serial.println("Failed to connect, going back to sleep");
  }
  Serial.print("Request resource2: "); 
  Serial.println(resource);
  client.print(String("GET ") + resource2 + 
                  " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" + 
                  "Connection: close\r\n\r\n");
                  
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
     Serial.println("No response, going back to sleep");
  }
  
  Serial.println("\nclosing connection");
  client.stop();
}
