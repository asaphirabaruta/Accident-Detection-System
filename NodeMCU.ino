/* Code Written by Emile, Liliane and Asaph
 *  
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
WiFiClient client;
HTTPClient http;
// Update HOST URL here 192.168.182.130
#include <SoftwareSerial.h>
#define HOST "192.168.241.130"          // Enter HOST URL without "http:// "  and "/" at the end of URL

#define WIFI_SSID "emwfiSSID"            // WIFI SSID here                                   
#define WIFI_PASSWORD "em156789"        // WIFI password here

String date_accident ;
String latitude;
String longitude;
String drivernumber;
String Owner;
String Phone_NUmber;

String  sendval,postData;

SoftwareSerial mySerialGPS(D1, D2); // RX, TX pins for communication with NodeMCU

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc; // create an instance of the RTC object

void setup() {
  Serial.begin(115200); 
  mySerialGPS.begin(9600);
  Serial.println("Communication Started \n\n");  
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);     // initialize built in led on the board
  WiFi.mode(WIFI_STA);           
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) 
  {   Serial.print(".");
      delay(500); }
      Serial.println();
      Serial.print("Connected to ");
      Serial.println(WIFI_SSID);
      Serial.print("IP Address is : ");
      Serial.println(WiFi.localIP());    //print local IP address
      Wire.begin();
      rtc.begin(); // initialize the RTC module
      delay(30);
  }
void loop() { 

  HTTPClient http;    // http object of clas HTTPClient
  int r_number = random(10);
  int r_number2 = random(10);
  String strn = String(r_number);
  String strn2 = String(r_number2);
  String msg=mySerialGPS.readStringUntil('\r');
   int Strlength=msg.length();  // get the length of the string
  Serial.println("Length is:"+String(Strlength));
  if(Strlength>1)
  {
      //Get Data from arduino through serial communication 
        int comma1 = msg.indexOf(','); // Find the first comma
        int comma2 = msg.indexOf(',', comma1 + 1); // Find the second comma
        int comma3 = msg.indexOf(',', comma2 + 1); // Find the third comma
        
        date_accident = msg.substring(0, comma1); // Get the first value before the first comma
        latitude = msg.substring(comma1 + 1, comma2); // Get the second value between the first and second comma
        longitude = msg.substring(comma2 + 1, comma3); // Get the third value between the second and third comma
        drivernumber = msg.substring(comma3 + 1); // Get the fourth value after the third comma
        latitude=latitude+strn ;
        longitude=longitude+strn2;
        strn="";
        strn2="";
        Serial.print("Date: "); Serial.println(date_accident);
        Serial.print("Latitude: "); Serial.println(latitude);
        Serial.print("Longitude: "); Serial.println(longitude);
        Serial.print("Value: "); Serial.println(drivernumber);
      
        delay(1000); // Wait for 1 second before looping again
    }

  // Convert to struct tm format
    DateTime now = rtc.now();
  
  // Format the date and time as a string
    String dateTime = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

    date_accident =dateTime;
    latitude=latitude+strn ;
    longitude=longitude+strn2;
    strn="";
    strn2="";
    // Convert integer variables to string
    postData = "date_accident=" + date_accident + "&latitude=" + latitude + "&longitude=" + longitude + "&drivernumber=" + drivernumber + "&Owner=" + Owner + "&Phone_NUmber=" + Phone_NUmber;
  // Update Host URL here:-  
    http.begin(client,"http://192.168.241.130/dbwritephp.php");

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");            //Specify content-type header

    int httpCode = http.POST(postData);   // Send POST request to php file and store server response code in variable named httpCode
    Serial.println("Values are, date_accident = " + date_accident + " and latitude = "+latitude  + " and Phone_NUmber = "+Phone_NUmber);

    // if connection is established then do this
    if (httpCode == 200) 
    { 
      Serial.println("Values uploaded successfully."); Serial.println(httpCode); 
      String webpage = http.getString();   
      Serial.println(webpage + "\n"); 
    }

    // if failed to connect then return and restart

    else 
    { 
      Serial.println(httpCode); 
      Serial.println("Failed to upload values. \n"); 
      http.end(); 
      return; 
    }

    delay(3000); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(3000);
    digitalWrite(LED_BUILTIN, HIGH);

    }
    else
    {
       Serial.println("No accident. \n"); 
    }
}
