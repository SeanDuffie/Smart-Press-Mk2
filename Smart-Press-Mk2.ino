/*
 * This Program is an example for how to initialize a device with a WiFi connection
 * The device will start up and look for any existing credentials from previous connections
 * If none are found and successfully connected, a wireless AP will be created for configuration
 */
#include <WiFiManager.h>
#include <ESP8266WiFi.h> // Used to get MAC Address

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

// TODO: Section to store variables for handling webpage logic
bool res = false; // Variable to determine whether to reset stored credentials or not

// Auxiliar variables to store the current output state
String output0State = "off";
String output1State = "off";
String output2State = "off";
String output3State = "off";

// Assign output variables to GPIO pins
const int output0 = 0;
const int output1 = 1;
const int output2 = 2;
const int output3 = 3;

void setup() {
//  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
//  Serial.setDebugOutput(true);

  debugln("\n--------------------------------");
  debugln("** Starting...");
  debugln("** Device MAC Address: " + WiFi.macAddress());

  wm.setAPCallback(configModeCallback);     // Callback Function to Config Mode

  // Set Static IP and Config Portal IP Addresses manually
  wm.setSTAStaticIPConfig(IPAddress(192,168,0,177), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  wm.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  wm.setConfigPortalTimeout(120);           // How long to wait in Config Mode
  wm.setConnectTimeout(20);                 // How long to try to connect
  wm.setClass("invert");                    // Dark Theme
  wm.setDebugOutput(false);                 // Comment out to enable Debug
  
  if (!wm.autoConnect("AutoConnectAP")) {   // Handles AutoConnect Failures
    debugln("** Failed to connect or hit timeout");
    debugln("** Restarting in 3 seconds...");
    debugln("--------------------------------");
    delay(3000);
    ESP.restart();
  }

  // Initialize the output variables as outputs
  pinMode(output0, OUTPUT);
//  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output0, LOW);
//  digitalWrite(output1, LOW);
  digitalWrite(output2, LOW);
  digitalWrite(output3, LOW);

  server.begin(); // Start the server
  debug("** Local IP: ");
  debugln(WiFi.localIP());    // Print the IP address
  debugln("** Setup Complete!");
  debugln("--------------------------------");
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {                           // Only runs if the client is connected
    debugln("\n================================");
    debugln("* New Client.");
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /3/on") >= 0) {
              debugln("* GPIO 3 on");
              output3State = "on";
              digitalWrite(output3, HIGH);
            } else if (header.indexOf("GET /3/off") >= 0) {
              debugln("* GPIO 3 off");
              output3State = "off";
              digitalWrite(output3, LOW);
            } else if (header.indexOf("GET /2/on") >= 0) {
              debugln("* GPIO 2 on");
              output2State = "on";
              digitalWrite(output2, HIGH);
            } else if (header.indexOf("GET /2/off") >= 0) {
              debugln("* GPIO 2 off");
              output2State = "off";
              digitalWrite(output2, LOW);
//            } else if (header.indexOf("GET /1/on") >= 0) {
//              debugln("* GPIO 1 on");
//              output1State = "on";
//              digitalWrite(output1, HIGH);
//            } else if (header.indexOf("GET /1/off") >= 0) {
//              debugln("* GPIO 1 off");
//              output1State = "off";
//              digitalWrite(output1, LOW);
            } else if (header.indexOf("GET /0/on") >= 0) {
              debugln("* GPIO 0 on");
              output0State = "on";
              digitalWrite(output0, HIGH);
            } else if (header.indexOf("GET /0/off") >= 0) {
              debugln("* GPIO 0 off");
              output0State = "off";
              digitalWrite(output0, LOW);
            }
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style>");
            client.println("<meta http-equiv=\"refresh\" content=\"5\"></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 3  
            client.println("<p>GPIO 3 - State " + output3State + "</p>");
            // If the output3State is off, it displays the ON button       
            if (output3State=="off") {
              client.println("<p><a href=\"/3/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/3/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 2  
            client.println("<p>GPIO 2 - State " + output2State + "</p>");
            // If the output2State is off, it displays the ON button       
            if (output2State=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
//            // Display current state, and ON/OFF buttons for GPIO 1
//            client.println("<p>GPIO 1 - State " + output1State + "</p>");
//            // If the output1State is off, it displays the ON button       
//            if (output1State=="off") {
//              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
//            } else {
//              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
//            } 
            
            // Display current state, and ON/OFF buttons for GPIO 0
            client.println("<p>GPIO 0 - State " + output0State + "</p>");
            // If the output0State is off, it displays the ON button       
            if (output0State=="off") {
              client.println("<p><a href=\"/0/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    debugln("* Client disconnected.");
    debugln("================================");
  }
  if (res) {wm.resetSettings();}
}
  
// Config Mode Function, Starts AP for configuring 
void configModeCallback(WiFiManager *myWiFiManager) {
  debugln("Entered config mode");
  debugln(WiFi.softAPIP());

  debugln(myWiFiManager->getConfigPortalSSID());
}
