/**************************************************************
 * 2016-05-14 gallochri.com
 **************************************************************/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Bounce2.h>

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
  // Let's send an e-mail when you press the button connected
  // to digital pin 5 (D1 on nodeMCU) on your ESP8266
  // Invert state, since button is "Active LOW"
  int isButtonPressed = !digitalRead(interruptPin);
  if (isButtonPressed)
  {
    Serial.println("Button is pressed.Try to send email.");
    Blynk.email( usermail, "Subject: You have Mail!", "There's a letter in your postbox!");
    Serial.println("Mail sent.");
  }
}

// Instantiate a Bounce object to debouncing signal on input pin
Bounce debouncer = Bounce();

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, server, port);

  while (Blynk.connect() == false) {
    Serial.println("Blynk server disconnected");
  }

  //Code for testing connectivity and email settings
  //Serial.println("Blynk.connect is true.Try to send.");
  //Blynk.email(usermail, "PostESP is online", "Ready.");
  //Serial.println("Mail sent.");

  // Setting the sensor pin and Bounce
  pinMode(interruptPin, INPUT_PULLUP);
  debouncer.attach(interruptPin);
  debouncer.interval(100); // interval in ms

  // Use attachInterrupt if the sensor havo no bouncing problem
  //attachInterrupt(digitalPinToInterrupt(interruptPin), emailOnButtonPress, FALLING);
}

// Keep this flag not to re-sync on every reconnection
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
}
  /*
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
  // Update the Bounce instance :
  debouncer.update();
  int status = debouncer.read();
  if (debouncer.fell()){
    emailOnButtonPress();
  }
}
