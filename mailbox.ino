#include <WiFiClientSecure.h>   // Include the HTTPS library
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <Servo.h> 

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

const char* host = "mail.google.com"; // the Gmail server
const char* url = "/mail/feed/atom";  // the Gmail feed url
const int httpsPort = 443;            // the port to connect to the email server

                                      // The SHA-1 fingerprint of the SSL certificate for the Gmail server (see below)
const char* fingerprint = "***";

                                      // The Base64 encoded version of your Gmail login credentials (see below)
const char* credentials = "***";

const int servoPin = 2;
Servo servo;

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  servo.attach(servoPin);
    
  wifiMulti.addAP("VODAFONE_D905", "***");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("AndroidAP", "***");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  Serial.println('\n');
}

void loop() {
  int unread = getUnread();
  if (unread == 0) {
    Serial.println("\r\nYou've got no unread emails");
    servo.write(0);
  } else if (unread > 0) {
    Serial.printf("\r\nYou've got %d new messages\r\n", unread);
    servo.write(90);
  } else {
    Serial.println("Could not get unread mails");
  }
  Serial.println('\n');
  delay(5000);
}

int getUnread() {    // a function to get the number of unread emails in your Gmail inbox
  WiFiClientSecure client; // Use WiFiClientSecure class to create TLS (HTTPS) connection
  client.setInsecure();
  Serial.printf("Connecting to %s:%d ... \r\n", host, httpsPort);
  if (!client.connect(host, httpsPort)) {   // Connect to the Gmail server, on port 443
    Serial.println("Connection failed");    // If the connection fails, stop and return
    return -1;
  }

  if (client.verify(fingerprint, host)) {   // Check the SHA-1 fingerprint of the SSL certificate
    Serial.println("Certificate matches");
  } else {                                  // if it doesn't match, it's not safe to continue
    Serial.println("Certificate doesn't match");
    return -1;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
               "Host: " + host + "\r\n" +
               "Authorization: Basic " + credentials + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n"); // Send the HTTP request headers

  Serial.println("Request sent");

  int unread = -1;

  while (client.connected()) {                          // Wait for the response. The response is in XML format
    client.readStringUntil('<');                        // read until the first XML tag
    String tagname = client.readStringUntil('>');       // read until the end of this tag to get the tag name
    if (tagname == "fullcount") {                       // if the tag is <fullcount>, the next string will be the number of unread emails
      String unreadStr = client.readStringUntil('<');   // read until the closing tag (</fullcount>)
      unread = unreadStr.toInt();                       // convert from String to int
      break;                                            // stop reading
    }                                                   // if the tag is not <fullcount>, repeat and read the next tag
  }
  Serial.println("Connection closed");

  return unread;                                        // Return the number of unread emails
}
