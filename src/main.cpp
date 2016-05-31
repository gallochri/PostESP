/**************************************************************
 * 2016-05-14 gallochri.com
 **************************************************************/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Bounce2.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DNSServer.h>
// Add user_config.h in src folder with: #define SETTING_<var> "value";
#include "user_config.h"

// Blynk config
char auth[] = SETTINGS_AUTH;
// WiFi config
char ssid[] = SETTINGS_SSID;
char pass[] = SETTINGS_PASS;
// Server config
char server[] = SETTINGS_SERVER;
int port = SETTINGS_PORT;
// Mail config
char usermail[] = SETTINGS_USER_MAIL;
// PostESP config
const byte interruptPin = SETTINGS_INTERRUPTPIN;
// Set LOW/HIGH led PIN
int ON = 1;
int OFF = 0;
// Global variables
String DEVICE_TITLE = "PostESP";
String SSID_LIST = "";
boolean setupModeStatus = false;
// Instantiate object
Bounce debouncer = Bounce();
Ticker spinticker;
ESP8266WebServer WEB_SERVER(80);
volatile int spincount;
volatile int modcount;

void setBoard() {
  if(SETTINGS_NODE_MCU == 1) {
    Serial.println("###Board type set to NodeMCU###");
    ON = 0;
    OFF = 1;
  }
}

