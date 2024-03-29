#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "src\Credentials.h"

#define TRIGGER_PIN 2

const unsigned int port = 8888;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
char replyBuffer[] = "ack";

WiFiUDP Udp;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(128);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.println("\nStarting");
  // Delay because some ESP-01 pins needs to be HIGH at boot
  delay(5000);

  WiFi.mode(WIFI_STA);

  if (digitalRead(TRIGGER_PIN) == LOW) {
    WiFiManager wifiManager;

    if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

    saveCredentials(WiFi.SSID(), WiFi.psk());
  } else {
    Credentials cred = getCredentials();

    WiFi.begin(cred.ssid, cred.pass);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  configTime(-3 * 3600, 0, "pool.ntp.br", "pool.ntp.org");

  while (time(nullptr) < 99999) { delay(500); }

  time_t now = time(nullptr);

  Serial.print("Current time: ");
  Serial.println(asctime(localtime(&now)));

  Udp.begin(port);
  Serial.printf("UDP server on port %d\n", port);

  EEPROM.end();
}

void loop() {
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    Serial.printf("Received packet of size %d from %s:%d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());

    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.print("Message: ");
    Serial.println(packetBuffer);

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyBuffer);
    Udp.endPacket();
  }
}

void saveCredentials(String ssid, String pass) {
  Credentials _cred = getCredentials();

  if (ssid.equals(_cred.ssid) && pass.equals(_cred.pass)) {
    return;
  }

  EEPROM.write(0, ssid.length());
  EEPROM.write(1, pass.length());
  saveString(ssid.c_str(), 2);
  saveString(pass.c_str(), 2 + ssid.length());
}

Credentials getCredentials() {
  int ssidLength = (int) EEPROM.read(0);
  int passLength = (int) EEPROM.read(1);

  Credentials cred = {
    ssid: readString(2, ssidLength),
    pass: readString(2 + ssidLength, passLength)
  };

  return cred;
}

void saveString(const char* str, int address) {
  int i;
  for (i = 0; str[i] != '\0'; i++) {
    EEPROM.write(address + i, str[i]);
  }
  EEPROM.write(address + i, '\0');
  EEPROM.commit();
}

char* readString(int address, int size) {
  char buffer[size + 1];
  for (int i = 0; i < size; i++) {
    buffer[i] = EEPROM.read(address + i);
  }
  buffer[size] = '\0';
  return strdup(buffer);
}
