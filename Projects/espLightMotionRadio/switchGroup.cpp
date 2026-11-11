// Loading the required libraries
#include <PubSubClient.h>
#include "SwitchGroup.h"

/*
  mySwitch.send("010001010001010000000011");
  mySwitch.send("010001010001010000001100");
  mySwitch.send("010001010001010000110000");
  mySwitch.send("010001010001010011000000");
  mySwitch.send("010001010001011100000000");
*/

SwitchGroup::SwitchGroup(int switchPin, int pulseWidth, int protocol, String topicPrefix)
{
  mySwitch = RCSwitch();
  this->switchPin = switchPin < 0 ? SwitchPin : switchPin;
  this->pulseWidth = pulseWidth;
  this->protocol = protocol < 0 ? Protocol : protocol;

  // Topics
  baseTopic = topicPrefix + "/switch";
}

void SwitchGroup::setup(char** onMessages, char** offMessages, int numSwitches)
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

  Serial.print("Num switches: ");
  Serial.println(numSwitches);

  switchStatus = new bool[numSwitches];
  for(int i=0;i<numSwitches;i++)
  {
    Serial.print("On mesage[");
    Serial.print(i);
    Serial.print("] = ");
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
    for(int i=0;i<numSwitches;i++)
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
      Serial.print("Switching light to ");
      if(messageTemp == "1"){
        Serial.println("On");
      }
      else if(messageTemp == "0"){
        Serial.println("Off");
      }
    }
    else
    {
      Serial.print("No switch toggled");
    }
  }
}

void SwitchGroup::subscribe(PubSubClient& client) {
  client.subscribe(baseTopic.c_str());
}

void SwitchGroup::toggleSwitch(int switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    Serial.print("Toggling switch #");
    Serial.print(switchNumber);
    Serial.print(" using message ");
    Serial.println(switchStatus[switchNumber-1] ? offMessages[switchNumber-1] : onMessages[switchNumber-1]); 
    mySwitch.send(switchStatus[switchNumber-1] ? offMessages[switchNumber-1] : onMessages[switchNumber-1]);
    switchStatus[switchNumber-1] = !switchStatus[switchNumber-1];
  }
}

void SwitchGroup::turnOnSwitch(int switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    if(!switchStatus[switchNumber-1]) {
      toggleSwitch(switchNumber);
    }
  }
}

void SwitchGroup::turnOffSwitch(int switchNumber) 
{
  if(switchNumber >= 1 && switchNumber <= numSwitches)
  {
    if(switchStatus[switchNumber-1]) {
      toggleSwitch(switchNumber);
    }
  }
}

bool SwitchGroup::getSwitchStatus(int switchNumber)
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

