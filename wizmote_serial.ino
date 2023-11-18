#ifdef ESP8266 
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#else // ESP32
  #include <WiFi.h>
  #include <esp_now.h>
#endif



uint8_t allow[] =   {0x44, 0x4f, 0x8e}; // Allowed WiZmote bssid prefix, can be extended to full bssid. 44:4f:83:xx:xx:xx
uint8_t fingerprint [10];               // Fingerprint of sender bssid + message id
uint8_t holder      [50];               // Data blob used as fingerprint history, must be sized in multiples of fingerpint. 50 = 5 fingerprints



// rarify_receive distills incoming WiZmote ESP-NOW messages before passing to on_receive
// Authenticates, de-duplicates, and allows for multiple WiZmotes to be used simultaniously
#ifdef ESP8266 
 void rarify_receive(uint8_t * bssid, uint8_t *dat, uint8_t len) {
#else
 void rarify_receive(const uint8_t * bssid, const uint8_t *dat, int len) {
#endif

  if(memcmp(bssid, allow, sizeof(allow))) return;                                        // Ignore if sender bssid is not allowed.
  memcpy(fingerprint, bssid, 6);                                                         // Create fingerprint +bssid
  memcpy(fingerprint+6, dat+1, 4);                                                       // Create fingerprint +message id
  if(memmem(holder, sizeof(holder), fingerprint, sizeof(fingerprint))) return;           // Ignore if fingerprint has been used recently
  memcpy(holder, holder+sizeof(fingerprint), sizeof(holder)-sizeof(fingerprint));        // Remove oldest fingerprint from holder
  memcpy(holder+(sizeof(holder)-sizeof(fingerprint)), fingerprint, sizeof(fingerprint)); // Add newest fingerprint to holder

  digitalWrite(BUILTIN_LED, LOW);

  on_receive( // Send bssid, battery Level, message id, and button code to on_receive
    String(bssid[0],HEX)+String(bssid[1],HEX)+String(bssid[2],HEX)+String(bssid[3],HEX)+String(bssid[4],HEX)+String(bssid[5],HEX), // bssid
    dat[8],                                         // battery level 0?-100
    (dat[4]<<24)|(dat[3]<<16)|(dat[2]<<8)|(dat[1]), // message id
    dat[6]                                          // button code
    );
}



void on_receive(String bssid, uint8_t battery, uint32_t mid, uint8_t but){
  Serial.println("{\"remote_id\": \"" + bssid + "\", \"battery\": " + battery + ", \"message_id\": " + mid + ", \"button_id\": " + but + "}");  
}



void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  
  pinMode(BUILTIN_LED, OUTPUT); 
  digitalWrite(BUILTIN_LED, HIGH);
  
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;}

  #ifdef ESP8266
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  #endif
  
  esp_now_register_recv_cb(rarify_receive);
    
  Serial.println("{\"msg\": \"ready\"");
}



void loop() {
  if(digitalRead(BUILTIN_LED)){
    delay(200);
    digitalWrite(BUILTIN_LED, HIGH);
  }
}
