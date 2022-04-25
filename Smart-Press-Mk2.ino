/*
 * This Program is an example for how to initialize a device with a WiFi connection
 * The device will start up and look for any existing credentials from previous connections
 * If none are found and successfully connected, a wireless AP will be created for configuration
 */
#include <WiFiManager.h>
#include <ESP8266WiFi.h> // Used to get MAC Address

#define APP "Coffee Maker" // Name of Attached Device
#define BACK #000000 // Background Color
#define COLOR #800000 // Item Color
#define TEXT #FFFFFF // Text Color

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
const int output0 = 0;
const int output1 = 1;
const int output2 = 2;
const int output3 = 3;

void setup() {
//  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
//  Serial.setDebugOutput(true);
  wm.setDebugOutput(false);                 // Comment out to enable Debug

  debugln("\n--------------------------------");
  debugln("** Starting...");
  debugln("** Device MAC Address: " + WiFi.macAddress());

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
    debugln("--------------------------------");
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
  debugln("** Local IP: " + WiFi.localIP());    // Print the IP address
  debugln("** Setup Complete!");
  debugln("--------------------------------");
}

void loop() {
  WiFiClient client = server.available(); // Check if a client has connected
  if (client) {                           // Only runs if the client is connected
    debugln("\n================================");
    debugln("* New Client.");
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {                 // if there's bytes to read from the client,
        receiveRequest(client);
        if (c == '\n') {                            // if the byte is a newline character
          // two newline characters in a row means end of the client HTTP request
          if (currentLine.length() == 0) {              // so send a response:
            updatePins(client);
            displayWebpage(client);
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
    debugln("* Client disconnected.");
    debugln("================================");
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

void receiveRequest(WiFiClient client) {
    /*
    * Reads HTTP Request from Client (one line per call)
    */
    char c = client.read();             // read a byte, then
    Serial.write(c);                    // print it out the serial monitor
    header += c;
}

void updatePins(WiFiManager client) {
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
    if (header.indexOf("GET /3/ON") >= 0) {
        debugln("* GPIO 3 ON");
        output3State = "ON";
        digitalWrite(output3, HIGH);
    } else if (header.indexOf("GET /3/OFF") >= 0) {
        debugln("* GPIO 3 OFF");
        output3State = "OFF";
        digitalWrite(output3, LOW);
    } else if (header.indexOf("GET /2/ON") >= 0) {
        debugln("* GPIO 2 ON");
        output2State = "ON";
        digitalWrite(output2, HIGH);
    } else if (header.indexOf("GET /2/OFF") >= 0) {
        debugln("* GPIO 2 OFF");
        output2State = "OFF";
        digitalWrite(output2, LOW);
    } else if (header.indexOf("GET /1/ON") >= 0) {
        debugln("* GPIO 1 ON");
        output1State = "ON";
        digitalWrite(output1, HIGH);
    } else if (header.indexOf("GET /1/OFF") >= 0) {
        debugln("* GPIO 1 OFF");
        output1State = "OFF";
        digitalWrite(output1, LOW);
    } else if (header.indexOf("GET /0/ON") >= 0) {
        debugln("* GPIO 0 ON");
        output0State = "ON";
        digitalWrite(output0, HIGH);
    } else if (header.indexOf("GET /0/OFF") >= 0) {
        debugln("* GPIO 0 OFF");
        output0State = "OFF";
        digitalWrite(output0, LOW);
    }
}

// Respond to the client with the Webpage
void displayWebpage(WiFiManager client) {
    /*
    * Generates a webpage using HTML and sends it to the HTTP Client
    * 
    * TODO: Specify which pins are connected to what (on button, off button, buzzer, etc.)
    * TODO: 
    * TODO: Test
    */

    // HTML Header
    client.println("<!DOCTYPE html><html>");
    client.println("<head>");
    client.println("<title>Smart-Press | " + APP + "</title>");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    
    // CSS style section
    client.println("<style>html { font-family: Helvetica; color: " + TEXT + "; display: inline-block; margin: 0px auto; text-align: center; }");
    client.println(".bg { background-color: " + BACK + "; }");
    client.println(".button1 { background-color: " + COLOR + "; border: none; color: " + TEXT + "; padding: 16px 40px; ");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");
    client.println(".button0 { background-color: " + TEXT + "; border: none; color: " + COLOR + "; padding: 16px 40px; }</style>");

    // Auto Refresh Option
    client.println("<meta http-equiv=\"refresh\" content=\"20\"></head>");
    
    // Begin Body
    client.println("<body class=\"bg\"><h1>ESP8266 Web Server</h1>");
    
    // Display current state, and ON/OFF buttons for GPIO 3  
    client.println("<p>GPIO 3 - Currently " + output3State + "</p>");
    // TODO: Do I want it like this??? If the output3State is OFF, it displays the ON button
    if (output3State=="OFF") {
        client.println("<p><a href=\"/3/ON\"><button class=\"button1\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/3/OFF\"><button class=\"button0\">OFF</button></a></p>");
    } 
        
    // Display current state, and ON/OFF buttons for GPIO 2  
    client.println("<p>GPIO 2 - Currently " + output2State + "</p>");
    // TODO: Do I want it like this??? If the output2State is OFF, it displays the ON button
    if (output2State=="OFF") {
        client.println("<p><a href=\"/2/ON\"><button class=\"button1\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/2/OFF\"><button class=\"button0\">OFF</button></a></p>");
    } 
    
    // // Display current state, and ON/OFF buttons for GPIO 1
    // client.println("<p>GPIO 1 - State " + output1State + "</p>");
    // // TODO: Do I want it like this??? If the output1State is OFF, it displays the ON button
    // if (output1State=="OFF") {
    //     client.println("<p><a href=\"/1/ON\"><button class=\"button1\">ON</button></a></p>");
    // } else {
    //     client.println("<p><a href=\"/1/OFF\"><button class=\"button0\">OFF</button></a></p>");
    // } 
    
    // Display current state, and ON/OFF buttons for GPIO 0
    client.println("<p>GPIO 0 - Currently " + output0State + "</p>");
    // TODO: Do I want it like this??? If the output0State is OFF, it displays the ON button
    if (output0State=="OFF") {
        client.println("<p><a href=\"/0/ON\"><button class=\"button1\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/0/OFF\"><button class=\"button0\">OFF</button></a></p>");
    } 
    client.println("</body></html>");
    
    // The HTTP response ends with another blank line
    client.println();
}