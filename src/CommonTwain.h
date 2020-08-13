#ifndef TWAINFUL_COMMONTWAIN_H
#define TWAINFUL_COMMONTWAIN_H

#include "Common.h"

using namespace std;

#pragma pack (push, before_twain)

/** TW_ONEVALUE that holds a TW_FIX32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT16 Dummy;
  TW_FIX32 Item;
} TWOneValueFix32, FAR *pTWOneValueFix32;

/** TW_ONEVALUE that holds a TW_STR32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_STR32 Item;
} TWOneValueStr32, FAR *pTWOneValueStr32;

/** TW_ONEVALUE that holds a TW_STR64 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_STR64 Item;
} TWOneValueStr64, FAR *pTWOneValueStr64;

/** TW_ONEVALUE that holds a TW_STR128 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_STR128 Item;
} TWOneValueStr128, FAR *pTWOneValueStr128;

/** TW_ONEVALUE that holds a TW_STR255 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_STR255 Item;
} TWOneValueStr255, FAR *pTWOneValueStr255;

/** TW_ONEVALUE that holds a TW_FRAME item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT16 Dummy;
  TW_FRAME Item;
} TWOneValueFrame, FAR *pTWOneValueFrame;

/** TW_ARRAY that holds a TW_UINT8 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT8 ItemList[1];
} TWArrayUInt8, FAR *pTWArrayUInt8;

/** TW_ARRAY that holds a TW_UINT16 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT16 ItemList[1];
} TWArrayUInt16, FAR *pTWArrayUInt16;

/** TW_ARRAY that holds a TW_UINT32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 ItemList[1];
} TWArrayUInt32, FAR *pTWArrayUInt32;

/** TW_ARRAY that holds a TW_STR32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_STR32 ItemList[1];
} TWArrayStr32, FAR *pTWArrayStr32;

/** TW_ARRAY that holds a TW_FIX32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_FIX32 ItemList[1];
} TWArrayFix32, FAR *pTWArrayFix32;

/** TW_ARRAY that holds a TW_FRAME item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_FRAME ItemList[1];
} TWArrayFrame, FAR *pTWArrayFrame;

/** TW_ENUMERATION that holds a TW_BOOL item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_BOOL ItemList[1];
} TWEnumerationBool, FAR *pTWEnumerationBool;

/** TW_ENUMERATION that holds a TW_INT16 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_INT16 ItemList[1];
} TWEnumerationInt16, FAR *pTWEnumerationInt16;

/** TW_ENUMERATION that holds a TW_INT32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_INT32 ItemList[1];
} TWEnumerationInt32, FAR *pTWEnumerationInt32;

/** TW_ENUMERATION that holds a TW_UINT16 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_UINT16 ItemList[1];
} TWEnumerationUint16, FAR *pTWEnumerationUint16;

/** TW_ENUMERATION that holds a TW_UINT32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_UINT32 ItemList[1];
} TWEnumerationUint32, FAR *pTWEnumerationUint32;

/** TW_ENUMERATION that holds a TW_STR32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_STR32 ItemList[1];
} TWEnumerationStr32, FAR *pTWEnumerationStr32;

/** TW_ENUMERATION that holds a TW_STR64 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_STR64 ItemList[1];
} TWEnumerationStr64, FAR *pTWEnumerationStr64;

/** TW_ENUMERATION that holds a TW_STR128 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_STR128 ItemList[1];
} TWEnumerationStr128, FAR *pTWEnumerationStr128;

/** TW_ENUMERATION that holds a TW_STR255 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_STR255 ItemList[1];
} TWEnumerationStr255, FAR *pTWEnumerationStr255;

/** TW_ENUMERATION that holds a TW_FIX32 item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_FIX32 ItemList[1];
} TWEnumerationFix32, FAR *pTWEnumerationFix32;

/** TW_ENUMERATION that holds a TW_FRAME item */
typedef struct {
  TW_UINT16 ItemType;
  TW_UINT32 NumItems;
  TW_UINT32 CurrentIndex;
  TW_UINT32 DefaultIndex;
  TW_FRAME ItemList[1];
} TWEnumerationFrame, FAR *pTWEnumerationFrame;

#pragma pack (pop, before_twain)

#pragma pack(1)

typedef struct tagBitmapFileHeader {
  uint16_t FileType;
  uint32_t FileSize;
  uint16_t Reserved1;
  uint16_t Reserved2;
  uint32_t OffsetBits;
} BitmapFileHeader;

#pragma pack()

/**
* Get the current value from a Capability as a TW_UINT32.
* @param[in] pCapabilityability a pointer to the capability to retrieve the current value
* @param[out] value the value retrieved from the capability
* @return true if successful
*/
bool GetCurrent(TW_CAPABILITY *pCapability, TW_UINT32 &value);

/**
* Get the current value from a Capability as a string for capabilities of
* types TWTY_STR32, TWTY_STR64, TWTY_STR128, and TWTY_STR256
* @param[in] pCapability a pointer to the capability to retrieve the current value
* @param[out] value the value retrieved from the capability
* @return true if successful
*/
bool GetCurrent(TW_CAPABILITY *pCapability, string &value);

/**
* Get the current value from a Capability as a TW_FIX32.
* @param[in] pCapability a pointer to the capability to retrieve the current value
* @param[out] value the value retrieved from the capability
* @return true if successful
*/
bool GetCurrent(TW_CAPABILITY *pCapability, TW_FIX32 &value);

/**
* Get the current value from a Capability as a TW_FRAME.
* @param[in] pCapability a pointer to the capability to retrieve the current value
* @param[out] value the value retrieved from the capability
* @return true if successful
*/
bool GetCurrent(TW_CAPABILITY *pCapability, TW_FRAME &value);

/**
* Get an item value from an array of values from a TW_ENUMERATION or TW_ARRAY
* type Capability as a TW_UINT32.
* @param[in] pCapability a pointer to the capability to retrieve the value
* @pCount[in] item the 0 based location in the array to retrieve the item.
* @param[out] value the value retrieved from the capability
* @return true if successful. false if no value returned
*/
bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_UINT32 &value);

/**
* Get an item value from an array of values from a TW_ENUMERATION or TW_ARRAY
* containing types TWTY_STR32, TWTY_STR64, TWTY_STR128, and TWTY_STR256
* @param[in] pCapability a pointer to the capability to retrieve the value
* @pCount[in] item the 0 based location in the array to retrieve the item.
* @param[out] value the value retrieved from the capability
* @return true if successful. false if no value returned
*/
bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, string &value);

/**
* Get an item value from an array of values from a TW_ENUMERATION or TW_ARRAY
* containing type TWTY_FIX32
* @param[in] pCapability a pointer to the capability to retrieve the value
* @pCount[in] item the 0 based location in the array to retrieve the item.
* @param[out] value the value retrieved from the capability
* @return true if successful. false if no value returned
*/
bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_FIX32 &value);

/**
* Get an item value from an array of values from a TW_ENUMERATION or TW_ARRAY
* containing type TWTY_FRAME
* @param[in] pCapability a pointer to the capability to retrieve the value
* @pCount[in] item the 0 based location in the array to retrieve the item.
* @param[out] value the value retrieved from the capability
* @return true if successful. false if no value returned
*/
bool GetItem(TW_CAPABILITY *pCapability, TW_UINT32 item, TW_FRAME &value);

/**
* Get the size of TWAIN type
* @param[in] itemType the TWAIN type to return the size for
* @return the size of the type returned
*/
int GetTwainTypeSize(TW_UINT16 itemType);

#endif //TWAINFUL_COMMONTWAIN_H
