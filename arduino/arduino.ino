#include "esp_camera.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <HTTPClient.h>

#define CROQ_PIN 14
#define MIC_PIN 16

HTTPClient http;
WebSocketsServer webSocket = WebSocketsServer(1337);
uint8_t cam_num;
bool connected = false;
int micState = 0;

const char* ssid = "Palomita";
const char* password = "01072001";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            connected = false;
            break;
        case WStype_CONNECTED:
            webSocket.sendTXT(num, "Connected");
            cam_num = num;
            connected = true;
            break;
        case WStype_TEXT:
            webSocket.sendTXT(num, payload);
            if (payload[0] == '-'){
              if(enabledDistributeur()){
              }
            }
            break;
        case WStype_BIN:
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

boolean enabledDistributeur(){
  digitalWrite(CROQ_PIN, HIGH);
  delay(1000);
  digitalWrite(CROQ_PIN, LOW);
  return true;
}

void micDetection(uint8_t num){
  micState = digitalRead(MIC_PIN);
  if (micState == HIGH) {
    Serial.println("Notification Send");
    http.begin("https://exp.host/--/api/v2/push/send");
    http.addHeader("Content-Type", "application/json");
    http.POST("{\n    to: ExponentPushToken[kPlRLFJkmNGxAACObGeKNF],\n    sound: 'default',\n    title: 'Aboiement Detecté',\n    body: 'Votre Chien est entrain d'aboyer, Ouvrez l'application pour lui donner une croquette'\n  }");
  }
}

void configMic(){
  pinMode(MIC_PIN, INPUT_PULLUP);
}

void configDistributeur(){
  pinMode(14, OUTPUT);
}

void configCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void liveCam(uint8_t num){
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Frame buffer could not be acquired");
      return;
  }
  //replace this with your own function
  webSocket.sendBIN(num, fb->buf, fb->len);

  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

void setup() {
  // put your setup code here, to run once:
  WiFi.begin(ssid, password);

  Serial.begin(115200);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  configCamera();
  configDistributeur();
  configMic();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
  if(connected == true){
    liveCam(cam_num);
  }
  //micDetection(cam_num);
}
