#include <ESP8266WiFi.h>




const char* ssid = "Emre6";
const char* password = "emreke66";
const char* server_ip = "172.20.10.2";  // Replace with the IP address of your server
const int server_port = 8080;

int BLUE = 0;  // Variable to store the state based on the server message

WiFiClient client;

const int bluePin = D0; // make big led blue : HIGH --> OPEN , LOW --> CLOSE
const int emptyPin = D1; // DOES NOTHING
const int emptyPin2 = D2; // DOES NOTHING
const int greenPin = D3; // make big led green : LOW --> OPEN, HIGH --> CLOSE
const int redPin = D4; // make big led red : LOW --> OPEN, HIGH --> CLOSE










void connectToServer() {
  Serial.println("Connecting to server...");
  if (client.connect(server_ip, server_port)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection to server failed!");
  }
}

void sendToServer(String message) {
  if (client.connected()) {
    Serial.println("Sending message to server...");
    client.println(message);
  } else {
    Serial.println("Not connected to server. Reconnecting...");
    connectToServer();
    if (client.connected()) {
      Serial.println("Sending message to server...");
      client.println(message);
    } else {
      Serial.println("Failed to reconnect to server!");
    }
  }
}

void checkServerMessages() {

  if (client.available()) {
    String response = client.readStringUntil('\n');
    Serial.print("Received message from server: ");
    Serial.println(response);

    // Check for the "BLUE" message
    if (response.equals("BLUE")) {
       //BLUE LED
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set BLUE variable to 1");
    } 
    else if (response.equals("RED")) {
      //RED LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set RED variable to 1");
    }
    else if (response.equals("GREEN")) {
      //GREEN LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      Serial.println("Set GREEN variable to 1");
    }
    else {
      Serial.println("Message was: " + response);
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);

  // Connect to the server
  connectToServer();

  //decrypt_with_aes_cbc();
}

void loop() {
  // Check for server messages periodically
  checkServerMessages();

  // Your other loop logic can go here
  delay(1000);
}