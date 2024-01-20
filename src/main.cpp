#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServerSecure.h>
#include <CertStoreBearSSL.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiClientSecure.h>

//const char* ssid = "Emre6";
//const char* password = "emreke66";
const char* server_ip = "172.20.10.7";  // Replace with the IP address of your server
const int server_port = 8080;

int BLUE = 0;  // Variable to store the state based on the server message

WiFiClient client;
const int bluePin = D0; // make big led blue : HIGH --> OPEN , LOW --> CLOSE
const int emptyPin = D1; // DOES NOTHING
const int emptyPin2 = D2; // DOES NOTHING
const int greenPin = D3; // make big led green : LOW --> OPEN, HIGH --> CLOSE
const int redPin = D4; // make big led red : LOW --> OPEN, HIGH --> CLOSE
const char *ssid = "ESPSSID";
const char *password = "asdqwe123";
const char *hostname = "MyESP";

const char cert[] = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDhTCCAm2gAwIBAgIUcCRI2XD0CQVSD5s9aQCjd2xBXgAwDQYJKoZIhvcNAQEL
BQAwUjELMAkGA1UEBhMCVFIxDDAKBgNVBAgMA0lTVDEOMAwGA1UEBwwFVFVaTEEx
EDAOBgNVBAoMB1NBQkFOQ0kxEzARBgNVBAsMClVOSVZFUlNJVFkwHhcNMjQwMTIw
MTYzMzU4WhcNMjUwMTE5MTYzMzU4WjBSMQswCQYDVQQGEwJUUjEMMAoGA1UECAwD
SVNUMQ4wDAYDVQQHDAVUVVpMQTEQMA4GA1UECgwHU0FCQU5DSTETMBEGA1UECwwK
VU5JVkVSU0lUWTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKpXBkN5
eFUsRUX1uYW0Z0IwvPSn+FdivYqwyWsGUDp14wfYCKoISvZhtlyizZzYaY3y6UW4
XMXmfiykSuYSAbQx1ZPAZ8mr9sfiPST3IOdRHpERBGmuMeBeZidf9gioQ0lbmL8c
d3kcZd3bLefrjjUVZY+RQK76i+A6CFtQdPxggvsP2nAbdeNMLYIKjx2eLBQerFtt
kc6sfqtgZtnfth7hx24yFUVqWsZ24UN77f1jCsv4689HEVa4fyzi17zG+GMZFxi6
paFLMRGqCpAe1ubrmakbr3JsIUYwTFMJulg439g8NSjBbzA9qgYUVJcuEochDGyG
YzOIbuDRE8Vp+OcCAwEAAaNTMFEwHQYDVR0OBBYEFF609Q26qmochB3JsQD6+4HB
0Zl3MB8GA1UdIwQYMBaAFF609Q26qmochB3JsQD6+4HB0Zl3MA8GA1UdEwEB/wQF
MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAJ0meDN3mwjCnBrLtJcv5cDjB34vhgL0
KnExoR9aHd2xo7wRDZsIiJas5wuKyOyGC5t/cIQDBkRRcpNGZNpaBDKbgeCTZv33
PFPUG88QIlvg3lKuk3Ok3FRq0POub/QJ4JF++if5gDCInEweDFZ/10EtLXqig5/d
jsOkIaKn+N7sVDbWbi7FsSlqqG4HwKsN+h+c8+N6Kaj0OQfpK38seNuXWDWh4E19
YzihC3NG63b4PQrCRiTwOXHWQTQYwahMo0tOE9FZkQ8pNiKnL/FyYRBeBBEK3m84
56SfwbFATaNvK+e+cpvF6f3GnJr1G3ZWP4+twOncs+wRLhtAi1Z3zrI=
-----END CERTIFICATE-----
)EOF";

