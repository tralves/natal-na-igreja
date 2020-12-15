#include <ESP8266WiFi.h>

// const char *ssid = "NOS_Internet_66D8";
// const char *password = "BFTMRN5FN4L";
const char *ssid = "Maridao&Maridona";
const char *password = "deusebom";

/**  
 *                 /\ 
 *              5 /  \ 16
 *               /    \
 *     __/ 0                4 \__
 *    /                          \
 *   ------14------  -------2------
 *    |  |    |  |    |  |    |  |
 *    |3 |    |15|    |13|    |12| 
 **/

// int PINS[] = {15, 13, 12, 14, 6, 7, 11, 8, 9, 10};
int PINS[] = {3, 15, 13, 12, 14, 2, 0, 4, 5, 16};
int LIGHT_COUNT = 10;
WiFiServer server(80);

void setup()
{
  Serial.begin(74880);
  delay(10);

  for (int i = 0; i < 10; i++)
  {
    pinMode(PINS[i], OUTPUT);
    digitalWrite(PINS[i], LOW);
  }

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request
  if (request.indexOf("/favicon.ico") != -1)
  {
    Serial.println("ignoring...");
    delay(1);
    Serial.println("Client disonnected");
    Serial.println("");
    return;
  }

  String currentPinValues;

  int pathIndex = request.indexOf("/LEDS=");
  if (pathIndex != -1)
  {
    String ledsVal = request.substring(
        pathIndex + 6,
        request.indexOf(" ", pathIndex + 6));
    Serial.println(ledsVal);
    long pinsVal = ledsVal.toInt();

    Serial.print("currentPinValues: ");
    Serial.println(String(pinsVal, BIN));

    applyPins(pinsVal);
    htmlResponse(client, pinsVal);
  }

  // Set ledPin according to the request
  //digitalWrite(ledPin, value);

  // Return the response

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

void applyPins(int pinsVal)
{
  bool pin;
  for (int i = 0; i < LIGHT_COUNT; i++)
  {
    pin = (pinsVal >> (LIGHT_COUNT - i - 1) & 1);
    digitalWrite(PINS[i], pin ? LOW : HIGH);
  }
}

void htmlResponse(WiFiClient client, int pinsVal)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Pins are now: ");

  client.print(String(pinsVal, BIN));
  client.println(" <a href=\"/\"\"><button>Refresh </button></a><br />");

  client.println("<br><br>");
  client.println("<a href=\"/LEDS=1023\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LEDS=0\"\"><button>Turn Off </button></a><br />");
  client.println("<a href=\"/LEDS=769\"\"><button>769 </button></a><br />");
  client.println("</html>");
}