#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDR   0x3C

int pb1 = 16;
int pb2 = 17;
int pb3 = 18;
int buzzer = 5;
int scale = 5;
int led = 19;

bool ledState = false;
bool lastButtonState1 = HIGH;
bool lastButtonState2 = HIGH;
bool lastButtonState3 = HIGH;

bool showCatScreen = false; // Flag to track screen state
bool screenBlinking = false; // Flag to track screen blinking state

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastBlinkTime = 0; // Variable to track blink timing
const unsigned long blinkInterval = 1000; // Interval for screen blink (in milliseconds)

uint8_t RxMACaddress[] = {0x24, 0xEC, 0x4A, 0x0E, 0xC0, 0x20};

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ESP-NOW Data Structure
typedef struct TxStruct {
  int buttonState;
} TxStruct;
TxStruct sentData;

// ESP-NOW Send Callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Function to draw first screen
void drawPixelArts() {
  display.clearDisplay();
  
  const uint8_t bunny[] = {
    0b00000000,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01111110,
    0b01011010,
    0b01111110,
    0b00000000,
  };

  const uint8_t leftArrow[] = {
    0b00000000,
    0b00000000,
    0b00100000,
    0b01100000,
    0b11100000,
    0b01100000,
    0b00100000,
    0b00000000,
  };

  const uint8_t rightArrow[] = {
    0b00000000,
    0b00000000,
    0b00000100,
    0b00000110,
    0b00000111,
    0b00000110,
    0b00000100,
    0b00000000,
  };

  int startX1 = (128 - (8 * scale)) / 2;
  int startX2 = 0;
  int startX3 = 126 - (8 * scale);
  int startY = (50 - (8 * scale)) / 2;

  auto drawArt = [](const uint8_t art[], int startX, int startY) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (art[y] & (1 << (7 - x))) {
          display.fillRect(startX + (x * scale), startY + (y * scale), scale, scale, SSD1306_WHITE);
        }
      }
    }
  };
  
  drawArt(bunny, startX1, startY);

  // Display text
  display.setTextSize(2);  // Set font size
  display.setTextColor(SSD1306_WHITE);  // Set text color
  display.setCursor(18, 45);
  display.print("pick me?");

  drawArt(leftArrow, startX2, startY);
  drawArt(rightArrow, startX3, startY);
  
  display.display();
}

// Function to draw second screen
void drawPixelArts2() {
  display.clearDisplay();
  
  const uint8_t cat[] = {
    0b00000000,
    0b00000000,
    0b10000001,
    0b11000011,
    0b11111111,
    0b11011011,
    0b01111110,
    0b00000000,
  };

  const uint8_t leftArrow[] = {
    0b00000000,
    0b00000000,
    0b00100000,
    0b01100000,
    0b11100000,
    0b01100000,
    0b00100000,
    0b00000000,
  };

  const uint8_t rightArrow[] = {
    0b00000000,
    0b00000000,
    0b00000100,
    0b00000110,
    0b00000111,
    0b00000110,
    0b00000100,
    0b00000000,
  };

  int startX1 = (128 - (8 * scale)) / 2;
  int startX2 = 0;
  int startX3 = 126 - (8 * scale);
  int startY = (64 - (8 * scale)) / 2;

  auto drawArt = [](const uint8_t art[], int startX, int startY) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (art[y] & (1 << (7 - x))) {
          display.fillRect(startX + (x * scale), startY + (y * scale), scale, scale, SSD1306_WHITE);
        }
      }
    }
  };
  
  drawArt(cat, startX1, startY);
  drawArt(leftArrow, startX2, startY);
  drawArt(rightArrow, startX3, startY);
  display.setTextSize(2);  // Set font size
  display.setTextColor(SSD1306_WHITE);  // Set text color
  display.setCursor(18, 45);
  display.print("pick me?");
  
  display.display();
}

// Setup Function
void setup() {
  Serial.begin(115200);
  
  pinMode(pb1, INPUT_PULLUP);
  pinMode(pb2, INPUT_PULLUP);
  pinMode(pb3, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  digitalWrite(led, ledState);
  
  // Initialize OLED
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  
  // Start with first screen
  drawPixelArts();
  display.display();

  // ESP-NOW Init
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, RxMACaddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

// Main Loop
void loop() {
  bool buttonState1 = digitalRead(pb1);
  bool buttonState2 = digitalRead(pb2);  // Read pb2 state for blinking
  bool buttonState3 = digitalRead(pb3);

  // Debounce logic for pb1 (Switch to Cat screen)
  if (buttonState1 == LOW && lastButtonState1 == HIGH) { 
    delay(debounceDelay); // Wait for debounce time
    if (digitalRead(pb1) == LOW) { // Confirm the button is still pressed
      showCatScreen = true;  // Switch to Cat screen
      beepBuzzer();
    }
  }

  // Debounce logic for pb3 (Switch back to Bunny screen)
  if (buttonState3 == LOW && lastButtonState3 == HIGH) {
    delay(debounceDelay);
    if (digitalRead(pb3) == LOW) {
      showCatScreen = false;  // Switch back to Bunny screen
      beepBuzzer();
    }
  }

  // Debounce logic for pb2 (Blink screen toggle)
  if (buttonState2 == LOW && lastButtonState2 == HIGH) {
    delay(debounceDelay); // Wait for debounce time
    if (digitalRead(pb2) == LOW) {
      screenBlinking = !screenBlinking;  // Toggle blinking state
      beepBuzzer();  // Beep the buzzer when pb2 is pressed
    }
  }

  // Handle screen blinking
  if (screenBlinking) {
    if (millis() - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = millis();  // Update the blink time
      display.clearDisplay();
      display.display();  // Turn the screen off
      delay(100);  // Wait for a short moment
      if (showCatScreen) {
        drawPixelArts2();  // Redraw Cat screen after blink
      } else {
        drawPixelArts();   // Redraw Bunny screen after blink
      }
    }
  } else {
    // Show the appropriate screen based on the state
    if (showCatScreen) {
      drawPixelArts2();  // Draw the Cat screen
    } else {
      drawPixelArts();  // Draw the Bunny screen
    }
  }

  // Update last button states for debounce
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;
  lastButtonState3 = buttonState3;

  // Send data via ESP-NOW
  sentData.buttonState = screenBlinking ? 1 : 0;
  esp_err_t result = esp_now_send(RxMACaddress, (uint8_t *)&sentData, sizeof(sentData));
  if (result == ESP_OK) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Error sending data");
  }

  delay(50);  // Small delay to prevent unnecessary CPU usage
}

void beepBuzzer() {
  digitalWrite(buzzer, HIGH);  // Turn on buzzer
  delay(100);                  // Keep buzzer on for 100 milliseconds
  digitalWrite(buzzer, LOW);   // Turn off buzzer
}
