#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid     = "ASUS_88_2G";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "majesty_7779";     // The password of the Wi-Fi network
const int ANALOG_PIN = A0;

void setup() {
  Serial.begin(9600);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
}

void loop() {
  
  int num_scans = 5;
  int32_t two_scans[num_scans];
  int32_t four_scans[num_scans];
  int32_t eight_scans[num_scans];
  String wifi_ssid[3];
  String wifi_bssid[3];

  for(int i = 0; i < num_scans; i++){
    two_scans[i] = 0;
    four_scans[i] = 0;
    eight_scans[i] = 0;
  }
  
  for(int j=0; j<num_scans; j++){ 
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    
    if (n != 0) {
      for (int i = 0; i < n; ++i) {
        if(WiFi.BSSIDstr(i) == "04:92:26:8D:71:28") {
          wifi_ssid[0] = WiFi.SSID(i);
          wifi_bssid[0] = WiFi.BSSIDstr(i);
          two_scans[j] = WiFi.RSSI(i);
        }else if(WiFi.BSSIDstr(i) == "04:92:26:8D:71:48"){
          wifi_ssid[1] = WiFi.SSID(i);;
          wifi_bssid[1] = WiFi.BSSIDstr(i);
          four_scans[j] = WiFi.RSSI(i);
        }else if(WiFi.BSSIDstr(i) == "04:92:26:8D:70:88"){
          wifi_ssid[2] = WiFi.SSID(i);
          wifi_bssid[2] = WiFi.BSSIDstr(i);
          eight_scans[j] = WiFi.RSSI(i);
        }
        delay(10);
      }
    }
  }

  float two_sum = 0;
  float four_sum = 0;
  float eight_sum = 0;
  
  for(int j = 0; j < num_scans; j++){
    two_sum += two_scans[j];
    four_sum += four_scans[j];
    eight_sum += eight_scans[j];
  }

  float sums[3];
  sums[0] = two_sum;
  sums[1] = four_sum;
  sums[2] = eight_sum;

  float averages[3];
  for(int i = 0; i < 3; i++){
    averages[i] = sums[i]/num_scans;
  }

  Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.print(wifi_ssid[0] + ", " + wifi_bssid[0] + " : " + averages[0]);
  Serial.println();
  Serial.print(wifi_ssid[1] + ", " + wifi_bssid[1] + " : " + averages[1]);
  Serial.println();
  Serial.print(wifi_ssid[2] + ", " + wifi_bssid[2] + " : " + averages[2]);
  Serial.println();
  Serial.print("Analog Pin = " + String(analogRead(ANALOG_PIN)));
  Serial.println();  


  DynamicJsonDocument doc(2048);
  doc["mac"] = String(WiFi.macAddress());
  doc["weightSensor"] = analogRead(ANALOG_PIN);
  JsonArray wifiScan = doc.createNestedArray("wifiScan");
  for(int i = 0; i < 3; i++) {
    JsonObject nested = wifiScan.createNestedObject();
    nested["ssid"] = wifi_ssid[i];
    nested["bssid"] = wifi_bssid[i];
    nested["rssi"] = averages[i];
  }
  
  char JSONmessageBuffer[300];
  serializeJson(doc, JSONmessageBuffer, sizeof(JSONmessageBuffer));

  Serial.println(measureJson(doc));
  
  HTTPClient http; //Declare object of class HTTPClient

  http.begin("http://192.168.50.172:8080/api/v1/location/add"); //Specify request destination
  http.addHeader("Content-Type", "application/json"); //Specify content-type header
  
  int httpCode = http.POST(JSONmessageBuffer); //Send the request
  String payload = http.getString(); //Get the response payload
  
  Serial.println(httpCode); //Print HTTP return code
  Serial.println(payload); //Print request response payload
  
  http.end(); //Close connection
}
