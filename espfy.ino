#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "types.h"

const char* ssid = "your-ssid";
const char* pass = "your-password";
const unsigned int port = 8888;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
char replyBuffer[] = "ack";

WiFiUDP Udp;

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(port);
  Serial.printf("UDP server on port %d\n", port);

  configTime(-3 * 3600, 0, "pool.ntp.br", "pool.ntp.org");

  while (time(nullptr) < 99999) { delay(500); }

  time_t now = time(nullptr);

  Serial.print("Current time: ");
  Serial.println(asctime(localtime(&now)));
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
  int ssidLength = (int) EEPROM.read(0);
  int passLength = (int) EEPROM.read(1);

  String _ssid = readString(2, ssidLength);
  String _pass = readString(2 + ssidLength, passLength);

  if (ssid.equals(_ssid) && pass.equals(_pass)) {
    return;
  }

  EEPROM.write(0, ssid.length());
  EEPROM.write(1, pass.length());
  saveString(ssid.c_str(), 2);
  saveString(pass.c_str(), 2 + ssid.length());
}

struct credentials getCredentials() {
  credentials cred;

  int ssidLength = (int) EEPROM.read(0);
  int passLength = (int) EEPROM.read(1);

  cred.ssid = readString(2, ssidLength);
  cred.pass = readString(2 + ssidLength, passLength);

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
  char buffer[size];
  for (int i = 0; i < size; i++) {
    buffer[i] = EEPROM.read(address + i);
  }
  return buffer;
}
