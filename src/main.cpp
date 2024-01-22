#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "base64.hpp"
#include "BigNumber.h"
#include "AES.h" // Include the AES library
// const char* ssid = "Emre6";
// const char* password = "emreke66";
#define BLOCK_SIZE 16
#define KEY_SIZE 16
#define CIPHER_SIZE 33
#define MAX_NAME 256
#define NETWORK_SIZE 3
struct keyMapping {
  char hostName[MAX_NAME];
  byte key[KEY_SIZE];
};

struct RsaKeyMapping {
  char hostName[MAX_NAME];
  BigNumber key = 0;
};

BigNumber public_modulus = BigNumber("211881535355297736509781884103072553991");
BigNumber drone_1_pub = BigNumber("124636197267822197929422428643519936353");
BigNumber drone_2_pub = BigNumber("110546888011459688598270328014252465287"); // 23
BigNumber drone_3_pub = BigNumber("88853547084479695943233408807154535271"); // 31

BigNumber drone_1_pri = 17;

struct keyMapping keyMappings[NETWORK_SIZE];

struct RsaKeyMapping RSAKeyMappings[NETWORK_SIZE];

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
  aes.do_aes_decrypt(cipher,size,plain,key,KEY_SIZE,iv_copy);
  Serial.write(plain,size);
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

int roundUpToMultipleOf4(int value) {
    return ((value + 3) / 4) * 4;
}

BigNumber from_bytes_big_endian(const unsigned char * bytes) {
    BigNumber result(0);
    for (size_t i = 0; i < 16; ++i) {
        result = result *  BigNumber(256) + BigNumber(bytes[i]);
    }
    return result;
}

void big_endian_to_bytes(const BigNumber &number, unsigned char *bytes) {
  BigNumber temp = number;
  for (size_t i = 15; i < 16; --i) {
    bytes[i] = (temp % BigNumber(256));
    temp = temp / BigNumber(256);
  }
}

BigNumber rsa_encrypt(BigNumber plaintext, BigNumber public_key, BigNumber modulus)
{
  return plaintext.powMod(public_key, modulus);
}

BigNumber rsa_decrypt(BigNumber ciphertext, BigNumber private_key, BigNumber modulus)
{
  return ciphertext.powMod(private_key, modulus);
}

