/*
 * This Program is an example for how to initialize a device with a WiFi connection
 * The device will start up and look for any existing credentials from previous connections
 * If none are found and successfully connected, a wireless AP will be created for configuration
 */
#include <WiFiManager.h>
//#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#define DEBUG true // Toggle this to enable/disable serial prints on TX/GPIO1
#if DEBUG == true
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

WiFiManager wm;

WiFiServer server(80); // Create an instance of the wifiserver on port 80

const char* host = "Smart Node AP";
bool RST = false; // Determine whether to reset stored credentials or not
String APP = "Coffee Maker"; // Name of Attached Device
String BACK = "#000000"; // Background Color
String COLOR = "#800000"; // Item Color
String TEXT = "#FFFFFF"; // Text Color

String header; // Variable to store the HTTP request

/* CSS Style String */
String style =
"<style>"
"    #file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"    input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"        #file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"        #bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"    form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
"    .btn{background:#3498db;color:#fff;cursor:pointer}"
"</style>";

/* HTML Login Page String */
String loginIndex = 
"<form name=loginForm>"
"    <h1>ESP32 Login</h1>"
"    <input name=userid placeholder='User ID'> "
"    <input name=pwd placeholder=Password type=Password>"
"    <input type=submit onclick=check(this.form) class=btn value=Login>"
"</form>"
"<script>"
"    function check(form) {"
"        if(form.userid.value=='admin' && form.pwd.value=='admin')"
"            {window.open('/serverIndex')}"
"        else"
"            {alert('Error Password or Username')}"
"    }"
"</script>" + style;
 
/* Server Index Page String */
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"    <input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"    <label id='file-input' for='file'>   Choose file...</label>"
"    <input type='submit' class=btn value='Update'>"
"    <br><br>"
"    <div id='prg'></div>"
"    <br><div id='prgbar'><div id='bar'></div></div><br>"
"</form>"
"<script>"
"    function sub(obj){"
"        var fileName = obj.value.split('\\\\');"
"        document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"    };"
"    $('form').submit(function(e){"
"        e.preventDefault();"
"        var form = $('#upload_form')[0];"
"        var data = new FormData(form);"
"        $.ajax({"
"            url: '/update',"
"            type: 'POST',"
"            data: data,"
"            contentType: false,"
"            processData:false,"
"            xhr: function() {"
"                var xhr = new window.XMLHttpRequest();"
"                xhr.upload.addEventListener('progress', function(evt) {"
"                    if (evt.lengthComputable) {"
"                        var per = evt.loaded / evt.total;"
"                        $('#prg').html('progress: ' + Math.round(per*100) + '%');"
"                        $('#bar').css('width',Math.round(per*100) + '%');"
"                    }"
"                }, false);"
"                return xhr;"
"            },"
"            success:function(d, s) {"
"                console.log('success!')"
"            },"
"            error: function (a, b, c) {}"
"        });"
"    });"
"</script>" + style;

String buttonHead =
"<!DOCTYPE html><html>"
"    <head>"
"        <title>Smart-Press | " + APP + "</title>"
"        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"        <link rel=\"icon\" href=\"data:,\">"
"        <style>html { font-family: Helvetica; color: " + TEXT + "; display: inline-block; margin: 0px auto; text-align: center; }"
"            .bg { background-color: " + BACK + "; }"
"            .ON { background-color: " + COLOR + "; border: none; color: " + TEXT + "; padding: 16px 32px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }"
"            .OFF { background-color: " + TEXT + "; border: none; color: " + COLOR + "; padding: 16px 40px; }"
"            text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }"
"        </style>"
"    <meta http-equiv=\"refresh\" content=\"20\"></head>"
"    <body class=\"bg\"><h1>ESP8266 Web Server</h1>\n";

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

  debugln("\n================================");
  debugln("** Starting...");
  debug("** Device MAC Address: ");
  debugln(WiFi.macAddress());

  /************************************  Start WiFi Manager  *************************************/
  wm.setDebugOutput(false);                 // Comment out to enable Debug
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
  /************************************   End WiFi Manager   *************************************/

  /************************************   Start Pin Setup    *************************************/
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
  /************************************    End Pin Setup     *************************************/

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
  if (RST) {wm.resetSettings();}
}
  
// Config Mode Function, Starts AP for configuring 
void configModeCallback(WiFiManager *myWiFiManager) {
  /*
  * This Function is called whenever no existing connection is found
  * 
  */
  debugln("*** Entered config mode");
  
//  debug("*** AP IP: ");
//  debugln(WiFi.softAPIP());

  debug("*** AP SSID: ");
  debugln(myWiFiManager->getConfigPortalSSID());
}

// Updates the status of the pins locally
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
    } else if (header.indexOf("RST") >= 0) {
        client.println("RESETTING...");
        client.println();
        RST = true;
    } else {
        debugln("Error with header input");
        client.println("Error with header input");
        client.println();
    }
}

// Respond to the client with the Webpage
void displayWebpage(WiFiClient client) {
    /*
    * Generates a webpage using HTML and sends it to the HTTP Client
    * 
    * TODO: Specify which pins are connected to what (on button, off button, buzzer, etc.)
    * TODO: 
    * TODO: Test
    */

    // Display current state, and ON/OFF buttons for GPIO 3
    String buttonIndex = buttonHead + "<p>GPIO " + output3 + " - Currently " + output3State + "</p>";
    if (output3State=="OFF") {
        buttonIndex = buttonIndex + "\t<p><a href=\"/25/ON\"><button class=\"ON\">ON</button></a></p>\n";
    } else {
        buttonIndex = buttonIndex + "\t<p><a href=\"/25/OFF\"><button class=\"OFF\">OFF</button></a></p>\n";
    }

    // Display current state, and ON/OFF buttons for GPIO 2
    buttonIndex = buttonIndex + "<p>GPIO " + output2 + " - Currently " + output2State + "</p>";
    if (output2State=="OFF") {
        buttonIndex = buttonIndex + "\t<p><a href=\"/33/ON\"><button class=\"ON\">ON</button></a></p>\n";
    } else {
        buttonIndex = buttonIndex + "%s\t<p><a href=\"/33/OFF\"><button class=\"OFF\">OFF</button></a></p>\n";
    }

    // Display current state, and ON/OFF buttons for GPIO 1
    buttonIndex = buttonIndex + "<p>GPIO " + output1 + " - Currently " + output1State + "</p>";
    if (output1State=="OFF") {
        buttonIndex = buttonIndex + "\t<p><a href=\"/32/ON\"><button class=\"ON\">ON</button></a></p>\n";
    } else {
        buttonIndex = buttonIndex + "\t<p><a href=\"/32/OFF\"><button class=\"OFF\">OFF</button></a></p>\n";
    }

    // Display current state, and ON/OFF buttons for GPIO 0
    buttonIndex = buttonIndex + "<p>GPIO " + output0 + " - Currently " + output0State + "</p>";
    if (output0State=="OFF") {
        buttonIndex = buttonIndex + "\t<p><a href=\"/5/ON\"><button class=\"ON\">ON</button></a></p>\n";
    } else {
        buttonIndex = buttonIndex + "\t<p><a href=\"/5/OFF\"><button class=\"OFF\">OFF</button></a></p>\n";
    }
    buttonIndex = buttonIndex + "</body></html>\n";

    client.println(buttonIndex);
}
