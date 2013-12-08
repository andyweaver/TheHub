// HouseWare Package Firmware
// Created by Andrew Fugier

// Includes
#include <SoftwareSerial.h>

// Common variables that chage
const char* VERSION = "0.0.2";   // version number
const int   DELAY   = 10;        // loop delay

SoftwareSerial mySerial =  SoftwareSerial(2, 3);    // XBee connected on pins D2 and D3
char str[20];    // used for most string operations - a single 20 byte memory segement
char out[20];    // buffer used for output storage
int pDoor = 0;   // state of the door

/* Sends the contents of out[] to the XBee and local Serial */
void sendMsg(){
  Serial.println(out);
  mySerial.println(out);
}

/* Firmware setup function */
void setup() {
  Serial.begin(9600);    // Local Serial Communications
  mySerial.begin(9600);  // XBee Communications

  strcpy(out, "Init HouseWare Hub");
  sendMsg();

  pinMode(4, INPUT);    // door pin
  pinMode(13, OUTPUT);  // light pin  
}

/* Read a message from the XBee interface */
/* Specify a char array as well as the array size */
int getMsg(char* m, int bsize){
  if (mySerial.available()) {                          // XBee has data
    int p = 0;                                         // buffer pos
    while(mySerial.available()){                       // Until buffer is empty
      if(mySerial.read() == '{'){                      // Start of JSON data
        while(mySerial.peek() != '}' && p < bsize){    // until the end of JSON data
          m[p++]=mySerial.read();                      // build the 'm' buffer
        }
        m[p++]=0;                                      // null terminate the 'm' buffer
      }
    }
    return 1;    // got a vailid message
  }
  return 0;      // no valid message
}

/* Process a givin JSON message */
/* Specify the char array */
int processMsg(char* m){
  switch(m[1]){
    case 'r':            // request
      switch(m[7]){
        case 'a':
          // TODO: code to return all sensor values
          break;
        default:
          // assume a pin xy  (pin > 50, analog - eg, pin 54 is analog 04)
          int t = 0;            // type:  0 -> Digital / 1 -> Analog
          int x = m[6] - '0';
          int y = m[7] - '0';
          int v = 0;            // value: sensor value
          
          // determine if digital or analog pin
          if(x >= 5) {
            t = 1;
            x = x - 5;
          }
          int z = (x * 10) + y; // the pin orginally requested, in int form
          
          if(t == 0){            // digital read
            v = digitalRead(z);
          }
          else if(t == 1){       // analog read
            v = analogRead(z);
          }
          
          // Build and return JSON
          sprintf(out, "{\"dat\":\"%i:%i\"}", (t*50)+z, v);
          sendMsg();
          break;
        }
      return 0;
      break;
      
    case 's':            // system
      switch(m[7]){
        case 'v':        // get version
          strcpy(out, "{\"dat\":\"v:");
          strcat(out, VERSION);
          strcat(out, "\"}");
          sendMsg();
          break;
        case 'r':       // reset system
          // TODO; Send notification of user reset
          asm volatile ("  jmp 0");    // hack-ish was to reset
          break;
      }
      return 0;
      break;
      
    case 'd':            // data
      return 0;
      break;
      
    case 'e':            // error
      return 0;
      break;
      
    // undefined
    strcpy(out, "{\"err\":001}");
    sendMsg();
    return -1;
  }
}

void loop() {
  // check the door
  if(digitalRead(4) != pDoor){
    processMsg("\"req\":04");
    pDoor = !pDoor;
  }
  
  // get and process messages
  if(getMsg(str, 20)){
    processMsg(str);
  }

  delay(DELAY);
}

