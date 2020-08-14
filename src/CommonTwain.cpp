#include "CommonTwain.h"

TW_FIX32 FloatToFix32(float floatValue) {
  TW_FIX32 fix32Value;
  TW_BOOL sign = floatValue < 0 ? 1 : 0;
  auto value = (TW_INT32) (floatValue * 65536.0 + (sign ? (-0.5) : 0.5));
  fix32Value.Whole = (TW_UINT16) value >> 16;
  fix32Value.Frac = (TW_UINT16) value & 0x0000ffffL;

  return fix32Value;
}

float Fix32ToFloat(const TW_FIX32 &fix32Value) {
  return float(fix32Value.Whole) + float(fix32Value.Frac / 65536.0);
}

bool GetCurrent(TW_CAPABILITY *pCapability, TW_UINT32 &value) {
  bool ret = false;

  if (pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTW_ENUMERATION) pCapability->hContainer;
      switch (pCapabilityPT->ItemType) {
        case TWTY_INT32:
          value = (TW_INT32) ((pTW_INT32) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_UINT32:
          value = (TW_INT32) ((pTW_UINT32) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_INT16:
          value = (TW_INT32) ((pTW_INT16) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_UINT16:
          value = (TW_INT32) ((pTW_UINT16) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_INT8:
          value = (TW_INT32) ((pTW_INT8) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_UINT8:
          value = (TW_INT32) ((pTW_UINT8) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

        case TWTY_BOOL:
          value = (TW_INT32) ((pTW_BOOL) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          ret = true;
          break;

      }
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTW_ONEVALUE) pCapability->hContainer;
      if (pCapabilityPT->ItemType < TWTY_FIX32) {
        value = pCapabilityPT->Item;
        ret = true;
      }
    } else if (pCapability->ConType == TWON_RANGE) {
      auto pCapabilityPT = (pTW_RANGE) pCapability->hContainer;
      if (pCapabilityPT->ItemType < TWTY_FIX32) {
        value = pCapabilityPT->CurrentValue;
        ret = true;
      }
    }
  }

  return ret;
}


bool GetCurrent(TW_CAPABILITY *pCapability, string &value) {
  bool ret = false;

  if (pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTW_ENUMERATION) pCapability->hContainer;
      switch (pCapabilityPT->ItemType) {
        case TWTY_STR32: {
          pTW_STR32 pStr = &((pTW_STR32) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          // In case the Capability is not null terminated:
          pStr[32] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR64: {
          pTW_STR64 pStr = &((pTW_STR64) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          // In case the Capability is not null terminated:
          pStr[64] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR128: {
          pTW_STR128 pStr = &((pTW_STR128) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          // In case the Capability is not null terminated:
          pStr[128] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR255: {
          pTW_STR255 pStr = &((pTW_STR255) (&pCapabilityPT->ItemList))[pCapabilityPT->CurrentIndex];
          // In case the Capability is not null terminated:
          pStr[255] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;
      }

    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTW_ONEVALUE) pCapability->hContainer;

      switch (pCapabilityPT->ItemType) {
        case TWTY_STR32: {
          pTW_STR32 pStr = ((pTW_STR32) (&pCapabilityPT->Item));
          // In case the Capability is not null terminated:
          pStr[32] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR64: {
          pTW_STR64 pStr = ((pTW_STR64) (&pCapabilityPT->Item));
          // In case the Capability is not null terminated:
          pStr[64] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR128: {
          pTW_STR128 pStr = ((pTW_STR128) (&pCapabilityPT->Item));
          // In case the Capability is not null terminated:
          pStr[128] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;

        case TWTY_STR255: {
          pTW_STR255 pStr = ((pTW_STR255) (&pCapabilityPT->Item));
          // In case the Capability is not null terminated:
          pStr[255] = 0;
          value = reinterpret_cast<const char *>(pStr);
          ret = true;
        }
          break;
      }
    }
  }

  return ret;
}

bool GetCurrent(TW_CAPABILITY *pCapability, TW_FIX32 &value) {
  bool ret = false;

  if (nullptr != pCapability->hContainer) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTWEnumerationFix32) pCapability->hContainer;

      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = pCapabilityPT->ItemList[pCapabilityPT->CurrentIndex];
        ret = true;
      }
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTWOneValueFix32) pCapability->hContainer;

      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = pCapabilityPT->Item;
        ret = true;
      }
    } else if (pCapability->ConType == TWON_RANGE) {
      auto pCapabilityPT = (pTW_RANGE) pCapability->hContainer;
      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = *(TW_FIX32 *) &pCapabilityPT->CurrentValue;
        ret = true;
      }
    }
  }

  return ret;
}

bool GetCurrent(TW_CAPABILITY *pCapability, TW_FRAME &frame) {
  bool ret = false;

  if (pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTWEnumerationFrame) pCapability->hContainer;

      if (pCapabilityPT->ItemType == TWTY_FRAME) {
        frame = pCapabilityPT->ItemList[pCapabilityPT->CurrentIndex];
        ret = true;
      }
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTWOneValueFrame) pCapability->hContainer;

      if (pCapabilityPT->ItemType == TWTY_FRAME) {
        frame = pCapabilityPT->Item;
        ret = true;
      }
    }
  }

  return ret;
}

bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_UINT32 &value) {
  bool ret = false;

  if (nullptr != pCapability && nullptr != pCapability->hContainer) {
    if (pCapability->ConType == TWON_ARRAY
        || pCapability->ConType == TWON_ENUMERATION) {
      TW_UINT8 *pData = nullptr;
      unsigned int itemCount = 0;
      TW_UINT16 itemType = 0;

      if (pCapability->ConType == TWON_ARRAY) {
        auto pArray = (pTW_ARRAY) pCapability->hContainer;
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTW_ENUMERATION) pCapability->hContainer;
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount) {
        switch (itemType) {
          case TWTY_INT32:
            value = (int) ((pTW_INT32) (pData))[item];
            ret = true;
            break;

          case TWTY_UINT32:
            value = (int) ((pTW_UINT32) (pData))[item];
            ret = true;
            break;

          case TWTY_INT16:
            value = (int) ((pTW_INT16) (pData))[item];
            ret = true;
            break;

          case TWTY_UINT16:
            value = (int) ((pTW_UINT16) (pData))[item];
            ret = true;
            break;

          case TWTY_INT8:
            value = (int) ((pTW_INT8) (pData))[item];
            ret = true;
            break;

          case TWTY_UINT8:
            value = (int) ((pTW_UINT8) (pData))[item];
            ret = true;
            break;

          case TWTY_BOOL:
            value = (int) ((pTW_BOOL) (pData))[item];
            ret = true;
            break;

          default:
            break;
        }
      }

    }
  }

  return ret;
}

bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, string &value) {
  bool ret = false;

  if (pCapability != nullptr && pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ARRAY
        || pCapability->ConType == TWON_ENUMERATION) {
      TW_UINT8 *pData = nullptr;
      unsigned int itemCount = 0;
      TW_UINT16 itemType = 0;

      if (pCapability->ConType == TWON_ARRAY) {
        auto pArray = (pTW_ARRAY) pCapability->hContainer;
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTW_ENUMERATION) pCapability->hContainer;
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount) {
        switch (itemType) {
          case TWTY_STR32: {
            pTW_STR32 pStr = &((pTW_STR32) (pData))[item];
            // In case the Capability is not null terminated:
            pStr[32] = 0;
            value = reinterpret_cast<const char *>(pStr);
            ret = true;
          }
            break;

          case TWTY_STR64: {
            pTW_STR64 pStr = &((pTW_STR64) (pData))[item];
            // In case the Capability is not null terminated:
            pStr[64] = 0;
            value = reinterpret_cast<const char *>(pStr);
            ret = true;
          }
            break;

          case TWTY_STR128: {
            pTW_STR128 pStr = &((pTW_STR128) (pData))[item];
            // In case the Capability is not null terminated:
            pStr[128] = 0;
            value = reinterpret_cast<const char *>(pStr);
            ret = true;
          }
            break;

          case TWTY_STR255: {
            pTW_STR255 pStr = &((pTW_STR255) (pData))[item];
            // In case the Capability is not null terminated:
            pStr[255] = 0;
            value = reinterpret_cast<const char *>(pStr);
            ret = true;
          }
            break;

          default:
            break;
        }
      }

    }
  }

  return ret;
}

bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_FIX32 &value) {
  bool ret = false;

  if (pCapability != nullptr && pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ARRAY
        || pCapability->ConType == TWON_ENUMERATION) {
      TW_FIX32 *pData = nullptr;
      unsigned int itemCount = 0;
      TW_UINT16 itemType = 0;

      if (pCapability->ConType == TWON_ARRAY) {
        auto pArray = (pTWArrayFix32) pCapability->hContainer;
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTWEnumerationFix32) pCapability->hContainer;
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount && itemType == TWTY_FIX32) {
        value = pData[item];
        ret = true;
      }

    }
  }

  return ret;
}

bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_FRAME &value) {
  bool ret = false;

  if (pCapability != nullptr && pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ARRAY
        || pCapability->ConType == TWON_ENUMERATION) {
      TW_FRAME *pData = nullptr;
      unsigned int itemCount = 0;
      TW_UINT16 itemType = 0;

      if (pCapability->ConType == TWON_ARRAY) {
        auto pArray = (pTWArrayFrame) pCapability->hContainer;
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTWEnumerationFrame) pCapability->hContainer;
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount && itemType == TWTY_FRAME) {
        value = pData[item];
        ret = true;
      }

    }
  }

  return ret;
}

int GetTwainTypeSize(TW_UINT16 itemType) {
  switch (itemType) {
    case TWTY_INT8:
      return sizeof(TW_INT8);
    case TWTY_INT16:
      return sizeof(TW_INT16);
    case TWTY_INT32:
      return sizeof(TW_INT32);
    case TWTY_UINT8:
      return sizeof(TW_UINT8);
    case TWTY_UINT16:
      return sizeof(TW_UINT16);
    case TWTY_UINT32:
      return sizeof(TW_UINT32);
    case TWTY_BOOL:
      return sizeof(TW_BOOL);
    case TWTY_FIX32:
      return sizeof(TW_FIX32);
    case TWTY_FRAME:
      return sizeof(TW_FRAME);
    case TWTY_STR32:
      return sizeof(TW_STR32);
    case TWTY_STR64:
      return sizeof(TW_STR64);
    case TWTY_STR128:
      return sizeof(TW_STR128);
    case TWTY_STR255:
      return sizeof(TW_STR255);
    default:
      return 0;
  }
}
