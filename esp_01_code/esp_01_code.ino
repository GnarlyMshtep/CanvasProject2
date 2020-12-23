#include <SoftwareSerial.h>  
SoftwareSerial esp8266(13,7);
#define DEBUG true 
void setup() {
  Serial.begin(115200);
  esp8266.begin(9600);
  Serial.println("Booting");
  InitWifiModule();
  sendData("AT+CIPSTART=0,\"TCP\",\"192.168.0.15\",3000\r\n",1000,DEBUG);
  String GET_url = "GET /products/5fe285e0963e11294c5a2b96 HTTP/1.1\r\n\r\n\r\n\r\n\r\n\r\n";
  //String POST_url = "POST /products HTTP/1.1";//, Host: 192.168.0.15:3000, User-Agent: Arduino/1.0, Connection: close\r\n\r\n\r\n\r\n\r\n\r\n";
  sendData("AT+CIPSEND=0,"+String(GET_url.length())+"\r\n",3000, DEBUG);
  sendData(GET_url, 3000, DEBUG);
  //delay(5000);
}

void loop() {
  //exit(0);
  //sendData("AT\r\n", 3000, DEBUG);
  //esp8266.print("AT\r\n");
  //delay(1000);
  // put your main code here, to run repeatedly:

}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}

void InitWifiModule()      
{      
  sendData("AT+RST\r\n", 2000, DEBUG);                                                        
  delay(100);      
  sendData("AT+CWJAP=\"BadWifi\",\"Sanmina02\"\r\n", 2000, DEBUG);                            
  delay (10000);      
  sendData("AT+CWMODE=1\r\n", 1500, DEBUG);                                                   
  delay (100);      
  sendData("AT+CIFSR\r\n", 1500, DEBUG);                                                      
  delay (100);      
  sendData("AT+CIPMUX=1\r\n", 1500, DEBUG);                                                   
  delay (100);      
  sendData("AT+CIPSERVER=1,80\r\n", 1500, DEBUG);   
  delay (100);                                          
}
