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

// Protocol
const byte STX = 0x02;
const byte ETX = 0x03;
const byte ACK = 0x06;
const byte NACK = 0x15;
const byte FRAME_LENGTH = 11;

// Bluetooth
const byte BT_BUFFER_LENGTH = 100;
SofwareSerial ble(2, 3);
char btBuffer[BT_BUFFER_LENGTH];
const int btTimeout = 800;
const long btBauds[] = { 9600, 57600, 115200, 38400, 2400, 4800, 19200 };

// Ethernet
bool isServer = false;  // !!! CHANGE THIS !!!
unsigned long previous_millis = 0UL;
unsigned long interval = 5000UL;

const int serverPort = 4080;
const byte mac[6] = { 0xA8, 0x61, 0x0A, 0xAE, 0xDD, 0xFF };  // !!! CHANGE THIS !!!
IPAddress ip(192, 168, 1, 11);                               // !!! CHANGE THIS !!!

// Client:
IPAddress serverAddress(192, 168, 1, 12);  // !!! CHANGE THIS !!!
EthernetClient client;

// Server:
EthernetServer server(serverPort);

long BLEAutoBaud() {
	int baudcount = sizeof(btBauds) / sizeof(long);
	for (int i = 0; i < baudcount; i++) {
		for (int x = 0; x < 3; x++) {  // test at least 3 times for each baud
			Serial.print("Test la vitesse ");
			Serial.println(bauds[i]);
			ble.begin(bauds[i]);
			if (BLEIsReady()) {
				return bauds[i];
			}
		}
	}
	return -1;
}

boolean BLEIsReady() {
	BLECmd(timeout, "AT", buffer);  // Send AT and store response to buffer
	if (strcmp(buffer, "OK") == 0) {
		return true;
	} else {
		return false;
	}
}

boolean BLECmd(long timeout, char* command, char* temp) {
	long endtime;
	boolean found = false;
	endtime = millis() + timeout;    //
	memset(temp, 0, BUFFER_LENGTH);  // clear buffer
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
			if (i >= BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
			delay(1);                       // give it a 2ms delay before reading next character
		}
		Serial.print("HM-10 BLE répond  = ");
		Serial.println(temp);
		return true;
	} else {
		Serial.println("HM-10 BLE timeout!");
		return false;
	}
}


void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  if (isServer) {
    Serial.println("RUNNING AS SERVER");
    Serial.print("  > With IP address: ");
    Serial.println(Ethernet.localIP());

    server.begin();
  } else {
    Serial.println("RUNNING AS CLIENT");

    if (client.connect(serverAddress, serverPort)) {
      Serial.println("Connected to server");
    } else {
      Serial.print("Failed to connect to server (Trying with: ");
      Serial.print(serverAddress);
      Serial.println(")");
    }
  }
}

//
// Utils functions
//

/// Read bytes from the provided client
void readCommand(EthernetClient client) {
  byte c;
  byte frame[FRAME_LENGTH];
  byte pos = 0;

  while ((c = client.read()) != 0xFF) {
    frame[pos] = c;
    pos++;
  }

  // for (byte i = 0; i < FRAME_LENGTH; i++) {
  //   Serial.print(frame[i]);
  //   Serial.print(" ");
  // }
  // Serial.println();

  // ... process frame
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
    Serial.print("RECEIVED COMMAND");

    readCommand(_client);

    Ethernet.maintain();
  }
}

void clientLoop() {
  if (!client.connected()) {
    Serial.println("Connection is disconnected");
    client.stop();

    if (client.connect(serverAddress, serverPort)) {
      Serial.println("Reconnected to TCP server");
    } else {
      Serial.println("Failed to reconnect to TCP server, control the serverAddress !");
    }
  } else {
    unsigned long current_millis = millis();

    // If there's data received from the server
    readCommand(client);

    if (current_millis - previous_millis > interval) {
      // sendCommand(client, {}, 0);
      Serial.println("sending");

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
