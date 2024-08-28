#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include "AudioTools.h"
#include "AudioLibs/A2DPStream.h"

// Configurația pentru I2C
#define I2C_SDA 15
#define I2C_SCL 14
TwoWire I2Cbus = TwoWire(0);  // Utilizare instanță specifică pentru I2C

// Configurația pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cbus, OLED_RESET);

// Configurația pentru butoane
const int buttonPrev = 12;  // GPIO 12 cu pull-down extern
const int buttonNext = 13;  // GPIO 13 cu pull-down extern
const int buttonSelect = 2; // GPIO 2 cu pull-down extern

// Configurația pentru Bluetooth și Audio
I2SStream i2sStream;                            // Access I2S as stream
A2DPStream a2dpStream;                          // Access A2DP as stream
StreamCopy copier(a2dpStream, i2sStream);       // Copy i2sStream to a2dpStream
ConverterFillLeftAndRight<int16_t> filler(LeftIsEmpty); // Fill both channels

// Configurația pentru WiFi
String ssidList[10];
int ssidCount = 0;
int selectedSSIDIndex = 0;
String selectedSSID = "";
String wifiPassword = "";
bool wifiConnected = false;

// Stațiile radio (URL-urile fluxurilor din Suedia)
String radioStations[] = {
  "https://fm01-ice.stream.khz.se/fm01_mp3", // rixfm
  "https://fm05-ice.stream.khz.se/fm05_mp3", // starFM
  "https://wr13-ice.stream.khz.se/wr13_mp3", // svenska favoriter
  "https://wr15-ice.stream.khz.se/wr15_mp3", // disco 54
  "https://wr09-ice.stream.khz.se/wr09_mp3"  // gamla favoriter
};
int currentStation = 0;
bool isPlaying = false;

// Meniuri
enum MenuState { MAIN_MENU, WIFI_MENU, BLUETOOTH_MENU, RADIO_MENU, PASSWORD_ENTRY };
MenuState currentMenu = MAIN_MENU;
int mainMenuIndex = 0;
String mainMenuOptions[] = { "WiFi", "Bluetooth", "Play Radio" };

// Declarațiile funcțiilor
void displayMainMenu();
void handleMainMenu();
void handleWiFiMenu();
void scanWiFiNetworks();
void displayWiFiNetworks();
void handleWiFiSelection();
void displayPasswordPrompt();
void handlePasswordEntry();
void connectToWiFi();
void displayStationInfo();
void handleStationSelection();
void transmitAudio(String stationUrl);
void displayWiFiPassword(char* password);

void setup() {
  Serial.begin(115200);

  // Inițializare I2C cu pinii definiți
  I2Cbus.begin(I2C_SDA, I2C_SCL, 100000);

  // Configurare OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.printf("SSD1306 OLED display failed to initialize.\nCheck that display SDA is connected to pin %d and SCL connected to pin %d\n", I2C_SDA, I2C_SCL);
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Configurare butoane
  pinMode(buttonPrev, INPUT_PULLDOWN);  // Utilizează pull-down extern
  pinMode(buttonNext, INPUT_PULLDOWN);  // Utilizează pull-down extern
  pinMode(buttonSelect, INPUT_PULLDOWN); // Utilizează pull-down extern

  // Configurare Bluetooth și Audio
  Serial.println("starting A2DP...");
  auto cfgA2DP = a2dpStream.defaultConfig(TX_MODE);
  cfgA2DP.name = "ESP32-A2DP";
  a2dpStream.begin(cfgA2DP);
  a2dpStream.setVolume(0.3);

  Serial.println("starting I2S...");
  a2dpStream.addNotifyAudioChange(i2sStream);
  i2sStream.begin(i2sStream.defaultConfig(RX_MODE));

  // Afișează meniul principal
  displayMainMenu();
}

void loop() {
  switch (currentMenu) {
    case MAIN_MENU:
      handleMainMenu();
      break;
    case WIFI_MENU:
      handleWiFiMenu();
      break;
    case BLUETOOTH_MENU:
      // Gestionare meniul Bluetooth
      break;
    case RADIO_MENU:
      handleStationSelection();
      if (isPlaying) {
        transmitAudio(radioStations[currentStation]);
      }
      break;
    case PASSWORD_ENTRY:
      handlePasswordEntry();
      break;
  }
}

void displayMainMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Main Menu:");
  display.println(mainMenuOptions[mainMenuIndex]);
  display.display();
}

void handleMainMenu() {
  if (digitalRead(buttonPrev) == HIGH) {  // Butonul Prev apăsat
    mainMenuIndex--;
    if (mainMenuIndex < 0) {
      mainMenuIndex = 2;
    }
    displayMainMenu();
    delay(200); // Debounce
  }

  if (digitalRead(buttonNext) == HIGH) {  // Butonul Next apăsat
    mainMenuIndex++;
    if (mainMenuIndex > 2) {
      mainMenuIndex = 0;
    }
    displayMainMenu();
    delay(200); // Debounce
  }

  if (digitalRead(buttonSelect) == HIGH) {  // Butonul Select apăsat
    switch (mainMenuIndex) {
      case 0:
        currentMenu = WIFI_MENU;
        scanWiFiNetworks();
        displayWiFiNetworks();
        break;
      case 1:
        currentMenu = BLUETOOTH_MENU;
        // Implementare meniul Bluetooth
        break;
      case 2:
        currentMenu = RADIO_MENU;
        displayStationInfo();
        break;
    }
    delay(200); // Debounce
  }
}

