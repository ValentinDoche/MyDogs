#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(1337);

const char* ssid = "Palomita";
const char* password = "01072001";

void startCameraServer();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED:
            webSocket.sendTXT(num, "Connected");
            break;
        case WStype_TEXT:
            webSocket.sendTXT(num, payload);
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



void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  ledcSetup(15, 10000, 10);
  ledcAttachPin(14, 15);
  ledcWrite(15, 0);

  ledcSetup(14, 5000, 8);
  ledcAttachPin(4, 14);
  ledcWrite(14, 1);
  
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

  sensor_t * s = esp_camera_sensor_get();

  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_SVGA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  //websocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
}
