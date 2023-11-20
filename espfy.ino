#include <ESP8266WiFi.h>

const char* ssid = "your-ssid";
const char* pass = "your-password";

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

  configTime(-3 * 3600, 0, "pool.ntp.br", "pool.ntp.org");

  while (time(nullptr) < 99999) { delay(500); }

  time_t now = time(nullptr);

  Serial.print("Current time: ");
  Serial.println(asctime(localtime(&now)));
}

void loop() { }
