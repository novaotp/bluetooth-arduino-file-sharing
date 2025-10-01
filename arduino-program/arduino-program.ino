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

// Client definition:
IPAddress serverAddress(192, 168, 1, 12);  // !!! CHANGE THIS !!!
EthernetClient client;

// Server definition:
EthernetServer server(serverPort);
IPAddress clientIp(192, 168, 1, 12);  // !!! CHANGE THIS !!!

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

/// Read bytes from the provided client
void readCommand(EthernetClient _client) {
  byte c;
  byte frame[ET_FRAME_LENGTH];
  byte pos = 0;

  while ((c = _client.read()) != 0xFF) {
    frame[pos] = c;
    pos++;
  }

  // Check for STX
  if (frame[0] != STX) {
    Serial.println(F("Invalid frame"));
    return;
  }

  // Get parameters from frame
  byte address[4] = { frame[1], frame[2], frame[3], frame[4] };
  byte length = frame[5];
  byte verification = frame[length + 5];

  byte body[length];
  for (byte i = 0; i < length; i++) {
    body[i] = frame[i + 6];
  }

  const bool isValid = verification == calculateVerification(address, length, body);

  if (isValid) {
    Serial.println(F("Valid"));
    byte ACKframe[] = { STX, clientIp[0], clientIp[1], clientIp[2], clientIp[3], 1, ACK, calculateVerification(clientIp, 1, { ACK }), STX };
    sendCommand(client, ACKframe, 9);
  }
  else {
    Serial.println(F("Not valid"));
    byte NACKframe[] = { STX, clientIp[0], clientIp[1], clientIp[2], clientIp[3], 1, NACK, calculateVerification(clientIp, 1, { NACK }), STX };
    sendCommand(client, NACKframe, 9);
  }
}

/// Send the provided body to the provided client
void sendCommand(EthernetClient client, byte body[], size_t length) {
  client.write(body, length);
  client.flush();
}

//
// Loop function
//

void serverLoop() {
  EthernetClient _client = server.available();

  if (_client) {
    Serial.print(F("RECEIVED COMMAND"));

    readCommand(_client);

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
    readCommand(client);

    if (current_millis - previous_millis > interval) {
      // sendCommand(client, {}, 0);
      Serial.println(F("sending"));

      previous_millis = current_millis;
    }
  }
}

void loop() {
  if (isServer) {
    serverLoop();
  } else {
    clientLoop();
  }
}
