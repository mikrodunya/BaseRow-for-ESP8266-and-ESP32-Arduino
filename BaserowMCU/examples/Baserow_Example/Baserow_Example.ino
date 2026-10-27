#include <BaserowMCU.h> // WiFi headers and platform are selected automatically.

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
BaserowMCU baserow("YOUR_TABLE_ID", "YOUR_DATABASE_TOKEN");
const int ROW_ID = 1; // Actual existing Baserow row ID, NOT its display position.
const uint32_t INTERVAL_MS = 5000;
uint32_t lastCycle = 0;
bool firstCycle = true;

void printResult(const char* operation, bool ok) {
  Serial.printf("%s: %s, HTTP=%d, %lu ms, %u bytes\n", operation,
                ok ? "OK" : baserow.lastError().c_str(), baserow.lastHttpCode(),
                static_cast<unsigned long>(baserow.lastRequestMs()),
                static_cast<unsigned int>(baserow.lastResponseBytes()));
}
void setup() {
  Serial.begin(115200);
  Serial.printf("Platform: %s\n", BaserowMCU::platform());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  baserow.setTimeout(5000);
  // Original compatibility default: TLS encryption without certificate validation.
  // For verified TLS, synchronize time and call baserow.setCACert(ROOT_CA_PEM).
  // Optional for legacy readNumber/readText only: baserow.setReadCacheMs(250);
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) { delay(1); return; }
  if (!firstCycle && static_cast<uint32_t>(millis() - lastCycle) < INTERVAL_MS) {
    delay(1);
    return;
  }
  firstCycle = false;
  // One PATCH for both fields; no separate HTTP request per field.
  JsonDocument fields;
  fields["Ruzgar_Hizi"] = 12.4;
  fields["Hava_Kalitesi"] = "NORMAL";
  bool written = baserow.updateRow(ROW_ID, fields);
  printResult("updateRow", written);

  // One GET for the row. Accessing multiple fields below makes no requests.
  JsonDocument row;
  bool read = baserow.readRow(ROW_ID, row);
  printResult("readRow", read);
  if (read) {
    Serial.print("LED_Komut: "); Serial.println(row["LED_Komut"].as<int>());
    Serial.print("Hava_Kalitesi: "); Serial.println(row["Hava_Kalitesi"].as<String>());
  }
  // To create ONE new row containing all fields: baserow.insertRow(fields);
  // Calls remain synchronous. millis() removes only idle delay(), not HTTP blocking.
  lastCycle = millis();
}
