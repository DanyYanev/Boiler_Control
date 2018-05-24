#pragma once
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <doxygen.h>
#include <NexButton.h>
#include <NexCheckbox.h>
#include <NexConfig.h>
#include <NexCrop.h>
#include <NexDualStateButton.h>
#include <NexGauge.h>
#include <NexGpio.h>
#include <NexHardware.h>
#include <NexHotspot.h>
#include <NexNumber.h>
#include <NexObject.h>
#include <NexPage.h>
#include <NexPicture.h>
#include <NexProgressBar.h>
#include <NexRadio.h>
#include <NexRtc.h>
#include <NexScrolltext.h>
#include <NexSlider.h>
#include <NexText.h>
#include <NexTimer.h>
#include <Nextion.h>
#include <NexTouch.h>
#include <NexUpload.h>
#include <NexVariable.h>
#include <NexWaveform.h>

#define POOL_PUMP_EE 0
#define FLOOR_PUMP_EE 1
#define CONV_PUMP_EE 2
#define FLOOR_CONV_PUMP_EE 3
#define BOILER_SOURCE_EE 4
#define BOILER_STATE_EE 5
#define HEATING_SOURCE_EE 6
#define HEATING_STATE_EE 7
#define PRIORITY_EE 8
#define BTEMP_EE 10
#define BTEMP_SET_EE 32
#define BHIST_SET_EE 74
#define HTEMP_SET_EE 106
 
extern int RELAY1;
extern int RELAY2;
extern int RELAY3;
extern int RELAY4;
extern int RELAY5;
extern int RELAY6;
extern int RELAY7;
extern int RELAY8;
extern int RELAY9;
extern int RELAY10;
extern int RELAY11;
extern int RELAY12;
extern int RESET_PIN;

extern uint32_t BTemp;
extern uint32_t BTempSet;
extern uint32_t BHistSet;
extern uint32_t HTemp;
extern uint32_t HTempSet;

//Whether or not H requires adjusting.
extern int HState;
extern int HTimeOut;

extern bool PoolPump;
extern bool FloorPump;
extern bool ConvPump;
extern bool FloorConvPump;
extern bool BoilerSource;
extern bool BoilerPic;
extern bool BoilerState;
extern bool HeatingSource;
extern bool HeatingState;
extern bool HeatingPic;
extern bool Priority;
extern bool HInMotion;
extern bool BCharge;


extern OneWire BSensor;
extern OneWire HSensor;

extern DallasTemperature BSensors;
extern DallasTemperature HSensors;

extern NexPage page0;
extern NexPage page1;
extern NexPage page2;
extern NexPage page5;

extern NexNumber BTempN;
extern NexNumber BTempSetN;
extern NexNumber BHistSetN;
extern NexNumber HTempN;
extern NexNumber HTempSetN;

extern NexCrop buttonPoolPump;
extern NexCrop buttonBoilerSource; // Crop on Page 1 For source pic swich
extern NexCrop buttonHeatingSource; // Crop on Page 5 for source pic swich
extern NexCrop buttonBoilerSwichC;
extern NexCrop buttonHeatingSwichC;
extern NexCrop buttonPriority;
extern NexCrop buttonFloorPump;
extern NexCrop buttonConvPump;
extern NexCrop buttonFloorConvPump;

extern NexCrop buttonBTempUp;
extern NexCrop buttonBTempDown;
extern NexCrop buttonBHistUp;
extern NexCrop buttonBHistDown;
extern NexCrop buttonHTempUp;
extern NexCrop buttonHTempDown;

extern NexButton buttonBoiler;
extern NexButton buttonHeating;
extern NexButton buttonBSourceK;
extern NexButton buttonBSourceH;
extern NexButton buttonHSourceK;
extern NexButton buttonHSourceHP;
extern NexButton buttonBackB;
extern NexButton buttonBackS;
extern NexButton buttonBackH;
extern NexButton buttonReset;

void HTempUpdate();
void UpdateLogistics();

void Pic_Update(bool state, NexCrop button, int pageOn, int pageOff){
  uint32_t var;
  if(state)var = pageOn;
  else var = pageOff;
  button.setPic(var);
}

