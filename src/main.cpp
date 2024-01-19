#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "AES.h" // Include the AES library

// const char* ssid = "Emre6";
// const char* password = "emreke66";
const char *server_ip = "172.20.10.7"; // Replace with the IP address of your server
const int server_port = 8080;

int BLUE = 0; // Variable to store the state based on the server message

WiFiClient client;
const int bluePin = D0;   // make big led blue : HIGH --> OPEN , LOW --> CLOSE
const int emptyPin = D1;  // DOES NOTHING
const int emptyPin2 = D2; // DOES NOTHING
const int greenPin = D3;  // make big led green : LOW --> OPEN, HIGH --> CLOSE
const int redPin = D4;    // make big led red : LOW --> OPEN, HIGH --> CLOSE
const char *ssid = "ESPSSID";
const char *password = "asdqwe123";
const char *hostname = "MyESP";
AES aes;

ESP8266WebServer _httpServer(80);
ESP8266HTTPUpdateServer _httpUpdater;

String readFile(const char *path)
{
  // Open the file for reading
  File file = SPIFFS.open(path, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return String();
  }

  // Read the content of the file
  String content = file.readString();

  // Close the file
  file.close();

  return content;
}

void handleRoot()
{
  String htmlContent = readFile("/index.html");
  Serial.println("Got request to webserver");
  _httpServer.send(200, "text/html", htmlContent);
}

void handleAP()
{
  String htmlContent = readFile("/apchange.html");
  Serial.println("Got request to webserver");
  _httpServer.send(200, "text/html", htmlContent);
}

