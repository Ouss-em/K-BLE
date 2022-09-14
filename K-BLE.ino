/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
BLECharacteristic *characteristicTX;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//actuators :
#define PUMP 4
#define FAN 0
#define COMP 2
//Sensors:
#define AKS 34
#define PL 32
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const float OffSet = 0.488 ;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };


    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  float get_temperature(int pin){
    float r, v, t ;
    v = (float) analogRead(pin) * (5.0 / 1023.0);
    r = (float) ((v * 10000) / (5 -v));
    t = (0.2642 * r) - 251.6;  
    return t;
}
  void aktt(){
    pTxCharacteristic->setValue("  AK Temp: ");
    pTxCharacteristic->notify();
    float ttt = get_temperature(25); 
    char txString[8];
    dtostrf(ttt, 2, 2, txString); 
    pTxCharacteristic->setValue(txString);
    pTxCharacteristic->notify(); 
    Serial.println(ttt);
             if (ttt >= 60)
         {
          pTxCharacteristic->setValue(" WARNING!!"); 
          pTxCharacteristic->notify();
         }
}    


void hum_tem() {  //DHT
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  Serial.print("HT: ");
  Serial.println(h);
  float t = dht.readTemperature();
  Serial.print("TT: ");
  Serial.println(t);
  pTxCharacteristic->setValue("Tempurature: ");
  pTxCharacteristic->notify(); 
  char txString[8]; 
  dtostrf(t, 2, 2, txString); 
  pTxCharacteristic->setValue(txString); 
  pTxCharacteristic->notify(); 
}
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
    int i=0;
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
         Serial.println(rxValue[i]);               
          if (rxValue[i] == 'o'){
            digitalWrite(COMP, HIGH);
            digitalWrite(FAN, HIGH);
            Serial.println(" ON! ");
            pTxCharacteristic->setValue("The machine is turned on!");
            pTxCharacteristic->notify();            
          }
          else if (rxValue[i] == 'O'){
            digitalWrite(COMP, LOW);
            digitalWrite(FAN, LOW);  
            Serial.println(" OFF! ");
            pTxCharacteristic->setValue("The machine is turned off!");
            pTxCharacteristic->notify();
          }
          else if (rxValue[i] == 'c'){
            digitalWrite(COMP, HIGH);
            Serial.println(" COMRESSOR OFF ");
            pTxCharacteristic->setValue("COMRESSOR OFF");
            pTxCharacteristic->notify();      
          }
          else if (rxValue[i] == 'C'){
            digitalWrite(COMP, LOW);
            Serial.println(" COMRESSOR ON ");    
            pTxCharacteristic->setValue("COMRESSOR ON");
            pTxCharacteristic->notify();
          }   
          else if (rxValue[i] == 'f'){
            digitalWrite(FAN, HIGH); 
            Serial.println(" FAN OFF ");      
            pTxCharacteristic->setValue("FAN OFF");
            pTxCharacteristic->notify();
          }
          else if (rxValue[i] == 'F'){
            digitalWrite(FAN, LOW); 
            Serial.println(" FAN ON ");    
            pTxCharacteristic->setValue("FAN ON");
            pTxCharacteristic->notify();
          }
          else if (rxValue[i] == 'p'){
            digitalWrite(PUMP, HIGH);
            Serial.println(" PUMP OFF ");            
            pTxCharacteristic->setValue("PUMP OFF");
            pTxCharacteristic->notify();
          }
          else if (rxValue[i] == 'P'){
            digitalWrite(PUMP, LOW);
            Serial.println(" PUMP ON ");      
            pTxCharacteristic->setValue("PUMP ON");
            pTxCharacteristic->notify();
          }            
          else if (rxValue[i] == 'I'){
            digitalWrite(PUMP, LOW);
            Serial.println(" PUMP ON ");      
            hum_tem();
            aktt();
          }   
          Serial.println("*********");  
       }
   
    }
};

//Sensors fonctions:

/*void checkValues() {

  if ((t < T - 2) || (t > T + 2) )|| (get_temperature(25) < T - 200) || (get_temperature(25) > T + 200){
    Once = true;
  }
  else {
    Once =false;
  }
}



/*float waterpress() {   //Cap 119
  float reading = analogRead(PL);
  float v = (reading * 5) / 1023;
  float p = (9.084 * v) - 4.4665;
  return p;
  Serial.print("WP: ");
  Serial.println(p);
  Serial.println(" ");
  char txString[8]; 
  dtostrf(p, 2, 2, txString); 
  pTxCharacteristic->setValue(txString); 
  pTxCharacteristic->notify();
}
  

  void hum_tem() {  //DHT
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  Serial.print("HT: ");
  Serial.println(h);
  float t = dht.readTemperature();
  Serial.print("TT: ");
  Serial.println(t);
  pTxCharacteristic->setValue("Tempurature: ");
  pTxCharacteristic->notify(); 
  char txString[8]; 
  dtostrf(t, 2, 2, txString); 
  pTxCharacteristic->setValue(txString); 
  pTxCharacteristic->notify(); 
}
  float get_temperature(int pin){
    float r, v, t ;
    v = (float) analogRead(pin) * (5.0 / 1023.0);
    r = (float) ((v * 10000) / (5 -v));
    t = (0.2642 * r) - 251.6;  
    return t;
}
   void aktt(){
    pTxCharacteristic->setValue("  AK Temp: ");
    pTxCharacteristic->notify();
    float ttt = get_temperature(25); 
    char txString[8];
    dtostrf(ttt, 2, 2, txString); 
    pTxCharacteristic->setValue(txString);
    pTxCharacteristic->notify(); 
    Serial.println(ttt);
             if (ttt >= 60)
         {
          pTxCharacteristic->setValue(" WARNING!!"); 
          pTxCharacteristic->notify();
         }
   }*/

void setup() {
  Serial.begin(115200); 
  pinMode(PUMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(COMP, OUTPUT);
  pinMode(DHTPIN, INPUT);
  dht.begin();

  // Create the BLE Device
  BLEDevice::init("KUMULUS..BLEEE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY                  
									);
                      
 pTxCharacteristic->addDescriptor(new BLE2902());
  

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE 
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the services  
  
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
  float get_temperature(int pin){
    float r, v, t ;
    v = (float) analogRead(pin) * (5.0 / 1023.0);
    r = (float) ((v * 10000) / (5 -v));
    t = (0.2642 * r) - 251.6;  
    return t;
}
void loop() {

    if (deviceConnected) {
       // pTxCharacteristic->setValue(&txValue, 1);
       // pTxCharacteristic->notify();
       // txValue++;
       float v;
       v = (float) analogRead(25) * (5.0 / 1023.0);
       Serial.print(v);
 	    delay(2000); // bluetooth stack will go into congestion, if too many packets are sent
	  }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