void UpdateSingleRelays(bool state, bool master, int relay1){  //Update Relay Function
  if(master){
    if(state == true){
      digitalWrite(relay1, LOW);
      //Serial.println("RELAY6 ON");
    }
    if(state == false){
      digitalWrite(relay1, HIGH);
      //Serial.println("RELAY6 OFF");
    }
  } else {
    digitalWrite(relay1, HIGH);
  }
}

void UpdateDoubleRelays(bool state, bool master, int relay1, int relay2){ 
  if(master){
    if(state){
      digitalWrite(relay1, LOW);
      //Serial.println("RELAY1 ON");
      digitalWrite(relay2, HIGH);
      //Serial.println("RELAY2 OFF");
    } else{
      digitalWrite(relay1, HIGH);
      //Serial.println("RELAY1 OFF");
      digitalWrite(relay2, LOW);
      //Serial.println("RELAY2 ON");
    }
  } else {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
  }
}

void Thermosthat(){
  if(!HInMotion && HeatingState && FloorPump){
    if(HTimeOut >= 2){ // will allow motor usage after 3 loops (3x3s)
       HTempUpdate();
       UpdateLogistics();
       if(HState == 1){ //Too hot
        Serial.println("TOO HOT");
        UpdateDoubleRelays(1, 1, RELAY9, RELAY10);
        HInMotion = true;
       } else if(HState == -1){ // Too cold
        Serial.println("TOO COLD");
        UpdateDoubleRelays(1, 1, RELAY10, RELAY9);
        HInMotion = true;
      }
    } else HTimeOut++;  
  } else {
    UpdateDoubleRelays(1, 0, RELAY9, RELAY10); //Turns both off.
    HInMotion = false;
    HTimeOut = 0; 
  } 
}

void UpdateSourceK(){
  bool K = false;
  
  if(BoilerState){
    if(BoilerSource == 1) K = true;
  }
  if(HeatingState){
    if(HeatingSource){
      if(FloorPump || ConvPump || FloorConvPump) K = true;
    }
  }
  if(PoolPump) K = true;
  //dbSerialPrintln("YOU WORKING?");
  UpdateSingleRelays(K, 1, RELAY8);
}

void UpdateSourceHP(){
  bool HP = false;
  if(HeatingState){
    if(!HeatingSource){ //if HeatPump source on
      if(FloorPump || ConvPump || FloorConvPump) HP = true;
    }
  }
  UpdateSingleRelays(HP, 1, RELAY7);
}

void UpdatePriority(){
  if(Priority && BoilerState && BoilerSource && HeatingPic && HeatingSource && (ConvPump || FloorConvPump || FloorPump)){
      HeatingState = false;
      UpdateSingleRelays(FloorPump, HeatingState, RELAY3);
      UpdateSingleRelays(ConvPump, HeatingState, RELAY4);
      UpdateSingleRelays(FloorConvPump, HeatingState, RELAY5);
      UpdateSourceK();
      UpdateSourceHP();
  } else {
      if(HeatingPic){
        HeatingState = true;
      } else {
        HeatingState = false;
      }
      UpdateSingleRelays(FloorPump, HeatingState, RELAY3);
      UpdateSingleRelays(ConvPump, HeatingState, RELAY4);
      UpdateSingleRelays(FloorConvPump, HeatingState, RELAY5);
      UpdateSourceK();
      UpdateSourceHP();
  }
}

void UpdateLogistics(){
  if(BoilerPic){
    if(BCharge){
      if(BTemp > BTempSet){
        BoilerState = false;
        UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
        BCharge = false;
      } else {
        BoilerState = true;
        UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
      }
    } else {
      if(BTemp > (BTempSet - BHistSet)){
        BoilerState = false;
        UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
      } else {
        BoilerState = true;
        UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
        BCharge = true;
      }
    }
  } else {
    BoilerState = false;
    UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
  }

  
  if(FloorPump && HeatingState){
    if(HTemp > HTempSet){
      HState = 1;
    } else if(HTemp < HTempSet){
      HState = -1;
    } else HState = 0;
  } else {
    HState = 0;
  }

  if(HState == 0){
      UpdateDoubleRelays(1, 0, RELAY9, RELAY10); //Turns both off.
      HInMotion = false;
      HTimeOut = 0; 
  }

  UpdatePriority();
}

void BTempUpdate(){
  BSensors.requestTemperatures();
  BTemp = BSensors.getTempCByIndex(0);
  BTempN.setValue(BTemp);
}

