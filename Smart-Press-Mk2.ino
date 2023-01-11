/*
 * This Program is an example for how to initialize a device with a WiFi connection
 * The device will start up and look for any existing credentials from previous connections
 * If none are found and successfully connected, a wireless AP will be created for configuration
 */
#include <WiFiManager.h>
//#include <ESP8266WiFi.h> // Used to get MAC Address

#define APP "Coffee Maker" // Name of Attached Device
#define BACK "#000000" // Background Color
#define COLOR "#800000" // Item Color
#define TEXT "#FFFFFF" // Text Color

#define RES false // Determine whether to reset stored credentials or not
#define DEBUG true // Toggle this to enable/disable serial prints on TX/GPIO1
#if DEBUG == true
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

WiFiManager wm;

WiFiServer server(80); // Create an instance of the server on port 80

String header; // Variable to store the HTTP request

// Auxiliar variables to store the current output state
String output0State = "OFF";
String output1State = "OFF";
String output2State = "OFF";
String output3State = "OFF";

// Assign output variables to GPIO pins
const int output0 = 5;
const int output1 = 32;
const int output2 = 33;
const int output3 = 25;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA); // explicitly set mode, esp defaults to STA+AP
//  Serial.setDebugOutput(true);
  wm.setDebugOutput(false);                 // Comment out to enable Debug

  debugln("\n================================");
  debugln("** Starting...");
  debug("** Device MAC Address: ");
  debugln(WiFi.macAddress());

  wm.setAPCallback(configModeCallback);     // Callback Function to Config Mode

  // Set Static IP and Config Portal IP Addresses manually
  wm.setSTAStaticIPConfig(IPAddress(192,168,0,177), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  wm.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  wm.setConfigPortalTimeout(120);           // How long to wait in Config Mode
  wm.setConnectTimeout(20);                 // How long to try to connect
  wm.setClass("invert");                    // Dark Theme for config mode
  
  if (!wm.autoConnect("AutoConnectAP")) {   // Handles AutoConnect Failures
    debugln("** Failed to connect or hit timeout");
    debugln("** Restarting in 3 seconds...");
    debugln("================================");
    delay(3000);
    ESP.restart();
  }

  // Initialize the output variables as outputs
  pinMode(output0, OUTPUT);
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output0, LOW);
  digitalWrite(output1, LOW);
  digitalWrite(output2, LOW);
  digitalWrite(output3, LOW);

  server.begin(); // Start the server
  debug("** Local IP: ");
  debugln(WiFi.localIP());    // Print the IP address
  debugln("** Setup Complete!");
  debugln("================================");
}

