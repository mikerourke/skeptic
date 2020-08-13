#include "CommonTwain.h"

extern TW_MEMREF LockDsmMemory(TW_HANDLE memoryHandle);

extern void UnlockDsmMemory(TW_HANDLE memoryHandle);

extern TW_HANDLE AllocDsmMemory(TW_UINT32 memorySize);

extern void FreeDsmMemory(TW_HANDLE memoryHandle);

TW_FIX32 FloatToFix32(float floatValue) {
  TW_FIX32 fix32Value;
  TW_BOOL sign = floatValue < 0 ? TRUE : FALSE;
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
      auto pCapabilityPT = (pTW_ENUMERATION) LockDsmMemory(pCapability->hContainer);
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
      UnlockDsmMemory(pCapability->hContainer);

    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTW_ONEVALUE) LockDsmMemory(pCapability->hContainer);
      if (pCapabilityPT->ItemType < TWTY_FIX32) {
        value = pCapabilityPT->Item;
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    } else if (pCapability->ConType == TWON_RANGE) {
      auto pCapabilityPT = (pTW_RANGE) LockDsmMemory(pCapability->hContainer);
      if (pCapabilityPT->ItemType < TWTY_FIX32) {
        value = pCapabilityPT->CurrentValue;
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    }
  }

  return ret;
}


bool GetCurrent(TW_CAPABILITY *pCapability, string &value) {
  bool ret = false;

  if (pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTW_ENUMERATION) LockDsmMemory(pCapability->hContainer);
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

      UnlockDsmMemory(pCapability->hContainer);
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTW_ONEVALUE) LockDsmMemory(pCapability->hContainer);

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

      UnlockDsmMemory(pCapability->hContainer);
    }
  }

  return ret;
}

bool GetCurrent(TW_CAPABILITY *pCapability, TW_FIX32 &value) {
  bool ret = false;

  if (nullptr != pCapability->hContainer) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTWEnumerationFix32) LockDsmMemory(pCapability->hContainer);

      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = pCapabilityPT->ItemList[pCapabilityPT->CurrentIndex];
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTWOneValueFix32) LockDsmMemory(pCapability->hContainer);

      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = pCapabilityPT->Item;
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    } else if (pCapability->ConType == TWON_RANGE) {
      auto pCapabilityPT = (pTW_RANGE) LockDsmMemory(pCapability->hContainer);
      if (pCapabilityPT->ItemType == TWTY_FIX32) {
        value = *(TW_FIX32 *) &pCapabilityPT->CurrentValue;
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    }
  }

  return ret;
}

bool GetCurrent(TW_CAPABILITY *pCapability, TW_FRAME &frame) {
  bool ret = false;

  if (pCapability->hContainer != nullptr) {
    if (pCapability->ConType == TWON_ENUMERATION) {
      auto pCapabilityPT = (pTWEnumerationFrame) LockDsmMemory(pCapability->hContainer);

      if (pCapabilityPT->ItemType == TWTY_FRAME) {
        frame = pCapabilityPT->ItemList[pCapabilityPT->CurrentIndex];
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
    } else if (pCapability->ConType == TWON_ONEVALUE) {
      auto pCapabilityPT = (pTWOneValueFrame) LockDsmMemory(pCapability->hContainer);

      if (pCapabilityPT->ItemType == TWTY_FRAME) {
        frame = pCapabilityPT->Item;
        ret = true;
      }
      UnlockDsmMemory(pCapability->hContainer);
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
        auto pArray = (pTW_ARRAY) LockDsmMemory(pCapability->hContainer);
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTW_ENUMERATION) LockDsmMemory(pCapability->hContainer);
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

      UnlockDsmMemory(pCapability->hContainer);
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
        auto pArray = (pTW_ARRAY) LockDsmMemory(pCapability->hContainer);
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTW_ENUMERATION) LockDsmMemory(pCapability->hContainer);
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

      UnlockDsmMemory(pCapability->hContainer);
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
        auto pArray = (pTWArrayFix32) LockDsmMemory(pCapability->hContainer);
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTWEnumerationFix32) LockDsmMemory(
          pCapability->hContainer);
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount && itemType == TWTY_FIX32) {
        value = pData[item];
        ret = true;
      }

      UnlockDsmMemory(pCapability->hContainer);
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
        auto pArray = (pTWArrayFrame) LockDsmMemory(pCapability->hContainer);
        itemCount = pArray->NumItems;
        itemType = pArray->ItemType;
        pData = &pArray->ItemList[0];
      }

      if (pCapability->ConType == TWON_ENUMERATION) {
        auto pEnumeration = (pTWEnumerationFrame) LockDsmMemory(
          pCapability->hContainer);
        itemCount = pEnumeration->NumItems;
        itemType = pEnumeration->ItemType;
        pData = &pEnumeration->ItemList[0];
      }

      if (item < itemCount && itemType == TWTY_FRAME) {
        value = pData[item];
        ret = true;
      }

      UnlockDsmMemory(pCapability->hContainer);
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
    case TWTY_STR1024:
      return sizeof(TW_STR1024);
    case TWTY_UNI512:
      return sizeof(TW_UNI512);
    case TWTY_HANDLE:
      return sizeof(TW_HANDLE);
    default:
      return 0;
  }
}
