#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "base64.hpp"
#include "AES.h" // Include the AES library
// const char* ssid = "Emre6";
// const char* password = "emreke66";
#define BLOCK_SIZE 16
#define KEY_SIZE 32
#define MAX_NAME 256
#define NETWORK_SIZE 3
struct keyMapping {
  char hostName[MAX_NAME];
  byte key[KEY_SIZE];
};

struct keyMapping keyMappings[NETWORK_SIZE];

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
String byteArrayToString(byte arr[], int size) {
  String result = "";
  for (int i = 0; i < size; i++) {
    if(arr[i] == 0) break;
    result += char(arr[i]);
  }
  return result;
}
bool getKey(const char * targetName,byte * resultKey) {
  for(int i = 0;i<NETWORK_SIZE;i++) {
    if (strcmp(keyMappings[i].hostName,targetName) == 0) {
      memcpy(resultKey, keyMappings[i].key, KEY_SIZE);
      return true;
    }
  }
  return false;
}
void encrypt(String plain,byte * cipher,byte * key,byte * iv) {
    byte plaintext[plain.length()+1];
    plain.getBytes((unsigned char *)plaintext, sizeof(plaintext));
    byte iv_copy[BLOCK_SIZE];
    memcpy(iv_copy,iv,BLOCK_SIZE);
    aes.do_aes_encrypt(plaintext, sizeof(plaintext),cipher, key,KEY_SIZE,iv_copy);
}
String decrypt(byte * cipher,uint32_t size,byte * key,byte * iv) {
  byte plain[size];
  byte iv_copy[BLOCK_SIZE];
  memcpy(iv_copy,iv,BLOCK_SIZE);
  aes.do_aes_decrypt(cipher,aes.get_size(),plain,key,KEY_SIZE,iv);
  return byteArrayToString(plain,size);
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
  _httpServer.send(200, "text/html", htmlContent);
}