void HTempUpdate(){
  HSensors.requestTemperatures();
  HTemp = HSensors.getTempCByIndex(0);
  HTempN.setValue(HTemp);  
}

void TempUpdate(){
 
  //dbSerialPrintln("IN");
  BTempUpdate();
  HTempUpdate();
  //HTempN
  UpdateLogistics();
}

/*
   BUTTONS component callback function.
*/ 

//PAGE CHANGING
void buttonBoilerPushCallback(void *ptr)  //GO TO PAGE BOILER
{
  dbSerialPrintln("buttonBoilerPushCallback");
  page1.show();
  Pic_Update(BoilerSource, buttonBoilerSource, 6, 4);
  Pic_Update(Priority, buttonPriority, 6, 4);
  BTempSetN.setValue(BTempSet);
  BHistSetN.setValue(BHistSet);
}

void buttonHeatingPushCallback(void *ptr)  //GO TO PAGE HEATING
{
  dbSerialPrintln("buttonHeatingPushCallback");
  page5.show();
  Pic_Update(HeatingSource, buttonHeatingSource, 8, 5);
  Pic_Update(FloorPump, buttonFloorPump, 8, 5);
  Pic_Update(ConvPump, buttonConvPump, 8, 5);
  Pic_Update(FloorConvPump, buttonFloorConvPump, 8, 5);
  HTempSetN.setValue(HTempSet);
}

void buttonBack0PushCallback(void *ptr){  //GO BACK TO HOME PAGE
  dbSerialPrintln("Back to page 0");
  page0.show();
  TempUpdate();
  Pic_Update(PoolPump, buttonPoolPump, 7, 2);
  Pic_Update(BoilerPic, buttonBoilerSwichC, 7, 2);
  Pic_Update(HeatingPic, buttonHeatingSwichC, 7, 2);
}

//LOGISTIC FUNCTIONS
void buttonBoilerSwichCPushCallBack(void *ptr){ //BOILER ON OFF
  if(BoilerPic)BoilerPic = false;
  else BoilerPic = true;
  EEPROM.put(BOILER_STATE_EE, BoilerPic);
  
  dbSerialPrint("Boiler Swich");
  dbSerialPrint(BoilerPic);
  
  UpdateLogistics();
  //UpdatePriority();
  Pic_Update(BoilerPic, buttonBoilerSwichC, 7, 2);
}

void buttonHeatingSwichCPushCallBack(void *ptr){ //HEATING ON OFF 
  if(HeatingPic)HeatingPic = false;
  else HeatingPic = true;
  EEPROM.put(HEATING_STATE_EE, HeatingPic);
  Serial.print("PUTTING: ");
  Serial.println(HeatingPic); 
  
  dbSerialPrint("Heating Swich");
  dbSerialPrint(HeatingPic);
  
  //UpdatePriority();
  UpdateLogistics();
  
  Pic_Update(HeatingPic, buttonHeatingSwichC, 7, 2);
}

void buttonPriorityPushCallBack(void *ptr){
  dbSerialPrintln("buttonPriorityPushCallback");

  if(Priority) Priority = false;
  else Priority = true;
  EEPROM.put(PRIORITY_EE, Priority);

  UpdateLogistics();
  
  Pic_Update(Priority, buttonPriority, 6, 4);
}

void buttonBTempUpPushCallBack(void *ptr){
  dbSerialPrintln("buttonBTempUpPushCallBack");

  if(BTempSet < 75){
    BTempSet++;
    EEPROM.put(BTEMP_SET_EE, BTempSet);
    BTempSetN.setValue(BTempSet);
    UpdateLogistics();
  } else {
    BTempSet = 75;
//    EEPROM.put(BTEMP_SET_EE, BTempSet);
    BTempSetN.setValue(BTempSet);
    UpdateLogistics();
  }
}

void buttonBTempDownPushCallBack(void *ptr){
  dbSerialPrintln("buttonBTempDownPushCallBack");

  if(BTempSet > 65){
    BTempSet--;
    EEPROM.put(BTEMP_SET_EE, BTempSet);
    BTempSetN.setValue(BTempSet);
    UpdateLogistics();
  } else {
    BTempSet = 65;
    //EEPROM.put(BTEMP_SET_EE, BTempSet);
    BTempSetN.setValue(BTempSet);
    UpdateLogistics();
  }
}
 
