#ifndef TWAINFUL_TWAINSTRING_H
#define TWAINFUL_TWAINSTRING_H

#pragma once

#include "CommonTwain.h"

typedef const char *(*pStringConvertFuncType)(const TW_UINT16 item);

const char *CapabilityToString(TW_UINT16 capability);

const char *ConditionCodeToString(TW_UINT16 conditionCode);

const char *ImageFileFormatToExtension(TW_UINT16 imageFileFormat);

#endif //TWAINFUL_TWAINSTRING_H
