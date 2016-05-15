/**************************************************************
 * 2016-05-14 gallochri.com
 **************************************************************/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Blynk config
char auth[] = "x";
//WiFi config
char ssid[] = "x";
char pass[] = "x";
//Server config
char server[] = "x";
int port = 8442;
//Mail config
char usermail[] = "x";
//PostESP config
const byte interruptPin = 5;

void emailOnButtonPress()
{
  // *** WARNING: You are limited to send ONLY ONE E-MAIL PER MINUTE! ***

  // Let's send an e-mail when you press the button
  // connected to digital pin 5 on your ESP8266

  int isButtonPressed = !digitalRead(interruptPin); // GPIO5 = D2 Invert state, since button is "Active LOW"

  if (isButtonPressed) // You can write any condition to trigger e-mail sending
  {
    Serial.println("Button is pressed."); // This can be seen in the Serial Monitor
    Blynk.email( usermail, "Subject: You have Mail!", "There's a letter in your postbox!");
    Serial.println("Mail sent.");
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, server, port);

  while (Blynk.connect() == false) {
    Serial.println("Blynk.connect is false");
  }

  Serial.println("Blynk.connect is true.Try to send.");
  Blynk.email(usermail, "PostESP is online", "Ready.");
  Serial.println("Mail sent.");
  // Setting the button
  pinMode(interruptPin, INPUT_PULLUP);
  // Attach pin 5 postbox sensor to our handler
  attachInterrupt(digitalPinToInterrupt(interruptPin), emailOnButtonPress, FALLING);
}

/*/ Keep this flag not to re-sync on every reconnection
bool isFirstConnect = true;
// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
  if (isFirstConnect) {
    // Request Blynk server to re-send latest values for all pins
    Blynk.syncAll();
    // You can also update an individual Virtual pin like this:
    //Blynk.syncVirtual(V0);
    isFirstConnect = false;
  }
  // Let's write your hardware uptime to Virtual Pin 2
  int value = millis() / 1000;
  Blynk.virtualWrite(V2, value);
}
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  Blynk.virtualWrite(V2, value);
}
*/

void loop()
{
  Blynk.run();
}