void buttonBHistUpPushCallBack(void *ptr){
  dbSerialPrintln("buttonBHistUpPushCallBack");
  
  if(BHistSet < 15){
    BHistSet++;
    EEPROM.put(BHIST_SET_EE, BHistSet);
    BHistSetN.setValue(BHistSet);
    UpdateLogistics();
  } else {
    BHistSet = 15;
    //EEPROM.put(BHIST_SET_EE, BHistSet);
    BHistSetN.setValue(BHistSet);
    UpdateLogistics();
  }
}
  
void buttonBHistDownPushCallBack(void *ptr){
  dbSerialPrintln("buttonBHistDownPushCallBack");
  if(BHistSet > 2){
    BHistSet--;
    EEPROM.put(BHIST_SET_EE, BHistSet);
    BHistSetN.setValue(BHistSet);
    UpdateLogistics();
  } else {
    BHistSet = 2;
    //EEPROM.put(BHIST_SET_EE, BHistSet);
    BHistSetN.setValue(BHistSet);
    UpdateLogistics();
  }
}
void buttonHTempUpPushCallBack(void *ptr){
  dbSerialPrintln("buttonBTempUpPushCallBack");
  if(HTempSet < 40){
    HTempSet++;
    EEPROM.put(HTEMP_SET_EE, HTempSet);
    HTempSetN.setValue(HTempSet);
    UpdateLogistics();
  }else {
    HTempSet = 40;
    //EEPROM.put(HTEMP_SET_EE, HTempSet);
    HTempSetN.setValue(HTempSet);
    UpdateLogistics();
  }
}

void buttonHTempDownPushCallBack(void *ptr){
  dbSerialPrintln("buttonBTempDownPushCallBack");
  if(HTempSet > 30){
    HTempSet--;
    EEPROM.put(HTEMP_SET_EE, HTempSet);
    HTempSetN.setValue(HTempSet);
    UpdateLogistics();
  }else{
    HTempSet = 30;
    //EEPROM.put(HTEMP_SET_EE, HTempSet);
    HTempSetN.setValue(HTempSet);
    UpdateLogistics();
  }
}

void buttonBoilerSourceKSwapPushCallBack(void *ptr){   //BOILER SOURCE
  dbSerialPrintln("Boiler Source Swap to Kotel");

  BoilerSource = true;
  EEPROM.put(BOILER_SOURCE_EE, BoilerSource);
  UpdateLogistics();
  
  UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
  
  Pic_Update(BoilerSource, buttonBoilerSource, 6, 4);
}

void buttonBoilerSourceHSwapPushCallBack(void *ptr){    //BOILER SOURCE
  dbSerialPrintln("Boiler Source Swap to Heater");

  BoilerSource = false;
  EEPROM.put(BOILER_SOURCE_EE, BoilerSource);
  UpdateLogistics();
  
  UpdateDoubleRelays(BoilerSource, BoilerState, RELAY1, RELAY2);
  
  Pic_Update(BoilerSource, buttonBoilerSource, 6, 4);
}

void buttonHeatingSourceKSwapPushCallBack(void *ptr){   //HEATING SOURCE
  dbSerialPrintln("Heating Source Swap to Kotel");

  HeatingSource = true;
  EEPROM.put(HEATING_SOURCE_EE, HeatingSource);
  
  UpdatePriority();
  
  Pic_Update(HeatingSource, buttonHeatingSource, 8, 5);
}

void buttonHeatingSourceHPSwapPushCallBack(void *ptr){    //HEATING SOURCE
  dbSerialPrintln("Boiler Source Swap to El Pompa");

  HeatingSource = false;
  EEPROM.put(HEATING_SOURCE_EE, HeatingSource);
  
  UpdatePriority();
  
  Pic_Update(HeatingSource, buttonHeatingSource, 8, 5);
}

void buttonPoolPumpPushCallback(void *ptr){   //POOL PUMP ON OFF
  dbSerialPrintln("PoolPumpPopCallback");
  
  if(PoolPump)PoolPump = false;
  else PoolPump = true;
  EEPROM.put(POOL_PUMP_EE, PoolPump);

  UpdateSingleRelays(PoolPump, 1, RELAY6);
  UpdateSourceK();
  
  Pic_Update(PoolPump, buttonPoolPump, 7, 2);
  
}

