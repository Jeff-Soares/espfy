#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

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
