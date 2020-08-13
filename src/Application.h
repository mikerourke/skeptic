#ifndef SKEPTIC_APPLICATION_H
#define SKEPTIC_APPLICATION_H

#pragma once

#include <string>
#include <vector>
#include <semaphore.h>
#include "CommonTwain.h"
#include "DsmInterface.h"
#include "TwainString.h"

using namespace std;

/**
* Determine if two TW_FIX32 variables are equal.
* @param[in] fix1 First TW_FIX32
* @param[in] fix2 Second TW_FIX32
* @return bool TRUE if equal
*/
bool operator==(const TW_FIX32 &fix1, const TW_FIX32 &fix2);

/**
* Output message to the command prompt.
* @param[in] pContents the message to display
* @param[in] ... additional variables
*/
void PrintMessage(const char *pContents, ...);

/** Populated TW_IDENTITY for the Application constructor */
TW_IDENTITY AppIdentity();

class Application {
public:
  Application();

  ~Application();

  TW_UINT16 CallDsm(
    TW_UINT16 dataGroup,
    TW_UINT16 dataArgumentType,
    TW_UINT16 message,
    TW_MEMREF pData);

  TW_INT16 PrintError(
    const string &errorMessage = "",
    pTW_IDENTITY pDestId = nullptr);

  TW_UINT16 GetConditionCode(pTW_IDENTITY pDestId, TW_INT16 &conditionCode);

  void Exit();

  void ConnectDsm();

  void DisconnectDsm();

  pTW_IDENTITY GetDefaultDataSource();

  pTW_IDENTITY SetDefaultDataSource(unsigned int index);

  pTW_IDENTITY SelectDefaultDataSource();

  void LoadDataSource(TW_INT32 id);

  void UnloadDataSource();

  pTW_IDENTITY GetDataSource(TW_INT16 index = -1) const;

  void GetDataSources();

  bool EnableDataSource();

  void DisableDataSource();

  bool UpdateImageInfo();

  void UpdateExtImageInfo();

  void InitiateNativeTransfer();

  void InitiateFileTransfer(TW_UINT16 fileFormat = TWFF_TIFF);

  void InitiateMemoryTransfer();

  TW_UINT16 DoAbortTransfer();

  TW_UINT16 SetCapabilityOneValue(
    TW_UINT16 id,
    int value,
    TW_UINT16 twainType);

  TW_UINT16 SetCapabilityOneValue(TW_UINT16 id, pTW_FIX32 pValue);

  TW_UINT16 SetCapabilityOneValue(TW_UINT16 id, pTW_FRAME pValue);

  TW_UINT16 SetCapabilityArray(
    TW_UINT16 capability,
    const int *pValues,
    int count,
    TW_UINT16 twainType);

  TW_INT16 GetCapability(TW_CAPABILITY &capability, TW_UINT16 message = MSG_GET);

  TW_INT16 QuerySupportCapability(TW_UINT16 id, TW_UINT32 &querySupport);

  TW_INT16 GetLabel(TW_UINT16 capabilityId, string &label);

  TW_INT16 GetHelp(TW_UINT16 capabilityId, string &help);

  pTW_IDENTITY GetAppIdentity() {
    return &mIdentity;
  }

  pTW_IMAGEINFO GetImageInfo() {
    return &mImageInfo;
  }

  string GetExtImageInfo() {
    return mExtImageInfoString;
  }

  void PrintAvailableDataSources();

  static void PrintIdentityStruct(const TW_IDENTITY &identity);

  void PrintMatchingIdentityStruct(TW_UINT32 identityId);

  void InitiateCapabilities();

  void TerminateCapabilities();

  void StartScan();

  bool GetImageBitDepth(TW_UINT16 &value);

  void SetImageBitDepth(TW_UINT16 value);

  bool GetImageCompression(TW_UINT16 &value);

  void SetImageCompression(TW_UINT16 value);

  bool GetImageFileFormat(TW_UINT16 &value);

  void SetImageFileFormat(TW_UINT16 value);

  bool GetImageFrames(TW_FRAME &value);

  void SetImageFrames(pTW_FRAME pValue);

  bool GetImagePixelType(TW_INT16 &value);

  void SetImagePixelType(TW_UINT16 value);

  bool GetImageTransferMechanism(TW_UINT16 &value);

  void SetImageTransferMechanism(TW_UINT16 value);

  bool GetImageUnits(TW_INT16 &value);

  void SetImageUnits(TW_UINT16 value);

  bool GetImageXResolution(TW_FIX32 &value);

  void SetImageXResolution(pTW_FIX32 pValue);

  bool GetImageYResolution(TW_FIX32 &value);

  void SetImageYResolution(pTW_FIX32 pValue);

  bool GetTransferCount(TW_INT16 &value);

  void SetTransferCount(TW_INT16 value);

  DsmState CurrentDsmState;
  TW_UINT16 DataSourceMessage;
  sem_t TwainEvent;

  /** Pixel bit depth for Current value of ICAP_PIXELTYPE. */
  TW_CAPABILITY ImageBitDepth;

  /** Compression method used for upcoming transfer. */
  TW_CAPABILITY ImageCompression;

  /** File format saved when using File Transfer Mechanism. */
  TW_CAPABILITY ImageFileFormat;

  /** Size and location of frames on page. */
  TW_CAPABILITY ImageFrames;

  /** The type of pixel data (B/W, gray, color, etc). */
  TW_CAPABILITY ImagePixelType;

  /** Used to learn options and set-up for upcoming transfer. */
  TW_CAPABILITY ImageTransferMechanism;

  /** Unit of measure (inches, centimeters, etc). */
  TW_CAPABILITY ImageUnits;

  /** Current/Available optical resolutions for x-axis. */
  TW_CAPABILITY ImageXResolution;

  /** Current/Available optical resolutions for y-axis */
  TW_CAPABILITY ImageYResolution;

  /** Number of images the application is willing to accept this session. */
  TW_CAPABILITY TransferCount;

private:
  TW_UINT16 callDsmControl(
    TW_UINT16 dataArgumentType,
    TW_UINT16 message,
    TW_MEMREF pData = nullptr);

  bool isDsmClosed() const;

  bool isDataSourceClosed() const;

  void setImageResolution(TW_UINT16 capabilityId, pTW_FIX32 pValue);

  static string validSavePath(string savePath);

  vector<TW_IDENTITY> mDataSources;
  string mExtImageInfoString;
  TW_UINT16 mGetHelpSupported;
  TW_UINT16 mGetLabelSupported;
  TW_IDENTITY mIdentity;
  TW_IMAGEINFO mImageInfo;
  pTW_IDENTITY mpDataSource;
  pTW_EXTIMAGEINFO mpExtImageInfo;
  string mSavePath;
  TW_UINT16 mTransferCount;
  TW_USERINTERFACE mUI;
};

#endif //SKEPTIC_APPLICATION_H
