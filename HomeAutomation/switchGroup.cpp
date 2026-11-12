// Loading the required libraries
#include "SwitchGroup.h"

/*
  mySwitch.send("010001010001010000000011");
  mySwitch.send("010001010001010000001100");
  mySwitch.send("010001010001010000110000");
  mySwitch.send("010001010001010011000000");
  mySwitch.send("010001010001011100000000");
*/

SwitchGroup::SwitchGroup(uint8_t switchPin, int pulseWidth, uint8_t protocol, String topicPrefix)
{
  mySwitch = RCSwitch();
  this->switchPin = switchPin;
  this->pulseWidth = pulseWidth;
  this->protocol = protocol;

  // Topics
  baseTopic = topicPrefix + "/switch";
}

void SwitchGroup::setup(char** onMessages, char** offMessages, uint8_t numSwitches)
{
  this->numSwitches = numSwitches;

  //this->onMessages = new char*[numSwitches];
  //this->offMessages = new char*[numSwitches];

  // Doing this is not a good idea since it just uses the external pinter to the codes
  // i.e. the memory must be declared in the global environment for it to persist
  // It would be better if I copy the contents here.
  this->onMessages = onMessages;
  this->offMessages = offMessages;

  this->switchesToToggle = new bool[numSwitches];

  Serial.print(F("Num switches: "));
  Serial.println(numSwitches);

  switchStatus = new bool[numSwitches];
  for(uint8_t i=0;i<numSwitches;i++)
  {
    Serial.print(F("On mesage["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(this->onMessages[i]);
    switchStatus[i] = false;
  }
  
  //Setup the radio transmitter
  mySwitch.enableTransmit(switchPin);
  mySwitch.setPulseLength(pulseWidth);
  mySwitch.setProtocol(protocol);
  //mySwitch.setRepeatTransmit(15);
}

void SwitchGroup::processMessage(PubSubClient& client, String topic, String messageTemp, bool switchesToToggle[]) {
  if(topic==baseTopic.c_str()){
    bool lightsToggled = false;
    for(uint8_t i=0;i<numSwitches;i++)
    {
      if(this->switchesToToggle[i])
      {
        toggleSwitch(i+1);
        delay(50);
        lightsToggled = true;
      }
    }

    if(lightsToggled)
    {
      Serial.print(F("Switching light to "));
      if(messageTemp == "1"){
        Serial.println(F("On"));
      }
      else if(messageTemp == "0"){
        Serial.println(F("Off"));
      }
    }
    else
    {
      Serial.print(F("No switch toggled"));
    }
  }
}

void SwitchGroup::subscribe(PubSubClient& client) {
  client.subscribe(baseTopic.c_str());
}

void SwitchGroup::toggleSwitch(uint8_t switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    Serial.print(F("Toggling switch #"));
    Serial.print(switchNumber);
    Serial.print(F(" using message "));
    Serial.println(switchStatus[switchNumber-1] ? offMessages[switchNumber-1] : onMessages[switchNumber-1]); 
    mySwitch.send(switchStatus[switchNumber-1] ? offMessages[switchNumber-1] : onMessages[switchNumber-1]);
    switchStatus[switchNumber-1] = !switchStatus[switchNumber-1];
  }
}

void SwitchGroup::turnOnSwitch(uint8_t switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    if(!switchStatus[switchNumber-1]) {
      toggleSwitch(switchNumber);
    }
  }
}

void SwitchGroup::turnOffSwitch(uint8_t switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    if(switchStatus[switchNumber-1]) {
      toggleSwitch(switchNumber);
    }
  }
}

bool SwitchGroup::getSwitchStatus(uint8_t switchNumber)
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    return switchStatus[switchNumber-1];
  }

  return false;
}

/*
void SwitchGroup::setSwitchesToToggle(bool switchesToToggle[])
{
  for(int i=0;switchesToToggle && i<numSwitches;i++)
  {
    this->switchesToToggle[i] = switchesToToggle[i];
  }
}*/

