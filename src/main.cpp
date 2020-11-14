#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>  
#include <ArduinoJson.h>


#define trigger       D0

float Kp,Ki,Kd;
uint8_t SetPoint,zaman;
String token,code;
String angleAdderss;

const char info[] ={"\r\n***************************************\r\n"
                        "************* AeroPendulum ************\r\n"
                        "******* Seyyed Amir Ali Masoumi *******\r\n"
                        "**** PhoneNumber: +98 930 927 1137 ****\r\n"
                        "********* Telegram: @Mashroti *********\r\n"
                        "****** Email: Mashroty@gmail.com ******\r\n"
                        "***************************************"};
//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setRxBufferSize(1000);

  pinMode(trigger,INPUT); 

    if(digitalRead(trigger) == HIGH)
    {
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      wifiManager.autoConnect("Aero Pendulum","9309271137");
    } 

  Serial1.println("");


  Serial1.print("Connecting to ");
  Serial1.print(WiFi.SSID());
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial1.print(".");
  }
  Serial1.println("");
  Serial.println(info);
  Serial1.println(info);
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop()
{
  HTTPClient http;    //Declare object of class HTTPClient
  DynamicJsonDocument doc(200);

  String payload;
  boolean chek = 1;
  int httpCode;
  
  while(chek)
  {
    http.begin("http://ap.damoon.pro/api/ap/status");            //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
    httpCode = http.POST("status=0");   //Send the request
    payload = http.getString();    //Get the response payload
    delay(1000);
    Serial1.println(httpCode);   //Print HTTP return code
    Serial1.println(payload);    //Print request response payload
    
    http.end();  //Close connection
    
    if(payload.indexOf("kp") > 0)
    {
      DeserializationError error = deserializeJson(doc, payload);
      if (error) 
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }   
      else
      {
        Kp = doc["kp"];
        Ki = doc["ki"];
        Kd = doc["kd"];
        SetPoint = doc["sp"];
        zaman = doc["time"];
        char *token_char = doc["token"];

        token = token_char;
        token += '/'; 

        char *code_char = doc["code"]; 
        code = code_char;

        angleAdderss = "http://ap.damoon.pro/api/ap/angle/";
        angleAdderss += token;
        angleAdderss += code; 
        chek =0;
      }  
    }
  }

  char i=0;
  while (i != 'Y')
  {
    for(uint8_t x=0 ; x<200 ; x++)
    {
      if (Serial.available())
      {
        i = Serial.read();
      }
      delay(1);
    }
    Serial.print("kp");
    Serial.print(Kp);
    Serial.print("ki");
    Serial.print(Ki);
    Serial.print("kd");
    Serial.print(Kd);
    Serial.print("time");
    Serial.print(zaman);
    Serial.print("sp");
    Serial.println(SetPoint);
  }

  while (i=='Y')
  {
    i = Serial.read();
  }
  
  
  String data_send = "angle=";;
  char buffer[600];
  int count=0;
  while (buffer[count-1]!='D')
  {
    http.begin(angleAdderss); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 

    count=0;
    while (buffer[count-1]!='E' && buffer[count-1]!='D')
    {
      if (Serial.available())
      {
        buffer[count++] = Serial.read();
      }
    }

      buffer[count-2]=0;
      data_send += buffer;

      Serial1.println(data_send); 

      httpCode = http.POST(data_send);
      payload = http.getString();
      Serial1.println(httpCode);
      Serial1.println(payload);  
      data_send = "angle=";

      http.end();  //Close connection
  }
    
}