const char privateKey[] = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCqVwZDeXhVLEVF
9bmFtGdCMLz0p/hXYr2KsMlrBlA6deMH2AiqCEr2YbZcos2c2GmN8ulFuFzF5n4s
pErmEgG0MdWTwGfJq/bH4j0k9yDnUR6REQRprjHgXmYnX/YIqENJW5i/HHd5HGXd
2y3n6441FWWPkUCu+ovgOghbUHT8YIL7D9pwG3XjTC2CCo8dniwUHqxbbZHOrH6r
YGbZ37Ye4cduMhVFalrGduFDe+39YwrL+OvPRxFWuH8s4te8xvhjGRcYuqWhSzER
qgqQHtbm65mpG69ybCFGMExTCbpYON/YPDUowW8wPaoGFFSXLhKHIQxshmMziG7g
0RPFafjnAgMBAAECggEADurWxkCfBHvUso8VvjB8Un7zgqGAeCFSOWZzXTINA0b2
U5DcbAzp3lspneyMASOwABm5IBKd3UmVhydO1TMtKbCrjuhCA+v8DVPcyEie0w3M
QCazXOXqbEcYFHQCQePVG1mFg5xkl3CgjdCiBot0cBBS9+21MnMH840MKk2ze7PQ
q5dQ/OU0ImWDFGNToPsQXG0NXtBEgrDD0hAmDMGoz3qVJ6wV8hQpTYGWxz2yL12S
maxGb0m+8sb7ou9PY3H6vus3VBoPD8JN1nikXJ6YtKW8LHdceucUnxGFCgtbrWSO
U1VAgokm4MmQjSFckIzs3qaZ4H+QCsSbadZKhTuvMQKBgQDhU9KP0s/rLk3HIPry
58FkY5BEoJk4liet/yvJv+Zh8+ZTo4rghq+RLlB7nqq+IvKmqB0mzlwCOwUsXQ1s
rIvAy19atTM4hQQodR1Jc0Yd/PDuRMCENM42QPQoBUSjdR+AIfB1Fk3L5Atc5vuA
yVACoiIOk356l0J8Cff1kfw/iwKBgQDBhwF+4aZ9Md9Rr+dRFS5YrU+66IVHsW3l
NpmIodonmhMQfZxHtaAkPtd+b8avlTlC9U/TWyFRN+xeeN9GuL7Z8VKhCDTTh811
tZD/zNAom4t6fxwdjNqhkKmJdNr83TNvejAaPfiV3AqDiaVnkWBztWHPxFRlZidm
u0u0sBqXlQKBgEsDluPesO2AutACQMlBMnbzVoIiXSAjjQ42FyPRSSdvBUzu8zsQ
W76fhpfBGOK3y8SDz0CuDp+Heo9sYx28y8HhcttF28mDLwExDBD52Wv+W09XgCgD
VGtncGEqb9FjGpuTsPOp+zhXsRi6pRwY6RQXWfM0UzUIyk91GiGc+jdFAoGBAJXW
p8vLfpJ0aMngfiMDWU0DczXuDjOCJupPqfRmDHiZDKW5+6Rx9bnahhK5DIap7Rnw
wYuNhXBS8kn7IDSrmek4tZdEuVIGVYgLumaz939ZX1bKz8P5aNWrIfjxZxRdWBO5
8UCANtYg9mr9yfk0UA+GomKxSf7wZxVdcrJFn3E5AoGAFl2D2/hJwikoA5EeXe9p
zUA3UsU5uCFuk1YFx4oUUZ9OQznWWYUI6+vRObxahSnQz8ESeMnEQRRbnTReU+3W
BadNwtpFY5+VbE6kvf5FPD/RJU1hRXKF/UpUqLai3aC5xKHAP5V+ww+k9F7hsXaW
LpUNcoRJCOK2xR6VlBAoS/c=
-----END PRIVATE KEY-----
)EOF";

ESP8266WebServerSecure _httpServer(443);
  ESP8266HTTPUpdateServer _httpUpdater;

  BearSSL::X509List certList(cert);
BearSSL::PrivateKey privateKeyObject(privateKey);



String readFile(const char *path) {
    // Open the file for reading
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return String();
    }

    // Read the content of the file
    String content = file.readString();

    // Close the file
    file.close();

    return content;
}


void handleRoot() {
   String htmlContent = readFile("/index.html");
  Serial.println("Got request to webserver");
    _httpServer.send(200, "text/html", htmlContent);
}

void handleAP() {
   String htmlContent = readFile("/apchange.html");
  Serial.println("Got request to webserver");
    _httpServer.send(200, "text/html", htmlContent);
}

