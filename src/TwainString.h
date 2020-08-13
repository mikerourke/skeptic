#ifndef SKEPTIC_TWAINSTRING_H
#define SKEPTIC_TWAINSTRING_H

#pragma once

#include "CommonTwain.h"

typedef const char *(*pStringConvertFuncType)(const TW_UINT16 item);

const char *CapabilityToString(TW_UINT16 capability);

const char *CapabilityItemToString(
  TW_UINT16 id,
  TW_UINT32 item,
  TW_UINT16 itemType);

const char *ConditionCodeToString(TW_UINT16 conditionCode);

const char *ImageFileFormatToExtension(TW_UINT16 imageFileFormat);

const char *TwainTypeToString(TW_UINT16 type);

#endif //SKEPTIC_TWAINSTRING_H
