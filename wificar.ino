#include <WiFi.h>

// Replace with your desired network credentials
const char* ssid = "nikhil car";
const char* password = "12345678";

// Define motor control pins
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19
#define ENA 25  // Left motor speed control (PWM)
#define ENB 26  // Right motor speed control (PWM)

WiFiServer server(80); // HTTP server on port 80

// HTML interface
String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>WiFi Car Controller</title>
<style>
  button {
    width: 100px;
    height: 50px;
    margin: 10px;
    font-size: 16px;
  }
  .container {
    display: flex;
    flex-direction: column;
    align-items: center;
    margin-top: 20px;
  }
  .row {
    display: flex;
    justify-content: center;
  }
</style>
</head>
<body>
<h1 style="text-align: center;"> NIKHIL'S WiFi Car </h1>
<div class="container">
  <div class="row">
    <button onclick="sendCommand('FORWARD')">Forward</button>
  </div>
  <div class="row">
    <button onclick="sendCommand('LEFT')">Left</button>
    <button onclick="sendCommand('STOP')">Stop</button>
    <button onclick="sendCommand('RIGHT')">Right</button>
  </div>
  <div class="row">
    <button onclick="sendCommand('BACKWARD')">Backward</button>
  </div>
  <div class="row">
    <label>Speed: </label>
    <input type="range" id="speed" min="0" max="255" value="255" onchange="setSpeed(this.value)">
  </div>
</div>
<script>
  function sendCommand(command) {
    fetch('/control?cmd=' + command);  // Send command to the ESP32
  }

  function setSpeed(speed) {
    fetch('/control?speed=' + speed);  // Send speed change to the ESP32
  }
</script>
</body>
</html>
)rawliteral";

// Global variable for motor speed (default: maximum speed)
int motorSpeed = 255; // PWM value (0-255)

// Handle motor control
void controlCar(String command) {
  if (command == "FORWARD") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (command == "BACKWARD") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else if (command == "LEFT") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else if (command == "RIGHT") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (command == "STOP") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  // Set motor speeds using analogWrite
  analogWrite(ENA, motorSpeed); // Left motor speed (PWM)
  analogWrite(ENB, motorSpeed); // Right motor speed (PWM)
}

// Handle HTTP requests
void handleRequests() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // Handle control commands
    if (request.indexOf("GET /control?cmd=") >= 0) {
      int cmdStart = request.indexOf("cmd=") + 4;
      int cmdEnd = request.indexOf(" ", cmdStart);
      String command = request.substring(cmdStart, cmdEnd);
      controlCar(command);
    }

    // Handle speed change
    if (request.indexOf("GET /control?speed=") >= 0) {
      int speedStart = request.indexOf("speed=") + 6;
      int speedEnd = request.indexOf(" ", speedStart);
      motorSpeed = request.substring(speedStart, speedEnd).toInt();
      analogWrite(ENA, motorSpeed);
      analogWrite(ENB, motorSpeed);
    }

    // Send HTML page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println(htmlPage);
    client.stop();
  }
}

void setup() {
  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Set up Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.begin(115200);
  Serial.println("WiFi Access Point started");
  Serial.print("Connect to WiFi network: ");
  Serial.println(ssid);
  Serial.println("Open a browser and go to: http://192.168.4.1");
}

void loop() {
  handleRequests();
}
