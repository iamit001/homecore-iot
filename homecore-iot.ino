#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// =====================================================
// Wi-Fi
// =====================================================

const char* ssid = "AirXtream-Gr_FL";
const char* password = "Wagonr@7207#3232";

// =====================================================
// MQTT
// =====================================================

const char* mqtt_server = "192.168.1.30";
const int mqtt_port = 1883;

const char* mqtt_topic = "sensors/ultrasonic/distance";

// =====================================================
// HC-SR04
// =====================================================

const int TRIG_PIN = 5; //D1
const int ECHO_PIN = 4; //D2

// =====================================================
// MQTT client
// =====================================================

WiFiClient espClient;
PubSubClient mqtt(espClient);


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("   ESP8266 ULTRASONIC MQTT");
  Serial.println("================================");


  // ===================================================
  // 1. Connect to Wi-Fi
  // ===================================================

  Serial.println();
  Serial.println("[1] Connecting to Wi-Fi...");

  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("ESP8266 IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Wi-Fi signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");


  // ===================================================
  // 2. Configure MQTT
  // ===================================================

  Serial.println();
  Serial.println("[2] Configuring MQTT...");

  Serial.print("MQTT server: ");
  Serial.println(mqtt_server);

  Serial.print("MQTT port: ");
  Serial.println(mqtt_port);

  Serial.print("MQTT topic: ");
  Serial.println(mqtt_topic);

  mqtt.setServer(mqtt_server, mqtt_port);

  Serial.println("MQTT configuration complete.");


  // ===================================================
  // 3. Connect to MQTT
  // ===================================================

  Serial.println();
  Serial.println("[3] Connecting to MQTT...");

  connectMQTT();


  // ===================================================
  // 4. Initialize HC-SR04
  // ===================================================

  Serial.println();
  Serial.println("[4] Initializing HC-SR04...");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  Serial.print("TRIG pin: ");
  Serial.println("D5");

  Serial.print("ECHO pin: ");
  Serial.println("D6");

  Serial.println("Ultrasonic sensor initialized.");


  // ===================================================
  // System ready
  // ===================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("        SYSTEM READY");
  Serial.println("================================");
  Serial.println();
}


// =====================================================
// CONNECT TO MQTT
// =====================================================

void connectMQTT() {

  while (!mqtt.connected()) {

    Serial.print("Connecting to MQTT broker... ");

    // Create unique MQTT client ID
    String clientId = "ESP8266-" + String(ESP.getChipId());

    if (mqtt.connect(clientId.c_str())) {

      Serial.println("SUCCESS!");

      Serial.print("Connected to: ");
      Serial.println(mqtt_server);

    } else {

      Serial.print("FAILED, state=");
      Serial.println(mqtt.state());

      Serial.println("Retrying in 3 seconds...");

      delay(3000);
    }
  }
}


// =====================================================
// READ ULTRASONIC DISTANCE
// =====================================================

float readDistance() {

  // Make sure trigger is LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10 microsecond trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure ECHO pulse
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo received
  if (duration == 0) {

    return -1;
  }

  // Calculate distance in centimeters
  float distance = duration * 0.0343 / 2.0;

  return distance;
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // ===================================================
  // Check Wi-Fi
  // ===================================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi connection lost!");

    delay(1000);

    return;
  }


  // ===================================================
  // Check MQTT
  // ===================================================

  if (!mqtt.connected()) {

    Serial.println("MQTT connection lost!");

    connectMQTT();
  }

  mqtt.loop();


  // ===================================================
  // Read ultrasonic sensor
  // ===================================================

  float distance = readDistance();


  // ===================================================
  // Handle sensor result
  // ===================================================

  if (distance < 0) {

    Serial.println();
    Serial.println("[SENSOR] No echo received!");

  } else {

    // -----------------------------------------------
    // Serial Monitor
    // -----------------------------------------------

    Serial.println();
    Serial.println("[SENSOR] Measurement");

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");


    // -----------------------------------------------
    // Convert distance to text
    // -----------------------------------------------

    char distanceString[20];

    dtostrf(distance, 1, 2, distanceString);


    // -----------------------------------------------
    // Publish MQTT
    // -----------------------------------------------

    Serial.println("[MQTT] Publishing...");

    Serial.print("Topic: ");
    Serial.println(mqtt_topic);

    Serial.print("Value: ");
    Serial.print(distanceString);
    Serial.println(" cm");


    if (mqtt.publish(mqtt_topic, distanceString)) {

      Serial.println("MQTT publish SUCCESS!");

    } else {

      Serial.println("MQTT publish FAILED!");

    }
  }


  // ===================================================
  // Wait before next measurement
  // ===================================================

  delay(1000);
}
