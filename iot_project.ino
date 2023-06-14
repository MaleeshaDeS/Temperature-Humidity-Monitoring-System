// If the code is being compiled for an ESP8266 board, include the ESP8266WiFi library.
// Otherwise, include the WiFi library for ESP32 boards.
#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #else
 #include <WiFi.h>
#endif

#include "DHTesp.h" //DHTesp library for interacting with the DHT11 temperature and humidity sensor.
#include <ArduinoJson.h> //ArduinoJson library for working with JSON data.
#include <PubSubClient.h> //PubSubClient library for publishing and subscribing to MQTT messages.
#include <WiFiClientSecure.h> //WiFiClientSecure library for establishing a secure WiFi connection.
#include <Servo.h> //Servo library for controlling the servo motor


#define DHTpin 5   // Set the pin number for the DHT11 sensor.
#define LED D2 //// Set the pin number for an LED.

DHTesp dht;// Create an instance of the DHTesp library.
Servo s1;// Create an instance of the Servo library.



/****** WiFi Connection Details *******/
//SHOULD BE EDITED ACCORDING TO WIFI CREDENTIALS
const char* ssid = "Dialog 4G KW";
const char* password = "jk#2413383";

/******* MQTT Broker Connection Details *******/
const char* mqtt_server = "4429533909a4442199abb8de800fdd7b.s2.eu.hivemq.cloud";
const char* mqtt_username = "SMMS_IOT";
const char* mqtt_password = "smms@IOT23";
const int mqtt_port = 8883;


/**** Secure WiFi Connectivity Initialisation *****/
WiFiClientSecure espClient;

/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient client(espClient);

// Set a buffer size for MQTT messages.
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


/************* Connect to WiFi ***********/
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#define humidity_topic "humid"
#define temperature_C "Ctemp"
#define temperature_F "Ftemp"


/***** Call back Method for Receiving MQTT messages and Switching LED ****/

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

}


/**** Method for Publishing MQTT Messages **********/
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}


/**** Application Initialisation Function******/
void setup() {

  dht.setup(14, DHTesp::DHT11); // GPIO14//Set up DHT11 sensor
  pinMode(LED, OUTPUT);
  Serial.begin(9600);

  s1.attach(5);
  s1.attach(5, 500, 2400);
  s1.write(0);  
  
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca); // Enable this line and the the "certificate" code for secure connection
  #endif

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

/******** Main Function *************/
void loop() {

  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

// Read temperature and humidity data from the DHT11 sensor
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float Ctemp = dht.getTemperature();
  
// Converting celcius to farenheit
  float FTemp = ((Ctemp * 9/5) + 32);

// Calculate the heat index using temperature and humidity values
  float HI;
  // HI = -42.379 + 2.04901523*T + 10.14333127*RH - .22475541*T*RH - .00683783*T*T - .05481717*RH*RH + .00122874*T*T*RH + .00085282*T*RH*RH - .00000199*T*T*RH*RH
  HI = -42.379 + 2.04901523*FTemp + 10.14333127*humidity - .22475541*FTemp*humidity - .00683783*FTemp*FTemp - .05481717*humidity*humidity + .00122874*FTemp*FTemp*humidity + .00085282*FTemp*humidity*humidity - .00000199*FTemp*FTemp*humidity*humidity;
  
  
// Control LED bulb to indicate heat index level (safe=light off, unsafe=light on)
  if(HI>95) {
    digitalWrite(LED, HIGH);    
  }else{
    digitalWrite(LED, LOW);        
  }   

// Create a JSON message containing temperature, humidity, and heat index data
  DynamicJsonDocument doc(1024);

  doc["deviceId"] = "NodeMCU";
  doc["siteId"] = "My Demo Lab";
  doc["humid"] = humidity;
  doc["Ctemp"] = Ctemp;
  doc["Ftemp"] = FTemp;
  doc["HI"] = HI;

  char mqtt_message[128];
  serializeJson(doc, mqtt_message);

// Publish the JSON message to an MQTT topic
  publishMessage("esp8266_data", mqtt_message, true);

// Move the servo motor axel based on HI value
  if(HI < 80){ // check if heat index is lesser than 80
      s1.write(166);
      delay(1000);  
    } else if(HI < 90){// check if heat index is lesser than 90
      s1.write(139);    
      delay(100);
    } else if(HI < 103){ // check if heat index is lesser than 103
      s1.write(107);  
      delay(1000);
    } else if(HI < 124){// check if heat index is lesser than 124
      s1.write(90);     
      delay(1000);
    } else if(HI > 125){// check if heat index is greater than 80
      s1.write(16);    
      delay(1000);
    } 
 
// Wait for 5 seconds before running the loop again
  delay(5000);

}