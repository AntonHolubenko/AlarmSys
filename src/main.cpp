/*
 Repeating Web client

 This sketch connects to a web server and makes a request
 using a WIZnet Ethernet shield. You can use the Arduino Ethernet Shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a WIZnet Ethernet module on board.

 This example uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 19 Apr 2012
 by Tom Igoe
 modified 21 Jan 2014
 by Federico Vanzati

 https://www.arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.

 */

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

// assign a MAC address for the Ethernet controller.
// fill in your address here:
byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);

// initialize the library instance:
EthernetClient client;

char server[] = "siren.pp.ua";// also change the Host line in httpRequest()
char host[] = "Host: siren.pp.ua";
char command[] = "GET /api/v3/alerts/607 HTTP/1.1";
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long stateIntervalSecond = 60;

uint8_t CONTROL_PIN_1 = 13;

void setupEther() {
    // You can use Ethernet.init(pin) to configure the CS pin
    //Ethernet.init(10);  // Most Arduino shields
    //Ethernet.init(5);   // MKR ETH Shield
    //Ethernet.init(0);   // Teensy 2.0
    //Ethernet.init(20);  // Teensy++ 2.0
    //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
    //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet


    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("Ethernet cable is not connected.");
        }
        // try to configure using IP address instead of DHCP:
        Ethernet.begin(mac, ip, myDns);
        Serial.print("My IP address: ");
        Serial.println(Ethernet.localIP());
    } else {
        Serial.print("  DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
}

//void setupWifi() {
//
//}

void setup() {
    // start serial port:
    Serial.begin(9600);
    while (!Serial) { ; // wait for serial port to connect. Needed for native USB port only
    }

    pinMode(CONTROL_PIN_1, OUTPUT);
    setupEther();
//    setupWifi();
}

// this method makes a HTTP connection to the server:
uint8_t httpRequest() {
    // close any connection before send a new request.
    // This will free the socket on the Ethernet shield
    client.stop();

    // if there's a successful connection:
    if (client.connect(server, 443)) {
        Serial.println("connecting...");
        // send the HTTP GET request:
        client.println(command);
        client.println(host);
        client.println("User-Agent: monitor");
        client.println("Connection: close");
        client.println();

        // note the time that the connection was made:
        lastConnectionTime = millis();

        JsonDocument doc;

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, client);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            client.stop();
            return 1;
        }


        if (doc["activeAlerts"].size() == 0) {
            return 2;
        } else {
            return 3;
        }
        // Extract values
        Serial.println(F("Response:"));
        Serial.println(doc["sensor"].as<const char *>());
        Serial.println(doc["time"].as<long>());
        Serial.println(doc["data"][0].as<float>(), 6);
        Serial.println(doc["data"][1].as<float>(), 6);

        return 1;
    } else {
        // if you couldn't make a connection:
        Serial.println("connection failed");
        return 0;
    }
}




void loop() {
    // if ten seconds have passed since your last connection,
    // then connect again and send data:
    if (millis() - lastConnectionTime > stateIntervalSecond * 1000) {
        switch (httpRequest()) {
            case 0: // request error
                digitalWrite(CONTROL_PIN_1, LOW);
                break;
            case 1: // response error
                digitalWrite(CONTROL_PIN_1, LOW);
                break;
            case 2: // not alarm
                digitalWrite(CONTROL_PIN_1, HIGH);
                break;
            case 3: // alarm
                digitalWrite(CONTROL_PIN_1, HIGH);
                break;
        }
    }

}