void buttonFloorPumpPushCallback(void *ptr){   //FLOOR PUMP ON OFF
  dbSerialPrintln("FloorPumpPopCallback");
  
  if(FloorPump)FloorPump = false;
  else FloorPump = true;
  EEPROM.put(FLOOR_PUMP_EE, FloorPump);
  
  UpdatePriority();
  
  Pic_Update(FloorPump, buttonFloorPump, 8, 5);
}

void buttonConvPumpPushCallback(void *ptr){    //CONV PUMP ON OFF
  dbSerialPrintln("ConvPumpPopCallback");
  
  if(ConvPump)ConvPump = false;
  else ConvPump = true;
  EEPROM.put(CONV_PUMP_EE, ConvPump);
  
  UpdatePriority();

  Pic_Update(ConvPump, buttonConvPump, 8, 5);
}

void buttonFloorConvPumpPushCallback(void *ptr){   //FLOOR CONV PUMP ON OFF
  dbSerialPrintln("FloorConvPopCallback");
  
  if(FloorConvPump)FloorConvPump = false;
  else FloorConvPump = true;
  EEPROM.put(FLOOR_CONV_PUMP_EE, FloorConvPump);
  
  UpdatePriority();
  
  Pic_Update(FloorConvPump, buttonFloorConvPump, 8, 5);
}

void buttonResetPushCallback(void *ptr){
  Serial.println("Reseting");
  Serial3.println("Reset");
  digitalWrite(RESET_PIN, LOW);
//  delay(10);
//  digitalWrite(RESET_PIN, HIGH);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void parseValues(JsonArray&);

void serialEvent3() {
  while (Serial3.available()) {
    String data = Serial3.readString();
    Serial.println(data);

    if(data[0] == '{'){
      Serial.println("We got data wohoo");

      StaticJsonBuffer<500> jsonBuffer;

      char jsonbuff[500];

      data.toCharArray(jsonbuff, 500);

      JsonObject& root = jsonBuffer.parseObject(jsonbuff);
      
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
    
      // Fetch value_objects from Json file.
      if(atoi(root["token"]) != 12345){
        Serial.println("Wrong token");
      }
      
//      JsonArray& value_objects = root["value_objects_attributes"];
      
      parseValues(root["value_objects_attributes"]);
      
    }

    
  }
}

//JSON PARSERS

void parseValues(JsonArray& valueObjects){
  
  for(auto value_object : valueObjects){

    void * ptr;
        
    if(strcmp("PoolPump", value_object["key"].as<const char*>()) == 0){
      if(PoolPump != atoi(value_object["value"].as<const char*>())){
        buttonPoolPumpPushCallback(ptr);
      }
    } else if(strcmp("FloorPump", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonFloorPumpPushCallback(ptr);
      }
    } else if(strcmp("ConvPump", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonConvPumpPushCallback(ptr);
      }
    } else if(strcmp("FloorConvPump", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonFloorConvPumpPushCallback(ptr);
      }
//    } else if(strcmp("BoilerSource", value_object["key"].as<const char*>()) == 0){
//      if(FloorPump != atoi(value_object["value"].as<const char*>())){
//        buttonPoolPumpPushCallback();
//      }
    }else if(strcmp("BoilerPic", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonBoilerSwichCPushCallBack(ptr);
      }
//    }else if(strcmp("HeatingSource", value_object["key"].as<const char*>()) == 0){
//      if(FloorPump != atoi(value_object["value"].as<const char*>())){
//        buttonPoolPumpPushCallback();
//      }
    }else if(strcmp("HeatingPic", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonHeatingSwichCPushCallBack(ptr);
      }
    }else if(strcmp("Priority", value_object["key"].as<const char*>()) == 0){
      if(FloorPump != atoi(value_object["value"].as<const char*>())){
        buttonPriorityPushCallBack(ptr);
      }
    }else if(strcmp("BTempSet", value_object["key"].as<const char*>()) == 0){
      BTempSet = atoi(value_object["value"].as<const char*>());
    }else if(strcmp("HTempSet", value_object["key"].as<const char*>()) == 0){
      HTempSet = atoi(value_object["value"].as<const char*>());
    }
    
  }

//  page0.show();
  
}

