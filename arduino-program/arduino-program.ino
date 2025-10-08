/*==============================================================================================
====                                                                                        ====
====           Ecole professionnelle technique et des métiers - EPTM                        ====
====                                                                                        ====
====                                                                                        ====
================================================================================================
==== ARDUINO ATMEGA2560                                                                     ====
====----------------------------------------------------------------------------------------====
==== Créateur: Gabriel Vannay, Sajidur Rahman                                               ====
==== Classe: MI-IN 4                                                                        ====
==== Date de création: 24.09.2025                                                           ====
==== Date de la dernière modification: 24.09.2025                                           ====
====----------------------------------------------------------------------------------------====
==== Description du programme:                                                              ====
==== -------------------------                                                              ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
====                                                                                        ====
==============================================================================================*/

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

//
// Vars / Setup
//

// Protocol variables
const byte STX = 0x02;
const byte ETX = 0x03;
const byte ACK = 0x06;
const byte NACK = 0x15;

// Bluetooth variables
// Prefixed with 'bt'
const byte BT_BUFFER_LENGTH = 128;
SoftwareSerial ble(2, 3);
char btBuffer[BT_BUFFER_LENGTH];
const int btTimeout = 800;
const long btBauds[] = { 9600, 57600, 115200, 38400, 2400, 4800, 19200 };

// Ethernet variables
// Prefixed with 'et' if similar to bluetooth
bool isServer = false;  // !!! CHANGE THIS !!!
unsigned long previous_millis = 0UL;
unsigned long interval = 5000UL;
const byte ET_FRAME_LENGTH = 136;

const int serverPort = 4080;
const byte mac[6] = { 0xA8, 0x61, 0x0A, 0xAE, 0xDD, 0xFF };  // !!! CHANGE THIS !!!
IPAddress ip(192, 168, 1, 11);                               // !!! CHANGE THIS !!!

// Config for CLIENT:
IPAddress serverAddress(192, 168, 1, 12);  // !!! CHANGE THIS !!!
int tempAddress[4] = {192, 168, 1, 12};
EthernetClient client;

// Config for SERVER:
EthernetServer server(serverPort);
IPAddress clientIp(192, 168, 1, 11);  // !!! CHANGE THIS !!!

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  if (isServer) {
    Serial.println(F("RUNNING AS SERVER"));
    Serial.print(F("  > With IP address: "));
    Serial.println(Ethernet.localIP());

    server.begin();
  } else {
    Serial.println(F("RUNNING AS CLIENT"));

    if (client.connect(serverAddress, serverPort)) {
      Serial.println(F("Connected to server"));
    } else {
      Serial.print(F("Failed to connect to server (Tried with: "));
      Serial.print(serverAddress);
      Serial.println(F(")"));
    }
  }
}

//
// Utils functions
//

long BLEAutoBaud() {
  int baudcount = sizeof(btBauds) / sizeof(long);
  for (int i = 0; i < baudcount; i++) {
    for (int x = 0; x < 3; x++) {  // test at least 3 times for each baud
      Serial.print("Test la vitesse ");
      Serial.println(btBauds[i]);
      ble.begin(btBauds[i]);
      if (BLEIsReady()) {
        return btBauds[i];
      }
    }
  }
  return -1;
}

boolean BLEIsReady() {
  BLECmd(btTimeout, "AT", btBuffer);  // Send AT and store response to buffer
  if (strcmp(btBuffer, "OK") == 0) {
    return true;
  } else {
    return false;
  }
}

