#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";
String iotdb_server_ip = "";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// URL REST API IoTDB (Port 8181 port REST API default)
String rest_api_url = "http://" + iotdb_server_ip + ":8181/rest/v2/insert";

// otorisasi default IoTDB (user:root, pass:root) di-encode ke Base64
// "root:root" -> "cm9vdDpyb290"
String auth = "Basic cm9vdDpyb290";

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Koneksi ke WiFi
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("not konek");
  }
  Serial.println("\nWiFi terhubung!");
  Serial.print("Alamat IP ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    delay(2000);
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(t);
  Serial.print(" °C, Kelembaban: ");
  Serial.print(h);
  Serial.println(" %");

  kirimDataKeIoTDB(t, h);
  delay(10000);
}

void kirimDataKeIoTDB(float suhu, float kelembaban) {
  HTTPClient http;
  
  // Siapkan JSON Payload
  // Kita tidak perlu mengirim 'time', IoTDB akan menggunakan waktu server saat data diterima
  StaticJsonDocument<256> doc;
  doc["device"] = "root.proyek_dht22.esp32_01";
  
  JsonArray measurements = doc.createNestedArray("measurements");
  measurements.add("suhu");
  measurements.add("kelembaban");
  
  JsonArray values = doc.createNestedArray("values");
  values.add(suhu);
  values.add(kelembaban);
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);

  http.begin(rest_api_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", auth);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