void connectAP(String newSSID,String newPassword) {
   WiFi.softAPdisconnect(true);
    WiFi.enableAP(false);
    delay(2000);
    WiFi.enableSTA(true);
    WiFi.mode(WIFI_STA);
    time_t timeout = 0;
    WiFi.begin(newSSID.c_str(), newPassword.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        timeout += 250;
        if(timeout == 10000) {
          break;
        }
        Serial.print("Connecting to New AP status: ");
        Serial.println(WiFi.status());
    }
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("couldnt connect to "+newSSID);
      WiFi.mode(WIFI_AP);
      WiFi.softAPdisconnect(false);
      WiFi.softAP(ssid + String(ESP.getChipId()),password);
      Serial.print("AP IP address: ");
      Serial.println(WiFi.softAPIP());
    }
    else {
      WiFi.softAPdisconnect(true);
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
    }
}
void persistCredentials(const char *ssid, const char *password) {
    // Save the WiFi credentials to a file
    File file = SPIFFS.open("/wifi_credentials.txt", "w");
    if (file) {
        file.println(ssid);
        file.println(password);
        file.close();
    } else {
        Serial.println("Failed to open file for writing");
    }
}
void handleSave() {
    String newSSID = _httpServer.arg("ssid");
    String newPassword = _httpServer.arg("password");
    _httpServer.send(200, "text/plain", "connecting to "+ newSSID);
    delay(100);
    connectAP(newSSID,newPassword);
    persistCredentials(newSSID.c_str(),newPassword.c_str());
    
   
}
void consensusPage() {
   String htmlContent = readFile("/consensus.html");
   htmlContent.replace("Device",String(ESP.getChipId()));
    _httpServer.send(200, "text/html", htmlContent);
}
void sendColorToOtherDevices(const String &color);
void handleConsensus() {
    if (_httpServer.hasArg("color")) {
        String selectedColor = _httpServer.arg("color");
        String identifier = _httpServer.arg("identifier");
        // Process the selected color (update LED, store in EEPROM, etc.)
        // ...

        // Send the selected color to other ESP8266 devices in the network
        if(identifier == String(ESP.getChipId())) {
          sendColorToOtherDevices(selectedColor);
          _httpServer.send(200, "text/plain", "color Consensus initiated");
        }
        else {
          Serial.println("got color change request from " +identifier +" color is "+selectedColor);
        }
        if (selectedColor.equals("BLUE")) {
       //BLUE LED
          digitalWrite(bluePin, HIGH);
          digitalWrite(redPin, HIGH);
          digitalWrite(greenPin, HIGH);
          Serial.println("Set BLUE variable to 1");
        } 
        else if (selectedColor.equals("RED")) {
          //RED LED
          digitalWrite(bluePin, LOW);
          digitalWrite(redPin, LOW);
          digitalWrite(greenPin, HIGH);
          Serial.println("Set RED variable to 1");
        }
        else if (selectedColor.equals("GREEN")) {
          //GREEN LED
          digitalWrite(bluePin, LOW);
          digitalWrite(redPin, HIGH);
          digitalWrite(greenPin, LOW);
          Serial.println("Set GREEN variable to 1");
        }

    }
}

void sendColorToOtherDevices(const String &color) {
    int n = MDNS.queryService("http", "tcp");
    for (int i = 0; i < n; ++i) {
        if (MDNS.IP(i) != WiFi.localIP()) { // Skip sending to itself
            // Create a client to send data to other devices
              WiFiClientSecure client;
            if (client.connect(MDNS.IP(i), 443)) {
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
            else {
              Serial.print("could not connect to");
              Serial.println(MDNS.hostname(i));
            }
        }
    }
}


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
  if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
    File file = SPIFFS.open("/wifi_credentials.txt", "r");
    if (file) {
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
        connectAP(newSSID,newPassword);
    } else {
        Serial.println("There is no persisted credentials");
        WiFi.softAP(ssid + String(ESP.getChipId()), password);
        Serial.println("Access Point started");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    }
    WiFi.hostname(hostname);
    WiFi.onWiFiModeChange([](const WiFiEventModeChange & mode) {
      
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
        }
    });
    //WiFi.mode(WIFI_AP_STA);

    _httpServer.getServer().setRSACert(&certList, &privateKeyObject);
    _httpServer.on("/", HTTP_GET, handleRoot);
    _httpServer.on("/config", HTTP_GET, handleAP);
    _httpServer.on("/save", HTTP_POST, handleSave);
     _httpServer.on("/consensuspage", HTTP_GET, consensusPage);
     _httpServer.on("/consensus", HTTP_POST, handleConsensus);
    _httpServer.begin();
    
    while (!MDNS.begin(hostname + String(ESP.getChipId()))) {
        delay(250);
    }

    Serial.println("mDNS responder started");

    //_httpUpdater.setup(&_httpServer.getServer());

    MDNS.addService("http", "tcp", 443);

   

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);

  // Connect to the server
  //connectToServer();

  //decrypt_with_aes_cbc();
}

void loop() {
      MDNS.update();
    _httpServer.handleClient();
  // Check for server messages periodically
  //checkServerMessages();

  // Your other loop logic can go here
  delay(1000);
}