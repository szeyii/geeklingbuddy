//receiver

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

bool ledState = false;
bool lastState = HIGH;

int led = 19;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; //50ms debounce delay

Adafruit_SSD1306 display(128, 64, &Wire, -1);

typedef struct RxStruct{
  int buttonState;
}RxStruct;
RxStruct receivedData;

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t * incomingData, int len){
  // Debugging: Print received data and MAC address
  Serial.println("Data received:");
  for (int i = 0; i < len; i++) {
    Serial.print(incomingData[i]);
    Serial.print(" ");
  }
  Serial.println();
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Debugging: Print received button state
  Serial.print("Button State: ");
  Serial.println(receivedData.buttonState);
}
void setup() {
  Serial.begin(115200);

  pinMode(led, OUTPUT);
  digitalWrite(led, ledState);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //Dont proceed, loop forever
  }
  display.clearDisplay();

  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

}

void loop() {
  if (receivedData.buttonState == 1){
    Serial.println("Button pressed");
    delay(debounceDelay);  // Simple debounce logic
    ledState = !ledState;   // Toggle the LED state
    digitalWrite(led, ledState);
    Serial.println(ledState ? "LED ON" : "LED OFF");

  }

}