boolean loadSavedConfig() {
  Serial.println("Lettura configurazione....");
  if (EEPROM.read(0) != 0) {
    String(ssid);
    String(pass);
    String(server);
    String(port);
    String(auth);
    String(usermail);
    // SSID
    for (int i = 0; i < 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    // Password
    for (int i = 32; i < 96; ++i) {
      pass += char(EEPROM.read(i));
    }
    Serial.print("Password: ");
    Serial.println(pass);
    // Server
    for (int i = 96; i < 128; ++i) {
      server += char(EEPROM.read(i));
    }
    Serial.print("Server: ");
    Serial.println(server);
    // Port
    for (int i = 128; i < 134; ++i) {
      port += char(EEPROM.read(i));
    }
    Serial.print("Port: ");
    Serial.println(port);
    // Token
    for (int i = 134; i < 166; ++i) {
      auth += char(EEPROM.read(i));
    }
    Serial.print("Token: ");
    Serial.println(auth);
    // User MAil
    for (int i = 166; i < 230; ++i) {
      usermail += char(EEPROM.read(i));
    }
    Serial.print("Mail: ");
    Serial.println(usermail);

    Blynk.begin(auth.c_str(), ssid.c_str(), pass.c_str(), server.c_str(), port.toInt());
    while (Blynk.connect() == false) {
      Serial.println("Blynk server disconnected");
    }
    //WiFi.begin(ssid.c_str(), pass.c_str());
    return true;
  }
  else {
    Serial.println("Configurazione non trovata.");
    return false;
  }
}

void initLEDS() {
  // Note: for nodeMCU leds are reversed (use HIGH)
    pinMode(SETTINGS_BLUE_PIN, OUTPUT);
    digitalWrite(SETTINGS_BLUE_PIN, OFF);
    pinMode(SETTINGS_RED_PIN, OUTPUT);
    digitalWrite(SETTINGS_RED_PIN, OFF);
}

void flashLED(int times, int delayms) {
    int blinkCount = times;
    while (blinkCount-- > 0) {
        if (blinkCount % 2 == 0) {
            digitalWrite(SETTINGS_BLUE_PIN, OFF);
        } else {
            digitalWrite(SETTINGS_BLUE_PIN, ON);
        }
        delay(delayms);
    }
    // Quando esco il pin deve essere acceso
    digitalWrite(SETTINGS_BLUE_PIN, ON);
}

void setupFlashLED() {
    modcount++;
    if (modcount % 2 == 0) {
        digitalWrite(SETTINGS_BLUE_PIN, ON);
    } else {
        digitalWrite(SETTINGS_BLUE_PIN, OFF);
    }
}

void messageErrorLED() {
    digitalWrite(SETTINGS_RED_PIN, ON);
}

void stopSetupLEDS() {
    spinticker.detach();
    digitalWrite(SETTINGS_BLUE_PIN, OFF);
    digitalWrite(SETTINGS_RED_PIN, OFF);
}

boolean checkWiFiConnection() {
  int count = 0;
  Serial.print("Connessione");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Connesso!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void powerOff() {
    Serial.println("Power OFF");
    delay(250);
    ESP.deepSleep(0, WAKE_RF_DEFAULT);
}

// HTML Page maker
// ---------------
String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<style>";
  // Simple Reset CSS
  s += "*,*:before,*:after{-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}html{font-size:100%;-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%}html,button,input,select,textarea{font-family:sans-serif}article,aside,details,figcaption,figure,footer,header,hgroup,main,nav,section,summary{display:block}body,form,fieldset,legend,input,select,textarea,button{margin:0}audio,canvas,progress,video{display:inline-block;vertical-align:baseline}audio:not([controls]){display:none;height:0}[hidden],template{display:none}img{border:0}svg:not(:root){overflow:hidden}body{font-family:sans-serif;font-size:16px;font-size:1rem;line-height:22px;line-height:1.375rem;color:#585858;font-weight:400;background:#fff}p{margin:0 0 1em 0}a{color:#cd5c5c;background:transparent;text-decoration:underline}a:active,a:hover{outline:0;text-decoration:none}strong{font-weight:700}em{font-style:italic}";
  // Basic CSS Styles
  s += "body{font-family:sans-serif;font-size:16px;font-size:1rem;line-height:22px;line-height:1.375rem;color:#585858;font-weight:400;background:#fff}p{margin:0 0 1em 0}a{color:#cd5c5c;background:transparent;text-decoration:underline}a:active,a:hover{outline:0;text-decoration:none}strong{font-weight:700}em{font-style:italic}h1{font-size:32px;font-size:2rem;line-height:38px;line-height:2.375rem;margin-top:0.7em;margin-bottom:0.5em;color:#343434;font-weight:400}fieldset,legend{border:0;margin:0;padding:0}legend{font-size:18px;font-size:1.125rem;line-height:24px;line-height:1.5rem;font-weight:700}label,button,input,optgroup,select,textarea{color:inherit;font:inherit;margin:0}input{line-height:normal}.input{width:100%}input[type='text'],input[type='email'],input[type='tel'],input[type='date']{height:36px;padding:0 0.4em}input[type='checkbox'],input[type='radio']{box-sizing:border-box;padding:0}";
  // Custom CSS
  s += "header{width:100%;background-color: #000000;top: 0;min-height:60px;margin-bottom:21px;font-size:15px;color: #fff}.content-body{padding:0 1em 0 1em}header p{font-size: 1.25rem;float: left;position: relative;z-index: 1000;line-height: normal; margin: 15px 0 0 10px}";
  s += "</style>";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += "<header><p>" + DEVICE_TITLE + "</p></header>";
  s += "<div class=\"content-body\">";
  s += contents;
  s += "</div>";
  s += "</body></html>";
  return s;
}

// Decode URL
// ----------
String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

void setupMode(){
  setupModeStatus = true;
  // Initialize DNSServer object
  DNSServer DNS_SERVER;

  // Disconnect Wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Scan networks
  int n = WiFi.scanNetworks();
  delay(100);
  // Serial.println("");
  for (int i = 0; i < n; ++i) {
    SSID_LIST += "<option value=\"";
    SSID_LIST += WiFi.SSID(i);
    SSID_LIST += "\">";
    SSID_LIST += WiFi.SSID(i);
    SSID_LIST += "</option>";
  }
  delay(100);

  // Start AP
  const char* AP_SSID = "PostESP_Setup";
  const IPAddress AP_IP(192, 168, 1, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID);
  DNS_SERVER.start(53, "*", AP_IP);
  Serial.print("Starting Access Point at ");
  Serial.println(WiFi.softAPIP());

  // Settings Page
  WEB_SERVER.on("/settings", []() {
    String s = "<h2>Parametri Wifi</h2><p>Inserisci il nome della rete e la password.</p>";
    s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
    s += SSID_LIST;
    s += "</select><br><br><label>Password: </label><input name=\"pass\" length=64 type=\"password\">";
    s += "<br><br>";
    s += "<h2>Parametri server</h2>";
    s += "<br><label>Server:    </label><input name=\"server\"    lenght=32><br>";
    s += "<br><label>Port:      </label><input name=\"port\"      lenght=6><br>";
    s += "<br><label>Token:     </label><input name=\"auth\"      length=32><br>";
    s += "<br><label>User mail: </label><input name=\"usermail\"  length=64><br>";
    s += "<br><input type=\"submit\"></form>";
    WEB_SERVER.send(200, "text/html", makePage("Parametri", s));
  });

  // setap Form Post
  WEB_SERVER.on("/setap", []() {
    for (int i = 0; i < 230; ++i) {
      EEPROM.write(i, 0);
    }
    String ssid = urlDecode(WEB_SERVER.arg("ssid"));
    Serial.print("SSID: ");
    Serial.println(ssid);

    String pass = urlDecode(WEB_SERVER.arg("pass"));
    Serial.print("Password: ");
    Serial.println(pass);

    String server = urlDecode(WEB_SERVER.arg("server"));
    Serial.print("Server: ");
    Serial.println(server);

    String port = urlDecode(WEB_SERVER.arg("port"));
    Serial.print("Porta: ");
    Serial.println(port);

    String auth = urlDecode(WEB_SERVER.arg("auth"));
    Serial.print("Token: ");
    Serial.println(auth);

    String usermail = urlDecode (WEB_SERVER.arg("usermail"));
    Serial.print("Email: ");
    Serial.println(usermail);

    Serial.println("Writing SSID to EEPROM...");
    for (int i = 0; i < ssid.length(); ++i) {
      EEPROM.write(i, ssid[i]);
    }

    Serial.println("Writing Password to EEPROM...");
    for (int i = 0; i < pass.length(); ++i) {
      EEPROM.write(32 + i, pass[i]);
    }

    Serial.println("Writing Server to EEPROM...");
    for (int i = 0; i < server.length(); ++i) {
      EEPROM.write(96 + i, server[i]);
    }

    Serial.println("Writing Port to EEPROM...");
    for (int i = 0; i < port.length(); ++i) {
      EEPROM.write(128 + i, port[i]);
    }

    Serial.println("Writing Token to EEPROM...");
    for (int i = 0; i < auth.length(); ++i) {
      EEPROM.write(134 + i, auth[i]);
    }

    Serial.println("Writing Mail to EEPROM...");
    for (int i = 0; i < usermail.length(); ++i) {
      EEPROM.write(166 + i, usermail[i]);
    }

    EEPROM.commit();
    Serial.println("Write EEPROM done!");
    String s = "<h1>Configurazione completa..</h1><p>PostESP connessione a  \"";
    s += ssid;
    s += "\" dopo il riavvio.";
    WEB_SERVER.send(200, "text/html", makePage("Parametri Wi-Fi", s));
    //ESP.restart();

    stopSetupLEDS();
    powerOff();

  });
  // Show the configuration page if no path is specified
  WEB_SERVER.onNotFound([]() {
    String s = "<h1>WiFi Configuration Mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
    WEB_SERVER.send(200, "text/html", makePage("Access Point mode", s));
  });
WEB_SERVER.begin();
}

void emailOnButtonPress()
{
  // Let's send an e-mail when you press the button connected
  // to digital pin 5 (D1 on nodeMCU) on your ESP8266
  // Invert state, since button is "Active LOW"
  int isButtonPressed = !digitalRead(interruptPin);
  if (isButtonPressed)
  {
    Serial.println("Button is pressed.Try to send email.");
    //This serves to avoid the sending of emails during the test phase
    int value = millis() / 1000;
    Serial.println(value);
    //Blynk.email( usermail, "Subject: You have Mail!", "There's a letter in your postbox!");
    Serial.println("Mail sent.");
  }
}

void wipeEEPROM()
{
  EEPROM.begin(512);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);

  EEPROM.end();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  // Init EEPROM
  EEPROM.begin(512);

  // set board type enviroment
  setBoard();

  // Initializzo i pin
  Serial.println("LED Init");
  initLEDS();

  // If SETUP_PIN pushed, enter to setup mode
  if(digitalRead(SETTINGS_SETUP_PIN) == 0) {
     Serial.println("Enter setup mode");
     stopSetupLEDS();
     spinticker.attach(SPININTERVAL, setupFlashLED);
     setupMode();
     return;
   }
   // If configuration loading failed, enter to setup mode
   if (!loadSavedConfig()) {
       Serial.print("WARNING: Non trovo una configurazione salvata");
       stopSetupLEDS();
       spinticker.attach(SPININTERVAL, setupFlashLED);
       setupMode();
       return;
   }
   // If WIFI check failing, enter to setup mode
   if (!checkWiFiConnection()) {
       Serial.print("ERROR: Non riesco a collegarmi alla wifi");
       stopSetupLEDS();
       powerOff();
       return;
   }
   flashLED(10, 100);
   /*if (!setupModeStatus){
     // Collegamento al server
     char(server);
     Serial.println(server);
     Serial.println(port);
     Serial.println(auth);
     Serial.println(ssid);
     Serial.println(pass);
     Blynk.begin(auth, ssid, pass, server, port);

     while (Blynk.connect() == false) {
       Serial.println("Blynk server disconnected");
     }*/

     //Code for testing connectivity and email settings
     //Serial.println("Blynk.connect is true.Try to send.");
     //Blynk.email(usermail, "PostESP is online", "Ready.");
     //Serial.println("Mail sent.");

     // Setting the sensor pin and Bounce
     pinMode(interruptPin, INPUT_PULLUP);
     debouncer.attach(interruptPin);
     debouncer.interval(100); // interval in ms

     // Use attachInterrupt if the sensor have no bouncing problem
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

void loop()
{
  if (setupModeStatus){
      WEB_SERVER.handleClient();
  }

  Blynk.run();
  // Update the Bounce instance :
  debouncer.update();
  int status = debouncer.read();
  if (debouncer.fell()){
    emailOnButtonPress();
  }
}