void connectAP(String newSSID, String newPassword)
{
  WiFi.softAPdisconnect(true);
  WiFi.enableAP(false);
  delay(2000);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  time_t timeout = 0;
  WiFi.begin(newSSID.c_str(), newPassword.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    timeout += 250;
    if (timeout == 10000)
    {
      break;
    }
    Serial.print("Connecting to New AP status: ");
    Serial.println(WiFi.status());
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("couldnt connect to " + newSSID);
    WiFi.mode(WIFI_AP);
    WiFi.softAPdisconnect(false);
    WiFi.softAP(ssid + String(ESP.getChipId()), password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    WiFi.softAPdisconnect(true);
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
}
void persistCredentials(const char *ssid, const char *password)
{
  // Save the WiFi credentials to a file
  File file = SPIFFS.open("/wifi_credentials.txt", "w");
  if (file)
  {
    file.println(ssid);
    file.println(password);
    file.close();
  }
  else
  {
    Serial.println("Failed to open file for writing");
  }
}
void handleSave()
{
  String newSSID = _httpServer.arg("ssid");
  String newPassword = _httpServer.arg("password");
  _httpServer.send(200, "text/plain", "connecting to " + newSSID);
  delay(100);
  connectAP(newSSID, newPassword);
  persistCredentials(newSSID.c_str(), newPassword.c_str());
}
void consensusPage()
{
  String htmlContent = readFile("/consensus.html");
  htmlContent.replace("Device", String(ESP.getChipId()));
  _httpServer.send(200, "text/html", htmlContent);
}
void sendColorToOtherDevices(const String &color);
void handleConsensus()
{
  if (_httpServer.hasArg("color"))
  {
    String selectedColor = _httpServer.arg("color");
    String identifier = _httpServer.arg("identifier");

    // Process the selected color (update LED, store in EEPROM, etc.)
    // ...
    Serial.println("Original Color: " + selectedColor);

    // Encrypt
    byte plaintext[selectedColor.length()];
    selectedColor.getBytes((unsigned char *)plaintext, selectedColor.length() + 1);

    byte keyArray[] = {'d', 'u', 'y', 'g', 'u', 't', 'u', 'm', 'e', 'r', 0, 0, 0, 0, 0, 0};
    aes.set_key(keyArray, sizeof(keyArray));

    byte iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    byte encryptedColor[selectedColor.length()];
    aes.cbc_encrypt(plaintext, encryptedColor, selectedColor.length(), iv);

    // Print the encrypted color
    Serial.print("Encrypted Color: ");
    for (int i = 0; i < sizeof(encryptedColor); i++)
    {
      if (encryptedColor[i] < 16)
        Serial.print("0");
      Serial.print(encryptedColor[i], HEX);
    }
    Serial.println();

    if (identifier == String(ESP.getChipId()))
    {
      sendColorToOtherDevices(selectedColor);
      _httpServer.send(200, "text/plain", "color Consensus initiated");
    }
    else
    {
      Serial.println("got color change request from " + identifier + " color is " + selectedColor);
    }

    // Decrypt
    byte decryptedColor[selectedColor.length()];
    aes.cbc_decrypt(encryptedColor, decryptedColor, selectedColor.length(), iv);

    // Print the decrypted color
    String decryptedColorString = "";
    for (int i = 0; i < sizeof(decryptedColor); i++)
    {
      if (decryptedColor[i] < 16)
        decryptedColorString += "0"; // Ensure leading zero for small values
      decryptedColorString += String(decryptedColor[i], HEX);
    }
    Serial.print("Decrypted Color: ");
    Serial.println(decryptedColorString);

    if (decryptedColor[0] == 'B' && decryptedColor[1] == 'L' && decryptedColor[2] == 'U' && decryptedColor[3] == 'E')
    {
      // BLUE LED
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set BLUE variable to 1");
    }
    else if (decryptedColor[0] == 'R' && decryptedColor[1] == 'E' && decryptedColor[2] == 'D')
    {
      // RED LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set RED variable to 1");
    }
    else if (decryptedColor[0] == 'G' && decryptedColor[1] == 'R' && decryptedColor[2] == 'E' && decryptedColor[3] == 'E' && decryptedColor[4] == 'N')
    {
      // GREEN LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      Serial.println("Set GREEN variable to 1");
    }
  }
}

void sendColorToOtherDevices(const String &color)
{
  int n = MDNS.queryService("http", "tcp");
  for (int i = 0; i < n; ++i)
  {
    if (MDNS.IP(i) != WiFi.localIP())
    { // Skip sending to itself
      // Create a client to send data to other devices
      WiFiClient client;
      if (client.connect(MDNS.IP(i), 80))
      {
        String payload = "color=" + color + "&identifier=" + String(ESP.getChipId());
        client.print("POST /consensus HTTP/1.1\r\n");
        client.print("Host: ");
        client.print(MDNS.hostname(i));
        client.print("\r\n");
        client.print("Content-Type: application/x-www-form-urlencoded\r\n");
        client.print("Content-Length: ");
        client.print(payload.length());
        client.print("\r\n\r\n");
        client.print(payload);
        client.stop();
        Serial.print("send post to ");
        Serial.println(MDNS.hostname(i));
      }
      else
      {
        Serial.print("could not connect to");
        Serial.println(MDNS.hostname(i));
      }
    }
  }
}

void connectToServer()
{
  Serial.println("Connecting to server...");
  if (client.connect(server_ip, server_port))
  {
    Serial.println("Connected to server");
  }
  else
  {
    Serial.println("Connection to server failed!");
  }
}

void sendToServer(String message)
{
  if (client.connected())
  {
    Serial.println("Sending message to server...");
    client.println(message);
  }
  else
  {
    Serial.println("Not connected to server. Reconnecting...");
    connectToServer();
    if (client.connected())
    {
      Serial.println("Sending message to server...");
      client.println(message);
    }
    else
    {
      Serial.println("Failed to reconnect to server!");
    }
  }
}

void checkServerMessages()
{

  if (client.available())
  {
    String response = client.readStringUntil('\n');
    Serial.print("Received message from server: ");
    Serial.println(response);

    // Check for the "BLUE" message
    if (response.equals("BLUE"))
    {
      // BLUE LED
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set BLUE variable to 1");
    }
    else if (response.equals("RED"))
    {
      // RED LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set RED variable to 1");
    }
    else if (response.equals("GREEN"))
    {
      // GREEN LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      Serial.println("Set GREEN variable to 1");
    }
    else
    {
      Serial.println("Message was: " + response);
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
  File file = SPIFFS.open("/wifi_credentials.txt", "r");
  if (file)
  {
    String newSSID = file.readStringUntil('\n');
    String newPassword = file.readStringUntil('\n');
    newSSID.trim();
    newPassword.trim();
    Serial.println("");
    Serial.print("ssid found ");
    Serial.print(newSSID);
    Serial.println("");
    Serial.print("password ");
    Serial.print(newPassword);
    Serial.println("");
    file.close();
    connectAP(newSSID, newPassword);
  }
  else
  {
    Serial.println("There is no persisted credentials");
    WiFi.softAP(ssid + String(ESP.getChipId()), password);
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  WiFi.hostname(hostname);
  WiFi.onWiFiModeChange([](const WiFiEventModeChange &mode)
                        {
      
        Serial.print("WiFi mode changed to: ");
        switch (mode.newMode) {
            case WIFI_STA:
                Serial.println("Switched to Station mode");
                break;
            case WIFI_AP:
                Serial.println("Switched to Access Point mode");
                break;
            case WIFI_AP_STA:
                Serial.println("Switched to Station + Access Point mode");
                break;
            default:
                Serial.println("Unknown mode");
                break;
        } });
  // WiFi.mode(WIFI_AP_STA);

  _httpServer.on("/", HTTP_GET, handleRoot);
  _httpServer.on("/config", HTTP_GET, handleAP);
  _httpServer.on("/save", HTTP_POST, handleSave);
  _httpServer.on("/consensuspage", HTTP_GET, consensusPage);
  _httpServer.on("/consensus", HTTP_POST, handleConsensus);
  _httpServer.begin();

  while (!MDNS.begin(hostname + String(ESP.getChipId())))
  {
    delay(250);
  }

  Serial.println("mDNS responder started");

  _httpUpdater.setup(&_httpServer);
  _httpServer.begin();

  MDNS.addService("http", "tcp", 80);

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);

  // Connect to the server
  // connectToServer();

  // decrypt_with_aes_cbc();
}

void loop()
{
  MDNS.update();
  _httpServer.handleClient();
  // Check for server messages periodically
  // checkServerMessages();

  // Your other loop logic can go here
  delay(1000);
}