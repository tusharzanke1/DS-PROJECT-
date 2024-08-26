#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// GPS and WiFi client setup
TinyGPS gps;
SoftwareSerial sgps(5, 4);
WiFiClient client;

// IFTTT configuration
String MakerIFTTT_Key = "f53mlQx6YHFA6qodgtOt-Zpk6JoB_cSDo0hAv1XPxNl";
String MakerIFTTT_Event = "help_me";
const char* ssid = "Tushar";
const char* password = "000000000";
const char* host = "maker.ifttt.com";
const int httpsPort = 443;

// Ultrasonic sensor and motor/buzzer pins
const int trigpin1 = 12;
const int echopin1 = 14;
const int trigpin2 = 15;
const int echopin2 = 13;
const int motor = 0;
const int buzz = 2;
int distance1, distance2;
long duration1, duration2;

// Helper functions for IFTTT request
char* append_str(char* here, String s) {
  int i = 0;
  while (*here++ = s[i++]);
  return here - 1;
}

char* append_ul(char* here, unsigned long u) {
  char buf[20];
  return append_str(here, ultoa(u, buf, 10));
}

char post_rqst[256];
char* p;
char* content_length_here;
char* json_start;
int compi;

void setup() {
  Serial.begin(9600);
  pinMode(motor, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(trigpin1, OUTPUT);
  pinMode(echopin1, INPUT);
  pinMode(trigpin2, OUTPUT);
  pinMode(echopin2, INPUT);

  sgps.begin(9600);
  randomSeed(analogRead(A0));

  // WiFi connection
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Ultrasonic sensor readings
  digitalWrite(trigpin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin1, LOW);
  duration1 = pulseIn(echopin1, HIGH);
  distance1 = duration1 * 0.034 / 2;

  digitalWrite(trigpin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin2, LOW);
  duration2 = pulseIn(echopin2, HIGH);
  distance2 = duration2 * 0.034 / 2;

  Serial.print("distance1: ");
  Serial.println(distance1);
  Serial.print("distance2: ");
  Serial.println(distance2);

  // Motor and buzzer control logic
  if (distance1 > 70 && distance2 > 70) {
    digitalWrite(buzz, LOW);
    digitalWrite(motor, LOW);
  } else if ((distance1 < 15 && distance1 > 3) || (distance2 < 15 && distance2 > 3)) {
    digitalWrite(buzz, HIGH);
    digitalWrite(motor, HIGH);
  } else if ((distance1 < 30 && distance1 > 15) || (distance2 < 30 && distance2 > 15)) {
    digitalWrite(buzz, LOW);
    digitalWrite(motor, LOW);
    delay(300);
    digitalWrite(buzz, HIGH);
    digitalWrite(motor, HIGH);
    delay(300);
  } else if ((distance1 < 50 && distance1 > 30) || (distance2 < 50 && distance2 > 30)) {
    digitalWrite(buzz, LOW);
    digitalWrite(motor, LOW);
    delay(500);
    digitalWrite(buzz, HIGH);
    digitalWrite(motor, HIGH);
    delay(400);
  } else if ((distance1 < 70 && distance1 > 50) || (distance2 < 70 && distance2 > 50)) {
    digitalWrite(buzz, LOW);
    digitalWrite(motor, LOW);
    delay(700);
    digitalWrite(buzz, HIGH);
    digitalWrite(motor, HIGH);
    delay(400);
  } else {
    digitalWrite(buzz, HIGH);
    digitalWrite(motor, HIGH);
  }

  // GPS data processing and IFTTT trigger
  if (sgps.available()) {
    int c = sgps.read();
    if (gps.encode(c)) {
      float slat, slon;
      gps.f_get_position(&slat, &slon);
      Serial.print("Latitude :");
      Serial.println(slat, 6);
      Serial.print("Longitude:");
      Serial.println(slon, 6);

      if (client.connect("maker.ifttt.com", 80)) {
        p = post_rqst;
        p = append_str(p, "POST /trigger/");
        p = append_str(p, MakerIFTTT_Event);
        p = append_str(p, "/with/key/");
        p = append_str(p, MakerIFTTT_Key);
        p = append_str(p, " HTTP/1.1\r\n");
        p = append_str(p, "Host: maker.ifttt.com\r\n");
        p = append_str(p, "Content-Type: application/json\r\n");
        p = append_str(p, "Content-Length: ");
        content_length_here = p;
        p = append_str(p, "NN\r\n");
        p = append_str(p, "\r\n");
        json_start = p;
        p = append_str(p, "{\"value1\":\"");
        p = append_str(p, String(slat, 6));
        p = append_str(p, "\",\"value2\":\"");
        p = append_str(p, String(slon, 6));
        p = append_str(p, "\",\"value3\":\"\"}");

        compi = strlen(json_start);
        content_length_here[0] = '0' + (compi / 10);
        content_length_here[1] = '0' + (compi % 10);
        client.print(post_rqst);
        delay(5000);
      }
    }
  }
}
