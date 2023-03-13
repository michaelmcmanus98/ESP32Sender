#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <esp_now.h>

#define LED_PIN 33
#define JOYSTICK_X 36
#define JOYSTICK_Y 39
#define LED_COUNT 12
#define bufferSize 10
// Set ESP-Now communication channel
#define CHANNEL 1

uint8_t destinationMAC[]= {0x40,0x22,0xD8,0xEB,0x09,0xD4};
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x40, 0x22, 0xD8, 0xEA, 0x92, 0xB4};

// Define data structure for communication
typedef struct struct_message {
  int A;
  int B;
} struct_message;

typedef struct{
  int coordinate;
} esp_message;

// Create instance of data structure
struct_message message;
esp_message myData;

esp_now_peer_info_t peerInfo;

Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint16_t xVal;
uint16_t yVal;

uint16_t xBuffer[bufferSize];
uint16_t yBuffer[bufferSize];

uint8_t ix = 0;
uint8_t iy = 0;

uint16_t getXBufferVal(int size){
  int result = 0;
  for(int i = 0; i < size; i++){
    result += xBuffer[i];
  }
  return result/size;
}

uint16_t getYBufferVal(int size){
  int result = 0;
  for(int i = 0; i < size; i++){
    result += yBuffer[i];
  }
  return result/size;
}

uint16_t getJoystickRegion(uint16_t xVal, uint16_t yVal){
  uint16_t result = 0;
  if(yVal < 1365)
    result = 0;
  else if(yVal < 2730)
    result = 3;
  else
    result = 6;

  if(xVal < 1365)
    result += 0;
  else if(xVal < 2730)
    result += 1;
  else
    result += 2;

  return result;
}

void setRing(uint16_t region){
  ring.clear();

  switch(region){
    case 0:
      ring.setPixelColor(7,ring.Color(30,0,0));
      ring.setPixelColor(8,ring.Color(30,0,0));
      break;
    case 1:
      ring.setPixelColor(5,ring.Color(30,0,0));
      ring.setPixelColor(6,ring.Color(30,0,0));
      ring.setPixelColor(7,ring.Color(30,0,0));
      break;
    case 2:
      ring.setPixelColor(4,ring.Color(30,0,0));
      ring.setPixelColor(5,ring.Color(30,0,0));
      break;
    case 3:
      ring.setPixelColor(8,ring.Color(30,0,0));
      ring.setPixelColor(9,ring.Color(30,0,0));
      ring.setPixelColor(10,ring.Color(30,0,0));
      break;
    case 6:
      ring.setPixelColor(10,ring.Color(30,0,0));
      ring.setPixelColor(11,ring.Color(30,0,0));
      break;
    case 5:
      ring.setPixelColor(2,ring.Color(30,0,0));
      ring.setPixelColor(3,ring.Color(30,0,0));
      ring.setPixelColor(4,ring.Color(30,0,0));
      break;
    case 8:
      ring.setPixelColor(1,ring.Color(30,0,0));
      ring.setPixelColor(2,ring.Color(30,0,0));
      break;
    case 7:
      ring.setPixelColor(11,ring.Color(30,0,0));
      ring.setPixelColor(0, ring.Color(30,0,0));
      ring.setPixelColor(1,ring.Color(30,0,0));
      break;
  }

  ring.show();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

}

// void onRecv(const unsigned char *mac, const unsigned char *data, int len) {
//   memcpy(&message, data, sizeof(message));
//   Serial.println("Message received from master:");
//   Serial.print("  A: ");
//   Serial.println(message.A);
//   Serial.print("  B: ");
//   Serial.println(message.B);
//   setRing(getJoystickRegion(message.A, message.B));
// }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  ring.begin();
  ring.clear();


  // Initialize WiFi module
  WiFi.mode(WIFI_STA);

  // Initialize ESP-Now
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-Now");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  //add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // // Register peer ESP32 (master)
  // esp_now_peer_info_t peerInfo;
  // memset(&peerInfo, 0, sizeof(peerInfo));
  // peerInfo.channel = CHANNEL;
  // peerInfo.encrypt = false;
  // memcpy(peerInfo.peer_addr, destinationMAC , sizeof(destinationMAC));
  // if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //   Serial.println("Failed to add peer");
  //   return;
  // }

  // Set callback for receiving messages
  //esp_now_register_recv_cb(onRecv);
}

void loop() {
  // strcpy(myData.a, "THIS IS A CHAR");
  // myData.b = random(1,20);
  // myData.c = 1.2;
  // myData.d = false;
  //delay(2000);
  // //-----------------------------
  xBuffer[ix] = analogRead(JOYSTICK_X);
  yBuffer[iy] = analogRead(JOYSTICK_Y);
  ix = (ix++)%bufferSize;
  iy = (iy++)%bufferSize;
  Serial.print(xVal);
  Serial.print(yVal);

  myData.coordinate = getJoystickRegion(getXBufferVal(bufferSize), getYBufferVal(bufferSize));

  // for (int i=0; i<12; i++){
  // ring.setPixelColor(i,ring.Color(30,0,0));
  // ring.show();
  // delay(500);
  // }

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
    Serial.println("Sent with success");
    Serial.println(myData.coordinate);
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(2000);

}