boolean BLECmd(long timeout, char* command, char* temp) {
  long endtime;
  boolean found = false;
  endtime = millis() + timeout;       //
  memset(temp, 0, BT_BUFFER_LENGTH);  // clear buffer
  found = true;
  Serial.print("Arduino envoie = ");
  Serial.println(command);
  ble.print(command);

  // The loop below wait till either a response is received or timeout
  // The problem with this BLE Shield is the HM-10 module does not response with CR LF at the end of the response,
  // so a timeout is required to detect end of response and also prevent the loop locking up.

  while (!ble.available()) {
    if (millis() > endtime) {  // timeout, break
      found = false;
      break;
    }
  }

  if (found) {  // response is available
    int i = 0;
    while (ble.available()) {  // loop and read the data
      char a = ble.read();
      // Serial.print((char)a);	// Uncomment this to see raw data from BLE
      temp[i] = a;  // save data to buffer
      i++;
      if (i >= BT_BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
      delay(1);                          // give it a 2ms delay before reading next character
    }
    Serial.print("HM-10 BLE répond  = ");
    Serial.println(temp);
    return true;
  } else {
    Serial.println("HM-10 BLE timeout!");
    return false;
  }
}

byte calculateVerification(byte address[], byte length, byte body[]) {
  byte crc = length;
  for (byte i = 0; i < 4; i++) { crc ^= address[i]; }
  for (byte i = 0; i < length; i++) { crc ^= body[i]; }
  return crc;
}

/// Read the frame and return if it's valid
bool processFrame(EthernetClient _client) {
  byte c;
  byte frame[ET_FRAME_LENGTH];
  byte pos = 0;

  while (_client.available()) {
    int c = _client.read();
    if (c < 0)
      break;
    frame[pos++] = c;
    if (pos >= ET_FRAME_LENGTH)
      break;
  }


  // Check for STX
  if (frame[0] != STX) {
    Serial.println(F("Invalid frame"));
    return false;
  }

  // Get parameters from frame
  byte address[4] = { frame[1], frame[2], frame[3], frame[4] };
  byte length = frame[5];
  byte verification = frame[length + 6];

  byte body[length];
  for (byte i = 0; i < length; i++) {
    body[i] = frame[i + 6];
  }

  const bool isValid = verification == calculateVerification(address, length, body);
  return isValid;
}

/// Read bytes from the provided client
/// Should be used by the server
void readClient(EthernetClient _client) {
  if (processFrame(_client)) {
    Serial.println(F("Valid"));
    byte b[] = { ACK };
    byte ACKframe[] = { STX, clientIp[0], clientIp[1], clientIp[2], clientIp[3], 1, ACK, calculateVerification(clientIp, 1, b), ETX };
    sendCommand(server, ACKframe, 9);
  }
  else {
    Serial.println(F("Not valid"));
    byte b[] = { NACK };
    byte NACKframe[] = { STX, clientIp[0], clientIp[1], clientIp[2], clientIp[3], 1, NACK, calculateVerification(clientIp, 1, b), ETX };
    sendCommand(server, NACKframe, 9);
  }
}

/// Send the provided body to the clients connected to the provided server
void sendCommand(EthernetServer _server, byte body[], size_t length) {
  for (byte i = 0; i < length; i++) {
    Serial.print(body[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  _server.write(body, length);
  _server.flush();
}

/// Send the provided body to the provided client
void sendCommand(EthernetClient _client, byte body[], size_t length) {
  for (byte i = 0; i < length; i++) {
    Serial.print(body[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  _client.write(body, length);
  _client.flush();
}

//
// Loop function
//

void serverLoop() {
  EthernetClient _client = server.available();

  if (_client) {
    Serial.println(F("RECEIVED COMMAND"));

    readClient(_client);

    Ethernet.maintain();
  }
}

void clientLoop() {
  if (!client.connected()) {
    Serial.println(F("Connection is disconnected"));
    client.stop();

    if (client.connect(serverAddress, serverPort)) {
      Serial.println(F("Reconnected to TCP server"));
    } else {
      Serial.println(F("Failed to reconnect to TCP server, control the serverAddress !"));
    }
  } else {
    unsigned long current_millis = millis();

    // If there's data received from the server
    byte c;
    if ((c = client.read()) != 0xFF) {
      Serial.println(c);
    }
    
    else if (current_millis - previous_millis > interval) {
      Serial.println(F("Sending data to server."));
      byte b[] = { 0x01, 0x01, 0x01 };
      byte addr[4] = { serverAddress[0], serverAddress[1], serverAddress[2], serverAddress[3] }; // Workaround to avoid unexpected behaviour with IPAddress class
      byte body[] = {
        0x02, // stx
        0xc0, // addr
        0xa8, // addr
        0x01, // addr
        0x0c, // addr
        0x03, // length
        0x01, // body
        0x01, // body
        0x01, // body
        calculateVerification(addr, 3, b),
        0x03 // etx
        };
      sendCommand(client, body, 11);

      previous_millis = current_millis;
    }
  }
}

void loop() {
  delay(500);
  if (isServer) {
    serverLoop();
  } else {
    clientLoop();
  }
}