void loop() {
  WiFiClient client = server.available(); // Check if a client has connected
  if (client) {                           // Only runs if the client is connected
    debug("* Remote IP:\t");
    debug(client.remoteIP());
    debug("\t:\t");
    debugln(client.remotePort());
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {                 // if there's bytes to read from the client,
        char c = client.read();             // Receive byte from request
        header += c;
        if (c == '\n') {                            // if the byte is a newline character
          // two newline characters in a row means end of the client HTTP request
          if (currentLine.length() == 0) {              // so send a response:
            updatePins(client);
            // displayWebpage(client);
            // getPin(client);
            break;
          } else {                                  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    header = ""; // Clear the header variable
    client.stop(); // Close the connection
    debugln("--------------------------------");
  }
  if (RES) {wm.resetSettings();}
}
  
// Config Mode Function, Starts AP for configuring 
void configModeCallback(WiFiManager *myWiFiManager) {
  /*
  * This Function is called whenever no existing connection is found
  * 
  */
  debugln("Entered config mode");
  debugln(WiFi.softAPIP());

  debugln(myWiFiManager->getConfigPortalSSID());
}

void updatePins(WiFiClient client) {
    /*
    * Updates the GPIO pins based on each line of the HTTP Request
    * 
    * TODO: This function will eventually contain the logic to decide what happens to the pins
    * TODO: Add a timer for a pin to switch off a specified amount of time after it was turned on
    * TODO: Specify which pins are connected to what (on button, off button, buzzer, etc.)
    * TODO: Test
    */

    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    
    // turns the GPIOs ON and OFF
    if (header.indexOf("GET /25/ON") >= 0) {
        debugln("* GPIO 25 ON");
        output3State = "ON";
        digitalWrite(output3, HIGH);
    } else if (header.indexOf("GET /25/OFF") >= 0) {
        debug("* GPIO ");
        debug(output3);
        debugln(" OFF");
        output3State = "OFF";
        digitalWrite(output3, LOW);
    } else if (header.indexOf("GET /25") >= 0) {
        debug("* GPIO ");
        debug(output3);
        debug("=");
        debugln(output3State);
        client.println(output3State);
        client.println();
    } else if (header.indexOf("GET /33/ON") >= 0) {
        debug("* GPIO ");
        debug(output2);
        debugln(" ON");
        output2State = "ON";
        digitalWrite(output2, HIGH);
    } else if (header.indexOf("GET /33/OFF") >= 0) {
        debug("* GPIO ");
        debug(output2);
        debugln(" OFF");
        output2State = "OFF";
        digitalWrite(output2, LOW);
    } else if (header.indexOf("GET /33") >= 0) {
        debug("* GPIO ");
        debug(output2);
        debug("=");
        debugln(output2State);
        client.println(output2State);
        client.println();
    } else if (header.indexOf("GET /32/ON") >= 0) {
        debug("* GPIO ");
        debug(output1);
        debugln(" ON");
        output1State = "ON";
        digitalWrite(output1, HIGH);
    } else if (header.indexOf("GET /32/OFF") >= 0) {
        debug("* GPIO ");
        debug(output1);
        debugln(" OFF");
        output1State = "OFF";
        digitalWrite(output1, LOW);
    } else if (header.indexOf("GET /32") >= 0) {
        debug("* GPIO ");
        debug(output1);
        debug("=");
        debugln(output1State);
        client.println(output1State);
        client.println();
    } else if (header.indexOf("GET /5/ON") >= 0) {
        debug("* GPIO ");
        debug(output0);
        debugln(" ON");
        output0State = "ON";
        digitalWrite(output0, HIGH);
    } else if (header.indexOf("GET /5/OFF") >= 0) {
        debug("* GPIO ");
        debug(output0);
        debugln(" OFF");
        output0State = "OFF";
        digitalWrite(output0, LOW);
    } else if (header.indexOf("GET /5") >= 0) {
        debug("* GPIO ");
        debug(output0);
        debug("=");
        debugln(output0State);
        client.println(output0State);
        client.println();
    } else {
        debugln("Error with header input");
        client.println("Error with header input");
    }
}

// // Respond to the client with the Webpage
// void displayWebpage(WiFiClient client) {
//     /*
//     * Generates a webpage using HTML and sends it to the HTTP Client
//     * 
//     * TODO: Specify which pins are connected to what (on button, off button, buzzer, etc.)
//     * TODO: 
//     * TODO: Test
//     */

//     // HTML Header
//     client.println("<!DOCTYPE html><html>");
//     client.println("<head>");
//     client.print("<title>Smart-Press | ");
//     client.print(APP);
//     client.println("</title>");
//     client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//     client.println("<link rel=\"icon\" href=\"data:,\">");
    
//     // CSS style section
//     client.print("<style>html { font-family: Helvetica; color: ");
//     client.print(TEXT);
//     client.println("; display: inline-block; margin: 0px auto; text-align: center; }");
//     client.print(".bg { background-color: ");
//     client.print(BACK);
//     client.println("; }");

//     client.print(".ON { background-color: ");
//     client.print(COLOR);
//     client.print("; border: none; color: ");
//     client.print(TEXT);
//     client.println("; padding: 16px 32px; ");
//     client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");

//     client.print(".OFF { background-color: ");
//     client.print(TEXT);
//     client.print("; border: none; color: ");
//     client.print(COLOR);
//     client.println("; padding: 16px 40px; }");
//     client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }</style>");

//     // Auto Refresh Option
//     client.println("<meta http-equiv=\"refresh\" content=\"20\"></head>");

//     // Begin Body
//     client.println("<body class=\"bg\"><h1>ESP8266 Web Server</h1>");

//     // Display current state, and ON/OFF buttons for GPIO 3
//     client.print("<p>GPIO ");
//     client.print(output3);
//     client.print(" - Currently ");
//     client.print(output3State);
//     client.println("</p>");
//     // TODO: Do I want it like this??? If the output3State is OFF, it displays the ON button
//     if (output3State=="OFF") {
//         client.println("<p><a href=\"/25/ON\"><button class=\"ON\">ON</button></a></p>");
//     } else {
//         client.println("<p><a href=\"/25/OFF\"><button class=\"OFF\">OFF</button></a></p>");
//     }

//     // Display current state, and ON/OFF buttons for GPIO 2
//     client.print("<p>GPIO ");
//     client.print(output2);
//     client.print(" - Currently ");
//     client.print(output2State);
//     client.println("</p>");
//     if (output2State=="OFF") {
//         client.println("<p><a href=\"/33/ON\"><button class=\"ON\">ON</button></a></p>");
//     } else {
//         client.println("<p><a href=\"/33/OFF\"><button class=\"OFF\">OFF</button></a></p>");
//     }
    
//      // Display current state, and ON/OFF buttons for GPIO 1
//     client.print("<p>GPIO ");
//     client.print(output1);
//     client.print(" - Currently ");
//     client.print(output1State);
//     client.println("</p>");
//      if (output1State=="OFF") {
//          client.println("<p><a href=\"/32/ON\"><button class=\"ON\">ON</button></a></p>");
//      } else {
//          client.println("<p><a href=\"/32/OFF\"><button class=\"OFF\">OFF</button></a></p>");
//      }
    
//     // Display current state, and ON/OFF buttons for GPIO 0
//     client.print("<p>GPIO ");
//     client.print(output0);
//     client.print(" - Currently ");
//     client.print(output0State);
//     client.println("</p>");
//     // TODO: Do I want it like this??? If the output0State is OFF, it displays the ON button
//     if (output0State=="OFF") {
//         client.println("<p><a href=\"/5/ON\"><button class=\"ON\">ON</button></a></p>");
//     } else {
//         client.println("<p><a href=\"/5/OFF\"><button class=\"OFF\">OFF</button></a></p>");
//     } 
//     client.println("</body></html>");
    
//     // The HTTP response ends with another blank line
//     client.println();
// }
