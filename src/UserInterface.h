#ifndef TWAINFUL_USERINTERFACE_H
#define TWAINFUL_USERINTERFACE_H

#pragma once

#include "twain.h"
#include "main.h"
#include "CommonTwain.h"

string GetUnexpectedTypeErrorString(TW_UINT16 invalidType);

void PrintOptions();

void PrintMainCapabilities();

void PrintCapability(TW_UINT16 id, pTW_ONEVALUE pCapability);

void PrintCapability(TW_UINT16 id, pTW_ENUMERATION pCapability);

#endif //TWAINFUL_USERINTERFACE_H
