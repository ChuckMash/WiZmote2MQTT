#include "EspMQTTClient.h"

EspMQTTClient client(
  "",             // WiFi SSID
  "",             // WiFi Password
  "",             // MQTT Broker Server IP
  "",             // MQTT Username
  "",             // MQTT Password
  "wizmote2mqtt", // MQTT Client Name Prefix
  1883            // MQTT Port 
);

uint8_t allow[]={0x44,0x4f,0x8e}; // Allowed WiZmote bssid  prefix, can be extended to full bssid. 44:4f:83:xx:xx:xx
uint8_t fingerprint[10];          // Fingerprint of sender bssid + message id
char         bssid_[16];          // Holds sender bssid
uint8_t      holder[50];          // Data blob used as fingerprint history, must be sized in multiples of fingerpint. 50 = 5 fingerprints
char      client_id[26];          // Holds whole client_id wizmote2mqtt-xxxxxxxxxxxx
char     will_topic[34];          // Holds last will and testament topic
char            msg[256];         // Holds messages sent to serial and mqtt
unsigned long ledtime = millis(); // Activity indicator LED timer
bool         activity = LOW;      // Used with activity indicator LED logic, automatically inverted for ESP32 during setup


// rarify_receive distills incoming WiZmote commands. Authenticates, de-duplicates, and allows for multiple WiZmotes to be used simultaniously
#ifdef ESP8266
 #include <espnow.h>
 void rarify_receive(uint8_t *bssid, uint8_t *dat, uint8_t len) {
#else // ESP32
 #include <esp_now.h>
 void rarify_receive(const uint8_t *bssid, const uint8_t *dat, int len) {
#endif
  if(memcmp(bssid, allow, sizeof(allow))) return;                                        // Ignore if sender bssid is not allowed.
  memcpy(fingerprint, bssid, 6);                                                         // Create fingerprint +bssid
  memcpy(fingerprint+6, dat+1, 4);                                                       // Create fingerprint +message id
  if(memmem(holder, sizeof(holder), fingerprint, sizeof(fingerprint))) return;           // Ignore if fingerprint has been used recently
  memcpy(holder, holder+sizeof(fingerprint), sizeof(holder)-sizeof(fingerprint));        // Remove oldest fingerprint from holder
  memcpy(holder+(sizeof(holder)-sizeof(fingerprint)), fingerprint, sizeof(fingerprint)); // Add newest fingerprint to holder  
  snprintf(bssid_,sizeof(bssid_),"%02x%02x%02x%02x%02x%02x",bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]); // Format bssid
  digitalWrite(BUILTIN_LED, activity); ledtime=millis()+200;                             // Toggle LED and set timer
  on_receive(bssid_, dat[8], (dat[4]<<24)|(dat[3]<<16)|(dat[2]<<8)|(dat[1]), dat[6]);    // Send bssid, battery level, message id, and button code to on_receive
}


void on_receive(char* bssid, uint8_t battery, uint32_t mid, uint8_t btnc){
  snprintf(msg, sizeof(msg), "{\"remote_id\": \"%s\", \"battery\": %u, \"message_id\": %u, \"button_id\": %u}", bssid, battery, mid, btnc); // Create JSON for Serial
  Serial.println(msg);                                     // Send JSON over serial
  snprintf(msg, sizeof(msg), "wizmote/wizmote-%s", bssid); // Create MQTT Topic
  client.publish(msg, String(btnc));                       // Publish button code to topic - ditch String if possible
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(BUILTIN_LED, OUTPUT); 
  digitalWrite(BUILTIN_LED, !activity);
  
  if(esp_now_init()!=0){return;}               // Quit if error intitlizing ESP-NOW

  #ifdef ESP8266
    WiFi.setSleepMode(WIFI_NONE_SLEEP);        // Disable Sleep on ESP8266 for good meassure
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); // Priority is given to the SoftAP interface ?
  #else // ESP32
    activity = !activity;                      // Invert LED logic for ESP32
    WiFi.setSleep(false);                      // Disable WiFi Sleep on ESP32, needed for ESP-NOW receiving while WiFi connected
  #endif
  
  esp_now_register_recv_cb(rarify_receive);    // Register callback for incoming WiZmote commands

  WiFi.macAddress(holder);                     // Use holder to temporarily store mac address for processing

  snprintf(client_id, sizeof(client_id), "%s-%02x%02x%02x%02x%02x%02x",client.getMqttClientName(),holder[0],holder[1],holder[2],holder[3],holder[4],holder[5]); // Create client id
  snprintf(will_topic, sizeof(will_topic), "wizmote/%s", client_id); // Create the will topic

  client.setMqttClientName(client_id);                       // Set the MQTT client id
  client.enableDebuggingMessages();                          // Enable debugging messages over serial
  client.enableLastWillMessage(will_topic, "offline", true); // Enable persistant will message
}


void onConnectionEstablished(){ // Is called when both WiFi and MQTT are connected.
  client.publish(will_topic, "online", true);
  }


void loop() {  
  if(digitalRead(BUILTIN_LED == activity) && ledtime < millis()){
    digitalWrite(BUILTIN_LED, !activity);
  }
  client.loop();
}
