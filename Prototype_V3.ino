const unsigned int TRIG_PIN=13;
const unsigned int ECHO_PIN=12;
const unsigned int BAUD_RATE=9600;
int slpTime = 0;
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
String sensorArray[] = {
  "PV163A1", "PV163A2", "PV163A3", "PV163A4", "PV163B1", "PV163B2"
};
int sensorTrigArray[] = {
  13, 11, 9, 7, 5, 3
};
int sensorEchoArray[] = {
  12, 10, 8, 6, 4, 2
};
int pinCount = 6
const byte rxPin = 1; // Wire this to Tx Pin of ESP8266
const byte txPin = 0; // Wire this to Rx Pin of ESP8266
 
// We'll use a software serial interface to connect to ESP8266
SoftwareSerial ESP8266 (rxPin, txPin);
void setup() {
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(sensorTrigArray[thisPin], OUTPUT);
    pinMode(sensorEchoArray[thisPin], INPUT);
  }
  Serial.begin(BAUD_RATE);
  ESP8266.begin(9600); 
  WiFi.begin("SSID", "Password");   //WiFi connection
 
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
 
    delay(500);
    Serial.println("Waiting for connection");
 
  }
 attachInterrupt(0, digitalInterrupt, FALLING); //interrupt for waking up
  
  
  //SETUP WATCHDOG TIMER
WDTCSR = (24);//change enable and WDE - also resets
WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
WDTCSR |= (1<<6);//enable interrupt mode

  //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
  ADCSRA &= ~(1 << 7);
  
  //ENABLE SLEEP - this enables the sleep mode
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep
}

void loop() {
  slpTime = 1200;
   for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    slpTime = checkSensor(sensorTrigArray[thisPin], sensorEchoArray[thisPin], sensorArray[thisPin])
    delay(100);
  }
    sleepTime(slpTime);
 }
int checkSensor(int TRIG_PIN, int ECHO_PIN, String sensor) {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  

 const unsigned long duration= pulseIn(ECHO_PIN, HIGH);
 int distance= duration/29/2;
 if(duration!=0){
    if( distance < 40){
      return postFunction(sensor, 1);
    }else if (distance < 100){
      return postFunction(sensor, 5);  
    }else{
      return postFunction(sensor, 0);
    }
  }
}
int postFunction(String sensor, int fullness) {
       HTTPClient http;    //Declare object of class HTTPClient
 
       http.begin("http://192.168.1.88:8080/yourpat");      //Specify request destination
       http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header
     
       int httpCode = http.POST("sensor=" + sensor + "&fullness=" + fullness);   //Send the request
       String payload = http.getString();                  //Get the response payload
     
       return payload.toInt();
     
       http.end();  //Close connection
}

void sleepTime(int timeS) {
    ESP8266.deepSleep(timeS * 1000000);
    int otherSlpLoop = timeS/8;
    for (int i=0; i<otherSlpLoop; i++){
      //BOD DISABLE - this must be called right before the __asm__ sleep instruction
      MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
      MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
      __asm__  __volatile__("sleep");//in line assembler to go to sleep
    }
}
void digitalInterrupt(){
  //needed for the digital input interrupt
}

ISR(WDT_vect){
  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
}// watchdog interrupt
 