void handleWiFiMenu() {
  handleWiFiSelection();
}

void scanWiFiNetworks() {
  int n = WiFi.scanNetworks();
  ssidCount = (n < 10) ? n : 10;
  
  for (int i = 0; i < ssidCount; i++) {
    ssidList[i] = WiFi.SSID(i);
  }
}

void displayWiFiNetworks() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Select WiFi:");
  display.println(ssidList[selectedSSIDIndex]);
  display.display();
}

void handleWiFiSelection() {
  if (digitalRead(buttonPrev) == HIGH) {  // Butonul Prev apăsat
    selectedSSIDIndex--;
    if (selectedSSIDIndex < 0) {
      selectedSSIDIndex = ssidCount - 1;
    }
    displayWiFiNetworks();
    delay(200); // Debounce
  }

  if (digitalRead(buttonNext) == HIGH) {  // Butonul Next apăsat
    selectedSSIDIndex++;
    if (selectedSSIDIndex >= ssidCount) {
      selectedSSIDIndex = 0;
    }
    displayWiFiNetworks();
    delay(200); // Debounce
  }

  if (digitalRead(buttonSelect) == HIGH) {  // Butonul Select apăsat
    selectedSSID = ssidList[selectedSSIDIndex];
    currentMenu = PASSWORD_ENTRY;
    displayPasswordPrompt();
  }
}

void displayPasswordPrompt() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Enter WiFi Password:");
  display.display();
}

void handlePasswordEntry() {
  char password[32] = {0};
  static int charIndex = 0;
  static char currentChar = 'A'; // Inițializam cu 'A'
  
  if (digitalRead(buttonPrev) == HIGH) {  // Butonul Prev apăsat
    currentChar = (currentChar == 'A') ? 'Z' : currentChar - 1; // Se întoarce la 'Z' dacă este 'A'
    password[charIndex] = currentChar;
    displayWiFiPassword(password);
    delay(200);
  }
  
  if (digitalRead(buttonNext) == HIGH) {  // Butonul Next apăsat
    currentChar = (currentChar == 'Z') ? 'A' : currentChar + 1; // Se întoarce la 'A' dacă este 'Z'
    password[charIndex] = currentChar;
    displayWiFiPassword(password);
    delay(200);
  }

  if (digitalRead(buttonSelect) == HIGH) {  // Butonul Select apăsat
    if (charIndex < 31) {
      charIndex++;
      currentChar = 'A'; // Resetează caracterul pentru următoarea poziție
    } else {
      wifiPassword = String(password);
      connectToWiFi();
      return; // Ieși din funcție după ce s-a terminat introducerea parolei
    }
    delay(200);
  }
}

void displayWiFiPassword(char* password) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Password:");
  display.println(password);
  display.display();
}

void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting to:");
  display.println(selectedSSID);
  display.display();

  WiFi.begin(selectedSSID.c_str(), wifiPassword.c_str());

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 10) {
    delay(1000);
    display.print(".");
    display.display();
    counter++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    display.println("\nConnected!");
  } else {
    display.println("\nFailed to connect.");
  }
  display.display();
  delay(2000); // Afișează mesajul pentru câteva secunde
  currentMenu = MAIN_MENU;
  displayMainMenu();
}

void displayStationInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Radio:");
  display.println(radioStations[currentStation]);
  display.display();
}

void handleStationSelection() {
  if (digitalRead(buttonPrev) == HIGH) {  // Butonul Prev apăsat
    currentStation--;
    if (currentStation < 0) {
      currentStation = sizeof(radioStations) / sizeof(radioStations[0]) - 1;
    }
    displayStationInfo();
    delay(200); // Debounce
  }

  if (digitalRead(buttonNext) == HIGH) {  // Butonul Next apăsat
    currentStation++;
    if (currentStation >= sizeof(radioStations) / sizeof(radioStations[0])) {
      currentStation = 0;
    }
    displayStationInfo();
    delay(200); // Debounce
  }

  if (digitalRead(buttonSelect) == HIGH) {  // Butonul Select apăsat
    isPlaying = !isPlaying;
    if (isPlaying) {
      // Configurează URL-ul pentru redare audio
      Serial.println("Playing station: " + radioStations[currentStation]);
    }
    delay(200); // Debounce
  }
}

void transmitAudio(String stationUrl) {
  HTTPClient http;
  http.begin(stationUrl);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    WiFiClient *audioClient = http.getStreamPtr();
    // Asigură-te că redirecționezi fluxul audio de la HTTPClient la i2sStream
    Serial.println("Streaming audio from: " + stationUrl);
    while (isPlaying) {
      if (audioClient->available()) {
        uint8_t buffer[512];
        int len = audioClient->read(buffer, sizeof(buffer));
        i2sStream.write(buffer, len);
      }
      delay(100); // Ajustează în funcție de nevoi
    }
    http.end();
  } else {
    Serial.println("Failed to connect to the audio stream.");
  }
}
