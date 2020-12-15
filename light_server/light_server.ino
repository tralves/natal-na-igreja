
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef STASSID
// #define STASSID "Maridao&Maridona"
// #define STAPSK "deusebom"
#define STASSID "I.E Pampilhosa"
#define STAPSK "QF84QesVrw"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

int PINS[] = {3, 15, 13, 12, 14, 2, 0, 4, 5, 16};
int LIGHT_COUNT = 10;

String animation = "0000000000 \
1000000000 \
1100000000 \
1110000000 \
1111000000 \
1111100000 \
1111110000 \
1111111000 \
1111111100 \
1111111110 \
1111111111 \
1111111111 \
1111111110 \
1111111100 \
1111111000 \
1111110000 \
1111100000 \
1111000000 \
1110000000 \
1100000000 \
1000000000 \
0000000000 \
1111000000 \
1111110000 \
1111111100 \
0000111111 \
0000001111 \
0000000011 \
0000000000 \
0000000000 \
1111000000 \
1111110000 \
1111111100 \
0000111111 \
0000001111 \
0000000011 \
0000000000 \
0000000000 \
1111000000 \
1111110000 \
1111111100 \
0000111111 \
0000001111 \
0000000011 \
0000000000 \
0000000000 \
1000000000 \
1100000000 \
0110000000 \
0011000000 \
0001000000 \
0000010000 \
0000100000 \
0000001000 \
0000000100 \
0000000001 \
0000000010 \
1000000000 \
1100000000 \
0110000000 \
0011000000 \
0001000000 \
0000010000 \
0000100000 \
0000001000 \
0000000100 \
0000000001 \
0000000010 \
1000000000 \
1100000000 \
0110000000 \
0011000000 \
0001000000 \
0000010000 \
0000100000 \
0000001000 \
0000000100 \
0000000001 \
0000000010 \
0000000000 \
0000000011";
int animationDelay = 500;
bool animating = false;
int currentFrame = 0;
int timeSinceLastUpdate = 1800000;
String lightsVal = "0000000000";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

const String postForms1 = "<html>\
    <body>\
        <h1>Pins are now: </h1>\
        <p>";
const String postForms2 = "</p>\
        <a href =\"/\"\"><button>Refresh </button></a><br />\
        <h1>Set lights</h1>\
        <form method =\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/\">\
        <input type=\"text\" name=\"lights\" value=\"1111111111\"><br>\
        <input type=\"submit\" value=\"Submit\">\
        </form>\
    </body>\
    </html>";

void handleRoot()
{
    if (server.method() == HTTP_POST)
    {
        lightsVal = server.arg("lights");
        animating = false;
        applyLights(lightsVal);
    }

    server.send(200, "text/html", postForms1 + lightsVal + postForms2);
}

void handleAnimation()
{
    if (server.method() != HTTP_POST ||
        !server.hasArg("animation") ||
        !server.hasArg("delay"))
    {
        server.send(400, "text/plain", "invalid request");
        return;
    }

    animation = server.arg("animation");

    Serial.println("animation: " + animation);
    Serial.println("delay: " + server.arg("delay"));
    animationDelay = server.arg("delay").toInt();

    startAnimation();

    server.send(200, "text/html", "ok!");
}

void setup(void)
{
    Serial.begin(74880);
    delay(10);

    for (int i = 0; i < 10; i++)
    {
        pinMode(PINS[i], OUTPUT);
        digitalWrite(PINS[i], LOW);
    }
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266"))
    {
        Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);

    server.on("/animation/", handleAnimation);

    server.begin();
    Serial.println("HTTP server started");

    timeClient.begin();
}

void loop(void)
{
    server.handleClient();

    if (timeSinceLastUpdate > 1800000)
    {
        timeClient.update();
        timeSinceLastUpdate = 0;
        int hour = timeClient.getHours();
        Serial.print(hour);
        Serial.print(":");
        Serial.print(timeClient.getMinutes());
        Serial.print(":");
        Serial.println(timeClient.getSeconds());
        if (hour >= 18 || hour <= 7)
        {
            animating = true;
        }
        else
        {
            animating = false;
            applyLights("0000000000");
            currentFrame = 0;
        }
    }

    if (animating)
    {
        int frameCount = (animation.length() + 1) / (LIGHT_COUNT + 1);
        applyLights(
            animation.substring(
                currentFrame * (LIGHT_COUNT + 1),
                currentFrame * (LIGHT_COUNT + 1) + 10));
        currentFrame = (currentFrame + 1) % frameCount;
    }
    delay(animationDelay);
    timeSinceLastUpdate += animationDelay;
}

void startAnimation()
{
    Serial.println("Starting Animation!");
    int frameCount = (animation.length() + 1) / (LIGHT_COUNT + 1);
    Serial.println("frames: " + String(frameCount));
    currentFrame = 0;
    animating = true;
}

void applyLights(String ledsVal)
{
    Serial.println(ledsVal);

    for (int i = 0; i < LIGHT_COUNT; i++)
    {
        digitalWrite(PINS[i], ledsVal.charAt(i) == '1' ? LOW : HIGH);
    }
}
