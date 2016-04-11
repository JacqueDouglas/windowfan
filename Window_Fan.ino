
#include <ESP8266WiFi.h>
#include "Adafruit_MCP9808.h"
#include "Adafruit_IO_Client.h"
#include <Wire.h>

//Set i2c pins
#define SDA  4
#define SCL  5

// Configure WiFi access point details.
#define WLAN_SSID  "Bill_Wi_The_Science_Fi"
#define WLAN_PASS  "patagonia"

// Configure Adafruit IO access.
#define AIO_KEY    "d6d03d372983dd61fc84dd3a1a3295092fa796a8"


// Create an ESP8266 WiFiClient class to connect to the AIO server.
WiFiClient client;

// Create an Adafruit IO Client instance.  Notice that this needs to take a
// WiFiClient object as the first parameter, and as the second parameter a
// default Adafruit IO key to use when accessing feeds (however each feed can
// override this default key value if required, see further below).
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);

// Finally create instances of Adafruit_IO_Feed objects, one per feed.  Do this
// by calling the getFeed function on the Adafruit_IO_FONA object and passing
// it at least the name of the feed, and optionally a specific AIO key to use
// when accessing the feed (the default is to use the key set on the
// Adafruit_IO_Client class).
Adafruit_IO_Feed Feed1 = aio.getFeed("bedroomtemp");
Adafruit_IO_Feed Feed2 = aio.getFeed("windowtemp");
Adafruit_IO_Feed Feed3 = aio.getFeed("targettemp");
// Alternatively to access a feed with a specific key:
//Adafruit_IO_Feed testFeed = aio.getFeed("esptestfeed", "...esptestfeed key...");

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempbedroom = Adafruit_MCP9808();
Adafruit_MCP9808 tempwindow = Adafruit_MCP9808();

void setup() {
  Wire.pins(SDA, SCL);
  // Setup serial port access.
  Serial.begin(115200);
  delay(10);
  Serial.println(); Serial.println();
  Serial.println(F("Adafruit IO ESP8266 Window Test!"));

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");  
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  // Initialize the Adafruit IO client class (not strictly necessary with the
  // client class, but good practice).
  aio.begin();

  if (!tempbedroom.begin(0x19)) {
    Serial.println("Couldn't find MCP9808 bedroom!");
    while (1);
  }

  if (!tempwindow.begin(0x18)) {
    Serial.println("Couldn't find MCP9808 window!");
    while (1);
  }

  pinMode(13, OUTPUT);
  Serial.println(F("Ready!"));
}

void loop() {
  int i;

  tempbedroom.shutdown_wake(0);   // Don't remove this line! required before reading temp
  tempwindow.shutdown_wake(0);   // Don't remove this line! required before reading temp

  // Read and print out the temperature, then convert to *F
  float cb = tempbedroom.readTempC();
  float fb = cb * 9.0 / 5.0 + 32;
  Serial.print("Temp Bedroom: "); 
  Serial.print(fb);   
  Serial.print("\n"); 
  // Read and print out the temperature, then convert to *F
  float cw = tempwindow.readTempC();
  float fw = cw * 9.0 / 5.0 + 32;
  Serial.print("Temp Window: "); 
  Serial.print(fw); 
  Serial.print("\n"); 

  FeedData latest = Feed3.receive();
  if (latest.isValid()) {
    Serial.print(F("Received value from feed target: ")); Serial.println(latest);
    
    if (latest.intValue(&i)) {
      Serial.print(F("Value as an int: ")); Serial.println(i, DEC);
      if ((fb - fw) > 1.0 && fb > i) {
        digitalWrite(13, HIGH); 
      }
      else {
        digitalWrite(13, LOW);
      }
    }
  else {
    Serial.println(F("Failed to receive the latest feed bedroom value!"));

    if ((fb - fw) > 1.0 && fb > 65) {
        digitalWrite(13, HIGH); 
      }
      else {
        digitalWrite(13, LOW);
      }
    }    
  }

  if (Feed1.send((int)fb)) {
    Serial.print(F("Wrote value to feed bedroom: ")); Serial.println(fb, DEC);
  }
  else {
    Serial.println(F("Error writing value to feed!"));
  }

  if (Feed2.send((int)fw)) {
    Serial.print(F("Wrote value to feed window: ")); Serial.println(fw, DEC);
  }
  else {
    Serial.println(F("Error writing value to feed!"));
  }

  tempbedroom.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  tempwindow.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  // Now wait 10 more seconds and repeat.
  Serial.println(F("Waiting 60 seconds and then writing a new feed value."));
  delay(60000 * 5);
}