void sendColorToOtherDevices(String &color);
void handleConsensus2() {
  if(!_httpServer.hasArg("identifier") || !_httpServer.hasArg("content") || !_httpServer.hasArg("iv")) {
    Serial.println("missing args");
    _httpServer.send(400, "text/plain", "Bad Request");
    return;
  }
  Serial.print("got handle consenys 2 from: ");
  String identifier = _httpServer.arg("identifier");
  Serial.print(identifier);
  String content = _httpServer.arg("content");
  String iv = _httpServer.arg("iv");


  size_t maxDecodedSize = (content.length() * 3) / 4;

  byte contentArray[maxDecodedSize];

  int contentLength = decode_base64((unsigned char *)content.c_str(), contentArray);

  Serial.print("content length");
  Serial.println(contentLength);
    size_t maxDecodedSize2 = (iv.length() * 3) / 4;

  byte ivArray[maxDecodedSize2];

  int ivLength = decode_base64((unsigned char *)iv.c_str(), ivArray);
  Serial.println(identifier);

  byte targetKey[KEY_SIZE];
  if(!getKey(identifier.c_str(),targetKey)) {
    Serial.print("could not find key of hostname: ");
    Serial.println(identifier);
    _httpServer.send(400, "text/plain", "Bad Request");
    return;
  }
  String contentS = decrypt(contentArray,contentLength,targetKey,ivArray);
  Serial.print("decrypted: ");
  Serial.println(contentS);
    // Check for the "BLUE" message
    if (contentS.indexOf("BLUE") != -1)
    {
      // BLUE LED
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set BLUE variable to 1");
    }
    else if (contentS.indexOf("RED") != -1)
    {
      // RED LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set RED variable to 1");
    }
    else if (contentS.indexOf("GREEN") != -1)
    {
      // GREEN LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      Serial.println("Set GREEN variable to 1");
    }
}
void handleConsensus()
{
  if (_httpServer.hasArg("color"))
  {
    String selectedColor = _httpServer.arg("color");
    sendColorToOtherDevices(selectedColor);
    Serial.println("Original Color: " + selectedColor);
    if (selectedColor == "BLUE")
    {
      // BLUE LED
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set BLUE variable to 1");
    }
    else if (selectedColor == "RED")
    {
      // RED LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      Serial.println("Set RED variable to 1");
    }
    else if (selectedColor == "GREEN")
    {
      // GREEN LED
      digitalWrite(bluePin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      Serial.println("Set GREEN variable to 1");
    }
     _httpServer.send(200, "text/plain", "setted color");
     return;
  }
   _httpServer.send(400, "text/plain", "Bad Request");

}

void fillIv(byte * iv, int size) {
  for (int i = 0; i < size; i++) {
    iv[i] = random(256); 
  }
}


void sendColorToOtherDevices(String &color)
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
        String payload = "identifier=" + String(hostname) + String(ESP.getChipId())+".local" + "&content=";
        byte ciphered_payload[color.length() + 1];
        byte targetKey[KEY_SIZE];
        if(!getKey(MDNS.hostname(i).c_str(),targetKey)) {
          Serial.print("could not find key of hostname: ");
          Serial.println(MDNS.hostname(i));
          continue;
        }
        byte iv[BLOCK_SIZE];
        fillIv(iv,BLOCK_SIZE);
        encrypt(color,ciphered_payload,targetKey,iv);
        byte cipherEncoded[sizeof(ciphered_payload) * 4 / 3 + 1];
      encode_base64(ciphered_payload,sizeof(ciphered_payload),cipherEncoded);
              byte ivEncoded[sizeof(iv) * 4 / 3 + 1];
      encode_base64(iv,sizeof(iv),ivEncoded);
        payload+=byteArrayToString(cipherEncoded,sizeof(cipherEncoded));
        payload+="&iv=";
        payload+=byteArrayToString(ivEncoded,sizeof(ivEncoded));
        Serial.println(payload);
        client.print("POST /consensus2 HTTP/1.1\r\n");
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



  snprintf(keyMappings[0].hostName,MAX_NAME,"MyESP15330671.local");
  snprintf(keyMappings[1].hostName,MAX_NAME,"MyESP12889460.local");
  snprintf(keyMappings[2].hostName,MAX_NAME,"MyESP15330598.local");

  byte key[]="12345678901234561234567890123456";

  memcpy(keyMappings[0].key,key,KEY_SIZE);
  memcpy(keyMappings[1].key,key,KEY_SIZE);
  memcpy(keyMappings[2].key,key,KEY_SIZE);

  for(int i  =0;i<NETWORK_SIZE;i++) {
    Serial.print("mapping host "+String(i) + " ");
    Serial.println(keyMappings[i].hostName);
    Serial.print("mapping key of " + String(i) + " " );
    Serial.write(keyMappings[i].key,KEY_SIZE);
    Serial.println("");
  }
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
  _httpServer.on("/consensus2", HTTP_POST, handleConsensus2);
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

  String s = "sdfghjkjhgfewrtyhujukjhgfdsdfghjk";
  byte cipher[s.length()];
  byte iv[BLOCK_SIZE];
  fillIv(iv,BLOCK_SIZE);
  encrypt(s,cipher,keyMappings[0].key,iv);
  Serial.print("cipher array: ");
  Serial.write(cipher,sizeof(cipher));
  Serial.println("");
  byte cipherEncoded[sizeof(cipher) * 4 / 3 + 1];
  encode_base64(cipher,sizeof(cipher),cipherEncoded);
  size_t maxDecodedSize = (sizeof(cipherEncoded)* 3) / 4;
  byte contentArray[maxDecodedSize];

  decode_base64(cipherEncoded,contentArray);
  //contentArray[maxDecodedSize - 1] = '\0';
  Serial.print("decoded cipher");
  Serial.write(contentArray,maxDecodedSize);
  Serial.println("");
  String decryyed = decrypt(cipher,sizeof(cipher),keyMappings[0].key,iv);
  Serial.print("decryerd: ");
  Serial.println(decryyed);

  // Connect to the server
  // connectToServer();

  // decrypt_with_aes_cbc();
}

void loop()
{
  MDNS.update();
  _httpServer.handleClient();
    int n = MDNS.queryService("http", "tcp");
  // Check for server messages periodically
  // checkServerMessages();

  // Your other loop logic can go here
  delay(1000);
}