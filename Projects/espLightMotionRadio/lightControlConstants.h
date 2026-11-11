#ifndef _LIGHT_CONTROL_CONSTANTS_H_
#define _LIGHT_CONTROL_CONSTANTS_H_

const char* LightTopicPrefix = "/light";
const char* LightStatusTopic = "/status";
const char* LightModeTopic = "/mode";
const char* LightThesholdTopic = "/threshold";

const char* TurnOnMessage = "TurnLightOn";
const char* TurnOffMessage = "TurnLightOff";
const char* ControlByLightMessage = "ControlByLight";

const char* LightStatusArmedMessage = "Armed";

const char* LightControlModeMessages[] = { (char*)"Control by light", (char*)"Light off", (char*)"Light on" };

enum LightControlModeEnum { LightModeByLight = 0, LightModeOff = 1, LightModeOn = 2 };

#endif