void requestAesKeyPair(String other_hostname,IPAddress ipaddress) {
  WiFiClient client;

  Serial.print("other hostnamae: ");
  Serial.println(other_hostname);
  String payload = "identifier=" + String(hostname) + String(ESP.getChipId())+".local";
  if(client.connect(ipaddress,80)) {
  client.print("POST /creatersa HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(other_hostname.c_str());
  client.print("\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: ");
  client.print(payload.length());
  client.print("\r\n\r\n");
  client.print(payload);
  Serial.println("Asked for aes key");
  while(client.connected() && !client.available()) {
    delay(100);
  }
  Serial.println("client availabel");
  Serial.println(client.available());

    String line = client.readString();
    int index = line.indexOf("\r\n\r\n");
    String encoded = line.substring(index + 4,line.length()-1);
    Serial.print("encoded: ");
    Serial.println(encoded);
    size_t maxDecodedSize2 = (encoded.length() * 3) / 4;

  byte aesArray[maxDecodedSize2];

  int aesLength = decode_base64((unsigned char *)encoded.c_str(),encoded.length(), aesArray);
  BigNumber aesNumber = rsa_decrypt(from_bytes_big_endian(aesArray),drone_1_pri,public_modulus);
  byte aesByte[KEY_SIZE];
  big_endian_to_bytes(aesNumber,aesByte);
     Serial.print("aes hex: ");
  for (int i = 0; i < 16; ++i) {
    Serial.print(aesByte[i], HEX);
    Serial.print(" ");
  }
  for(int i  =0;i<NETWORK_SIZE;i++) {
    if(strlen(keyMappings[i].hostName) == 0) {
      Serial.print("Writing key for the hostname: ");
      Serial.print(other_hostname);
      Serial.print(" at position: ");
      Serial.println(i);
      snprintf(keyMappings[i].hostName,MAX_NAME,other_hostname.c_str());
      memcpy(keyMappings[i].key,aesByte,KEY_SIZE);
      break;
    }
  }
  
  client.stop();
  }
}



void sendColorToOtherDevices(String &color);
void handleConsensus2() {

  String plain = _httpServer.arg("plain");
  Serial.println(plain);
  int currentIndex = 0;
  String identifier = "",iv = "",content = "";
  while (currentIndex < plain.length()) {
    // Find the position of the next '&'
    int nextAmpersand = plain.indexOf('&', currentIndex);

    // Extract the current key-value pair
    String keyValuePair;
    if (nextAmpersand == -1) {
      keyValuePair = plain.substring(currentIndex);
      currentIndex = plain.length();
    } else {
      keyValuePair = plain.substring(currentIndex, nextAmpersand);
      currentIndex = nextAmpersand + 1;
    }

    // Split the key-value pair into key and value
    int equalsIndex = keyValuePair.indexOf('=');
    if (equalsIndex != -1) {
      String key = keyValuePair.substring(0, equalsIndex);
      String value = keyValuePair.substring(equalsIndex + 1);

      // Assign the value to the corresponding variable based on the key
      if (key.equals("identifier")) {
        identifier = value;
      } else if (key.equals("content")) {
        content = value;
      } else if (key.equals("iv")) {
        iv = value;
      }
    }
  }
  Serial.print("got handle consenys 2 from: ");
  Serial.println(identifier);
  Serial.print("content: ");
  Serial.println(content);
  Serial.print("iv: ");
  Serial.println(iv);
  size_t maxDecodedSize = (content.length() * 3) / 4;
  //maxDecodedSize = roundUpToMultipleOf4(maxDecodedSize) + 1;

  byte contentArray[maxDecodedSize];

  int contentLength = decode_base64((unsigned char *)content.c_str(),content.length(), contentArray);
  //contentArray[maxDecodedSize -1] = '\n';


  Serial.print("content buffer: ");
  Serial.write(contentArray,sizeof(contentArray));
  Serial.println("");


    size_t maxDecodedSize2 = (iv.length() * 3) / 4;
    //maxDecodedSize2 = roundUpToMultipleOf4(maxDecodedSize2) + 1;

  byte ivArray[maxDecodedSize2];

  int ivLength = decode_base64((unsigned char *)iv.c_str(),iv.length(), ivArray);
  if(ivLength != BLOCK_SIZE) {
    Serial.println("ANAANI SIKIM");
  }
  Serial.print("iv hex: ");
  for (int i = 0; i < 16; ++i) {
    Serial.print(ivArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");

    Serial.print("cipher hex: ");
  for (int i = 0; i < 16; ++i) {
    Serial.print(contentArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  //ivArray[maxDecodedSize2 - 1] = '\n';
  byte targetKey[KEY_SIZE];
  if(!getKey(identifier.c_str(),targetKey)) {
    Serial.println("Creating pair");
    //requestAesKeyPair(identifier);

    Serial.print("could not find key of hostname: ");
    Serial.println(identifier);
    _httpServer.send(400, "text/plain", "Bad Request");
    return;
  }
  String contentS = decrypt(contentArray,sizeof(contentArray),targetKey,ivArray);
  Serial.print("decrypted: ");
  Serial.println(contentS);
    // Check for the "BLUE" message
    if (contentS.indexOf("BLU") != -1)
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
  int maxAttempts = 5;
  int attempt = 0;
  while(attempt < maxAttempts) {
    int n = MDNS.queryService("http", "tcp");
    int send = 0;
    for (int i = 0; i < n; ++i)
    {
      if (MDNS.IP(i) != WiFi.localIP())
      { // Skip sending to itself
        // Create a client to send data to other devices
        WiFiClient client;
       
          String payload = "identifier=" + String(hostname) + String(ESP.getChipId())+".local" + "&content=";
          byte ciphered_payload[CIPHER_SIZE];
          byte targetKey[KEY_SIZE];
          if(!getKey(MDNS.hostname(i).c_str(),targetKey)) {
            requestAesKeyPair(MDNS.hostname(i),MDNS.IP(i));
            if(!getKey(MDNS.hostname(i).c_str(),targetKey)) {
              Serial.print("Could not find key of hostname: ");
              Serial.println(MDNS.hostname(i));
              continue;
            }
          }
          aes.setPadMode((paddingMode) 0);
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
        if (client.connect(MDNS.IP(i), 80))
        {
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
          while(client.connected() && !client.available()) {
            delay(100);
          }
          client.stop();
          send++;
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
    if(send > 0) break;
    else attempt++;
  }
}

const unsigned char* generateKey() {
    String numericString;
    for (int i = 0; i < 32; ++i) {
        numericString += aes.getrandom() % 10 + '0';
    }
    return (unsigned char*) numericString.c_str();
}




void send_message(int target_ssid, byte* result) {
  const unsigned char* key_ptr = generateKey();
  byte key[17];
  memcpy(key, key_ptr, sizeof(key_ptr));

  for (int i = 0; i < 16; ++i) {
    Serial.print(key[i], HEX);
    Serial.print(" ");
  }

  memcpy(keyMappings[target_ssid].key,key,KEY_SIZE);

  BigNumber aes_key_big = from_bytes_big_endian(key);
  BigNumber cur_public =  RSAKeyMappings[target_ssid].key;
  BigNumber cnum_key = rsa_encrypt(aes_key_big, cur_public, public_modulus);

  unsigned char ctext_key[16];
  big_endian_to_bytes(cnum_key, ctext_key); 
 
  byte cipherEncoded[sizeof(ctext_key) * 4 / 3 + 1];
  encode_base64(ctext_key,sizeof(ctext_key),cipherEncoded);
  
  for (int i = 0; i < 23; i++) {
    result[i] = cipherEncoded[i];
  }
}


void createPairwiseAesKey() {
  byte cur_encoded[22]; 

  String identifier = _httpServer.arg("identifier");
  for (int i = 0; i < NETWORK_SIZE; i++) {
    if (strcmp(keyMappings[i].hostName, identifier.c_str()) == 0) {
      send_message(i, cur_encoded);
      break;
    }
  }

  _httpServer.send(200, "text/plain", (const char*) cur_encoded);
}

void setup()
{
  Serial.begin(115200);

  BigNumber::begin();


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

  snprintf(RSAKeyMappings[0].hostName,MAX_NAME,"MyESP15330671.local");
  snprintf(RSAKeyMappings[1].hostName,MAX_NAME,"MyESP12889460.local");
  snprintf(RSAKeyMappings[2].hostName,MAX_NAME,"MyESP15330598.local");

  RSAKeyMappings[0].key = drone_1_pub;
  RSAKeyMappings[1].key = drone_1_pub;
  RSAKeyMappings[2].key = drone_1_pub;

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
  _httpServer.on("/creatersa", HTTP_POST, createPairwiseAesKey);
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