#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <CoreFoundation/CoreFoundation.h>

#include "TiffWriter.h"
#include "Application.h"
#include "TwainString.h"

FAR TW_UINT16 DSMCallback(
  pTW_IDENTITY pOrigin,
  pTW_IDENTITY pDest,
  TW_UINT32 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData);

bool operator==(const TW_FIX32 &fix1, const TW_FIX32 &fix2) {
  return (fix1.Whole == fix2.Whole) && (fix1.Frac == fix2.Frac);
}

void PrintMessage(const char *const pContents, ...) {
  char buffer[200];

  va_list vaList;
  va_start(vaList, pContents);
  vsnprintf(buffer, 200, pContents, vaList);
  va_end(vaList);

  cout << buffer << endl;
}

pTW_IDENTITY AppIdentity() {
  pTW_IDENTITY identity;

  identity->Id = nullptr;
  identity->Version.MajorNum = 1;
  identity->Version.MinorNum = 0;
  identity->Version.Language = TWLG_USA;
  identity->Version.Country = TWCY_USA;
  identity->ProtocolMajor = TWON_PROTOCOLMAJOR;
  identity->ProtocolMinor = TWON_PROTOCOLMINOR;
  identity->SupportedGroups = DG_CONTROL | DG_IMAGE;
  strcpy((char *) identity->Manufacturer, "Skeptic");
  strcpy((char *) identity->ProductFamily, "Skeptic");
  strcpy((char *) identity->ProductName, "Skeptic");

  return identity;
}

Application::Application() :
  DataSourceMessage((TW_UINT16) -1),
  CurrentDsmState(DsmState::Unknown),
  TwainEvent(0),
  mGetHelpSupported(TWCC_SUCCESS),
  mGetLabelSupported(TWCC_SUCCESS),
  mpDataSource(nullptr),
  mpExtImageInfo(nullptr),
  mTransferCount(0) {
  mIdentity = AppIdentity();
}

Application::~Application() {
  UnloadDsmLibrary();
  mDataSources.erase(mDataSources.begin(), mDataSources.end());
}

TW_UINT16 Application::CallDsm(
  TW_UINT16 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData
) {
  return DsmEntry(
    mIdentity,
    mpDataSource,
    dataGroup,
    dataArgumentType,
    message,
    pData);
}

TW_INT16 Application::PrintError(const string &errorMessage, pTW_IDENTITY pDestId) {
  TW_INT16 conditionCode = TWCC_SUCCESS;

  cerr << "Application: ";
  if (errorMessage.length() > 0) {
    cerr << errorMessage << endl;
  } else {
    cerr << "An error has occurred." << endl;
  }

  if (GetConditionCode(pDestId, conditionCode) == TWRC_SUCCESS) {
    cerr << "The condition code is: " << ConditionCodeToString(conditionCode) << endl;
  }

  return conditionCode;
}

TW_UINT16 Application::GetConditionCode(pTW_IDENTITY pDestId, TW_INT16 &conditionCode) {
  TW_STATUS status;
  memset(&status, 0, sizeof(TW_STATUS));

  TW_UINT16 returnCode = DsmEntry(
    mIdentity,
    pDestId,
    DG_CONTROL,
    DAT_STATUS,
    MSG_GET,
    (TW_MEMREF) &status);
  if (returnCode == TWRC_SUCCESS) {
    conditionCode = status.ConditionCode;
  }

  return returnCode;
}

void Application::Exit() {
  if (isDsmClosed()) {
    return;
  }

  // If we have selected a source, but it's null, disconnect and bail:
  if (CurrentDsmState >= DsmState::DataSourceLoaded && mpDataSource != nullptr) {
    if (CurrentDsmState >= DsmState::DataSourceEnabled) {
      DisableDataSource();

      if (CurrentDsmState >= DsmState::DataSourceEnabled) {
        DoAbortTransfer();
        CurrentDsmState = DsmState::DataSourceEnabled;
        DisableDataSource();
      }
    }

    UnloadDataSource();
  }

  DisconnectDsm();
}

void Application::ConnectDsm() {
  if (CurrentDsmState > DsmState::Open) {
    PrintMessage("The DSM has already been opened, close it first\n");
    return;
  }

  if (!LoadDsmLibrary()) {
    PrintMessage(
      "The DSM could not be opened. Please ensure that it is installed into a directory that is in the library path:");
    PrintMessage(kDSM_LIBRARY_PATH);
    return;
  }

  CurrentDsmState = DsmState::Closed;

  TW_UINT16 callResult = callDsmControl(DAT_PARENT, MSG_OPENDSM);
  if (callResult != TWRC_SUCCESS) {
    PrintMessage("DG_CONTROL / DAT_PARENT / MSG_OPENDSM Failed: %u\n", callResult);
    return;
  }

  PrintMessage("Successfully opened the DSM\n");
  CurrentDsmState = DsmState::Open;

  mDataSources.erase(mDataSources.begin(), mDataSources.end());
}

void Application::DisconnectDsm() {
  if (isDsmClosed()) {
    return;
  }

  TW_UINT16 callResult = callDsmControl(DAT_PARENT, MSG_CLOSEDSM);
  if (callResult == TWRC_SUCCESS) {
    PrintMessage("Successfully closed the DSM\n");
    CurrentDsmState = DsmState::Closed;
  } else {
    PrintError("Failed to close the DSM");
  }
}

TW_UINT16 Application::CallBack(TW_INT16 message) {
  TW_CALLBACK callback = { nullptr, nullptr, message };

  switch (message) {
    case MSG_XFERREADY:
      return callDsmControl(
        DAT_CALLBACK,
        MSG_INVOKE_CALLBACK,
        (TW_MEMREF) &callback);

    default:
      break;
  }

  return 0;
}

TW_IDENTITY gSource;

pTW_IDENTITY Application::GetDefaultDataSource() {
  if (isDsmClosed()) {
    return nullptr;
  }

  memset(&gSource, 0, sizeof(TW_IDENTITY));

  TW_UINT16 callResult = callDsmControl(
    DAT_IDENTITY,
    MSG_GETDEFAULT,
    (TW_MEMREF) &gSource);
  if (callResult == TWRC_FAILURE) {
    PrintError("Failed to get data source info!");
  }


  return &gSource;
}

pTW_IDENTITY Application::SetDefaultDataSource(unsigned int index) {
  if (isDsmClosed()) {
    return nullptr;
  }

  if (CurrentDsmState > DsmState::Open) {
    PrintMessage("A source has already been opened, please close it first\n");
    return nullptr;
  }

  if (index >= 0 && index < mDataSources.size()) {
    mpDataSource = &mDataSources[index];

    TW_UINT16 returnCode = callDsmControl(
      DAT_IDENTITY,
      MSG_SET,
      (TW_MEMREF) mpDataSource);
    if (returnCode == TWRC_FAILURE) {
      PrintError("Failed to get data source info!");
    }
  } else {
    return nullptr;
  }

  return mpDataSource;
}

pTW_IDENTITY Application::SelectDefaultDataSource() {
  if (isDsmClosed()) {
    return nullptr;
  }

  memset(&gSource, 0, sizeof(TW_IDENTITY));

  TW_UINT16 returnCode = callDsmControl(
    DAT_IDENTITY,
    MSG_USERSELECT,
    (TW_MEMREF) &gSource);
  switch (returnCode) {
    case TWRC_CANCEL:
      PrintError("Canceled select data source!");
      return nullptr;

    case TWRC_FAILURE:
      PrintError("Failed to select the data source!");
      return nullptr;

    default:
      return &gSource;
  }
}

void Application::LoadDataSource(const TW_INT32 id) {
  if (isDsmClosed()) {
    return;
  }

  if (CurrentDsmState > DsmState::Open) {
    PrintMessage("A source has already been opened, please close it first\n");
    return;
  }

  mGetLabelSupported = TWCC_SUCCESS;
  mGetHelpSupported = TWCC_SUCCESS;

  if (id > 0) {
    // Find the data source with Id = id:
    mpDataSource = nullptr;
    unsigned int index = 0;

    for (; index < mDataSources.size(); ++index) {
      auto dataSource = (pTW_IDENTITY) &mDataSources[index];
      // TODO: Find out if I broke this (since Id is a TW_MEMREF field for macOS).
      if (id == (TW_INT32) *dataSource->Id) {
        mpDataSource = dataSource;
        break;
      }
    }

    if (mpDataSource == nullptr) {
      PrintMessage("Data source with id: [%u] can not be found\n", id);
      return;
    }
  } else {
    // Open the default:
    memset(&gSource, 0, sizeof(TW_IDENTITY));
    mpDataSource = &gSource;
  }

  TW_CALLBACK callback = {nullptr};

  TW_UINT16 returnCode = callDsmControl(
    DAT_IDENTITY,
    MSG_OPENDS,
    (TW_MEMREF) mpDataSource);
  if (returnCode == TWRC_SUCCESS) {
    PrintMessage("Data source successfully opened!\n");
    CurrentDsmState = DsmState::DataSourceLoaded;

    callback.CallBackProc = (TW_MEMREF) DSMCallback;
    /*
     * RefCon on 32-bit could be used to store a pointer to this class to help
     * passing the message on to be processed. But RefCon is too small to store
     * a pointer on 64bit. For 64-bit, RefCon could store an index to some
     * global memory array. But if there is only one instance of the Application
     * Class connecting to the DSM then the single global pointer to the
     * application class can be used, and the RefCon can be ignored as we do here.
     */
    callback.RefCon = nullptr;

    returnCode = CallDsm(
      DG_CONTROL,
      DAT_CALLBACK,
      MSG_REGISTER_CALLBACK,
      (TW_MEMREF) &callback);
    if (returnCode != TWRC_SUCCESS) {
      PrintMessage(
        "DG_CONTROL / DAT_CALLBACK / MSG_REGISTER_CALLBACK Failed: %u\n",
        returnCode);
    }
  } else {
    PrintError("Failed to open data source.", mpDataSource);
    mpDataSource = nullptr;
  }
}

void Application::UnloadDataSource() {
  if (isDataSourceClosed()) {
    return;
  }

  TW_UINT16 returnCode = callDsmControl(
    DAT_IDENTITY,
    MSG_CLOSEDS,
    (TW_MEMREF) mpDataSource);
  if (returnCode == TWRC_SUCCESS) {
    PrintMessage("Data source successfully closed\n");

    CurrentDsmState = DsmState::Open;
    mpDataSource = nullptr;
  } else {
    PrintError("Failed to close data source.");
  }
}

pTW_IDENTITY Application::GetDataSource(TW_INT16 index) const {
  if (index < 0) {
    return mpDataSource;
  }

  if ((unsigned int) index < mDataSources.size()) {
    return (pTW_IDENTITY) &mDataSources[index];
  }

  return nullptr;
}

void Application::GetDataSources() {
  if (isDsmClosed()) {
    return;
  }

  assert(mDataSources.empty() == true);

  TW_IDENTITY dataSource;
  memset(&dataSource, 0, sizeof(TW_IDENTITY));

  TW_UINT16 returnCode = callDsmControl(
    DAT_IDENTITY,
    MSG_GETFIRST,
    (TW_MEMREF) &dataSource);

  switch (returnCode) {
    case TWRC_SUCCESS:
      mDataSources.push_back(dataSource);
      do {
        memset(&dataSource, 0, sizeof(TW_IDENTITY));
        returnCode = callDsmControl(
          DAT_IDENTITY,
          MSG_GETNEXT,
          (TW_MEMREF) &dataSource);

        switch (returnCode) {
          case TWRC_SUCCESS:
            mDataSources.push_back(dataSource);
            break;

          case TWRC_ENDOFLIST:
            break;

          case TWRC_FAILURE:
            PrintError("Failed to get the data source info!");
            break;

          default:
            break;

        }
      } while (returnCode == TWRC_SUCCESS);
      break;

    case TWRC_ENDOFLIST:
      break;

    case TWRC_FAILURE:
      PrintError("Failed to get the data source info!");
      break;

    default:
      break;
  }
}

bool Application::EnableDataSource() {
  if (isDataSourceClosed()) {
    return false;
  }

  CurrentDsmState = DsmState::DataSourceEnabled;

  mUI.ShowUI = FALSE;
  mUI.ModalUI = TRUE;
  mUI.hParent = nullptr;

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_USERINTERFACE,
    MSG_ENABLEDS,
    (TW_MEMREF) &mUI);
  if (returnCode != TWRC_SUCCESS && returnCode != TWRC_CHECKSTATUS) {
    CurrentDsmState = DsmState::DataSourceLoaded;
    PrintError("Cannot open source", mpDataSource);
    return false;
  }

  /*
   * Usually at this point the application sits here and waits for the scan to start.
   * We are notified that we can start a scan through the DSM's callback mechanism.
   * The callback was registered when the DSM was opened. If callbacks are not
   * being used, then the DSM will be polled to see when the DS is ready to start
   * scanning.
   */
  return true;
}

void Application::DisableDataSource() {
  if (CurrentDsmState < DsmState::DataSourceEnabled) {
    PrintMessage("You need to enable the data source first.\n");
    return;
  }

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_USERINTERFACE,
    MSG_DISABLEDS,
    (TW_MEMREF) &mUI);
  if (returnCode == TWRC_SUCCESS) {
    CurrentDsmState = DsmState::DataSourceLoaded;
  } else {
    PrintError("Cannot disable source", mpDataSource);
  }
}

bool Application::UpdateImageInfo() {
  // Clear our image info structure:
  memset(&mImageInfo, 0, sizeof(mImageInfo));

  PrintMessage("Getting the image info...\n");
  TW_UINT16 returnCode = CallDsm(
    DG_IMAGE,
    DAT_IMAGEINFO,
    MSG_GET,
    (TW_MEMREF) &mImageInfo);
  if (returnCode != TWRC_SUCCESS) {
    PrintError("Error while trying to get the image information!", mpDataSource);
  }

  return returnCode == TWRC_SUCCESS;
}

void Application::InitiateFileTransfer(TW_UINT16 fileFormat) {
  PrintMessage("app: Starting a TWSX_FILE transfer...\n");

  bool transfersPending = true;
  TW_UINT16 returnCode = TWRC_SUCCESS;
  string savePath = validSavePath(mSavePath);

  TW_SETUPFILEXFER fileTransfer;
  memset(&fileTransfer, 0, sizeof(fileTransfer));

  const char *pExt = ImageFileFormatToExtension(fileFormat);
  if (fileFormat == TWFF_TIFFMULTI) {
    Printf(
      reinterpret_cast<char *>(fileTransfer.FileName),
      sizeof(fileTransfer.FileName),
      "%sFROM_SCANNER_F%s",
      savePath.c_str(),
      pExt);
  } else {
    Printf(
      reinterpret_cast<char *>(fileTransfer.FileName),
      sizeof(fileTransfer.FileName),
      "%sFROM_SCANNER_%06dF%s",
      savePath.c_str(),
      mTransferCount,
      pExt);
  }
  fileTransfer.Format = fileFormat;

  while (transfersPending) {
    mTransferCount++;

    if (!UpdateImageInfo()) {
      break;
    }

    // The data returned by ImageInfo can be used to determine if this image is wanted.
    // If it is not then DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER can be
    // used to skip to the next image:
    if (fileFormat != TWFF_TIFFMULTI) {
      Printf(
        reinterpret_cast<char *>(fileTransfer.FileName),
        sizeof(fileTransfer.FileName),
        "%sFROM_SCANNER_%06dF%s",
        savePath.c_str(),
        mTransferCount,
        pExt);
    }

    PrintMessage("Sending file transfer details...\n");
    returnCode = CallDsm(
      DG_CONTROL,
      DAT_SETUPFILEXFER,
      MSG_SET,
      (TW_MEMREF) &fileTransfer);
    if (returnCode != TWRC_SUCCESS) {
      PrintError("Error while trying to setup the file transfer", mpDataSource);
      break;
    }

    PrintMessage("Starting file transfer...\n");
    returnCode = CallDsm(DG_IMAGE, DAT_IMAGEFILEXFER, MSG_GET, nullptr);

    switch (returnCode) {
      case TWRC_XFERDONE: {
        returnCode = CallDsm(
          DG_CONTROL,
          DAT_SETUPFILEXFER,
          MSG_GET,
          (TW_MEMREF) &fileTransfer);
        PrintMessage("File \"%s\" saved...\n", fileTransfer.FileName);

        PrintMessage("Checking to see if there are more images to transfer...\n");
        TW_PENDINGXFERS pendingTransfers;
        memset(&pendingTransfers, 0, sizeof(pendingTransfers));

        returnCode = CallDsm(
          DG_CONTROL,
          DAT_PENDINGXFERS,
          MSG_ENDXFER,
          (TW_MEMREF) &pendingTransfers);
        if (returnCode == TWRC_SUCCESS) {
          PrintMessage("Remaining images to transfer: %u\n", pendingTransfers.Count);

          if (pendingTransfers.Count == 0) {
            transfersPending = false;
          } else {
            PrintError("Failed to properly end the transfer", mpDataSource);
            transfersPending = false;
          }
        }
      }
        break;

      case TWRC_CANCEL:
        PrintError("Canceled transfer image");
        break;

      case TWRC_FAILURE:
        PrintError("Failed to transfer image");
        break;

      default:
        break;
    }
  }

  if (transfersPending) {
    DoAbortTransfer();
  }

  CurrentDsmState = DsmState::DataSourceEnabled;

  PrintMessage("File transfer complete!\n");
}

void Application::InitiateMemoryTransfer() {
  // For memory transfers, the FreeImage library will not be used, instead a
  // TIFF will be progressively written. This method was chosen because it
  // is possible that a 4GB image could be transferred and an image of that
  // size can not fit in most systems memory.
  PrintMessage("Starting a TWSX_MEMORY transfer...\n");

  TiffWriter *pTiffImage = nullptr;
  TW_STR255 outFileName;
  TW_SETUPMEMXFER sourcesBufferSizes;
  bool transfersPending = true;
  TW_UINT16 returnCode = TWRC_SUCCESS;
  string savePath = validSavePath(mSavePath);

  while (transfersPending) {
    mTransferCount++;
    memset(outFileName, 0, sizeof(outFileName));

    if (!UpdateImageInfo()) {
      break;
    }
    // The data returned by ImageInfo can be used to determine if this image is wanted.
    // If it is not then DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER can be
    // used to skip to the next image.

    // Set the filename to save to
    Printf(
      reinterpret_cast<char *>(outFileName),
      sizeof(outFileName),
      "%sFROM_SCANNER_%06dM.tif",
      savePath.c_str(),
      mTransferCount);

    // Get the buffer sizes that the source wants to use:
    PrintMessage("Getting the buffer sizes...\n");
    memset(&sourcesBufferSizes, 0, sizeof(sourcesBufferSizes));

    returnCode = CallDsm(
      DG_CONTROL,
      DAT_SETUPMEMXFER,
      MSG_GET,
      (TW_MEMREF) &(sourcesBufferSizes));

    if (TWRC_SUCCESS != returnCode) {
      PrintError("Error while trying to get the buffer sizes from the source!", mpDataSource);
      break;
    }

    // Setup a buffer to hold the strip from the data source. This buffer is a
    // template that will be used to reset the real buffer before each call to
    // get a strip:
    TW_IMAGEMEMXFER imageMemTransfer;
    imageMemTransfer.Compression = TWON_DONTCARE16;
    imageMemTransfer.BytesPerRow = TWON_DONTCARE32;
    imageMemTransfer.Columns = TWON_DONTCARE32;
    imageMemTransfer.Rows = TWON_DONTCARE32;
    imageMemTransfer.XOffset = TWON_DONTCARE32;
    imageMemTransfer.YOffset = TWON_DONTCARE32;
    imageMemTransfer.BytesWritten = TWON_DONTCARE32;

    imageMemTransfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
    imageMemTransfer.Memory.Length = sourcesBufferSizes.Preferred;

    auto memoryHandle = (TW_HANDLE) sizeof(sourcesBufferSizes.Preferred);
    if (memoryHandle == nullptr) {
      PrintError("Error allocating memory");
      break;
    }

    imageMemTransfer.Memory.TheMem = (TW_MEMREF) memoryHandle;

    // This is the real buffer that will be sent to the data source:
    TW_IMAGEMEMXFER memTransferBuffer;

    // This is set to true once one row has been successfully acquired. We have
    // to track this because we can't transition to state 7 until a row has been
    // received:
    bool wasScanStarted = false;

    int nBytePerRow = ((mImageInfo.ImageWidth * mImageInfo.BitsPerPixel) + 7) / 8;

    // Now that the memory has been setup, get the data from the scanner:
    PrintMessage("Starting the memory transfer...\n");
    for (;;) {
      // Reset the transfer buffer:
      memcpy(&memTransferBuffer, &imageMemTransfer, sizeof(imageMemTransfer));

      // Clear the row data buffer:
      memset(memTransferBuffer.Memory.TheMem, 0, memTransferBuffer.Memory.Length);

      // Get the row data:
      returnCode = CallDsm(
        DG_IMAGE,
        DAT_IMAGEMEMXFER,
        MSG_GET,
        (TW_MEMREF) &(memTransferBuffer));

      if (returnCode == TWRC_SUCCESS || returnCode == TWRC_XFERDONE) {
        if (!wasScanStarted) {
          // The state can be changed to state 7 now that we have successfully
          // received at least one strip:
          CurrentDsmState = DsmState::StripReceived;
          wasScanStarted = true;

          // Write the TIFF header now that all info needed for the header has
          // been received:
          pTiffImage = new TiffWriter(
            reinterpret_cast<char *>(outFileName),
            mImageInfo.ImageWidth,
            mImageInfo.ImageLength,
            mImageInfo.BitsPerPixel,
            nBytePerRow);

          pTiffImage->setXResolution(mImageInfo.XResolution.Whole, 1);
          pTiffImage->setYResolution(mImageInfo.YResolution.Whole, 1);

          pTiffImage->writeImageHeader();
        }

        char *pbuf = reinterpret_cast<char *>(memTransferBuffer.Memory.TheMem);

        // write the received image data to the image file
        for (unsigned int x = 0; x < memTransferBuffer.Rows; ++x) {
          pTiffImage->WriteTIFFData(pbuf, nBytePerRow);
          pbuf += memTransferBuffer.BytesPerRow;
        }

        if (returnCode == TWRC_XFERDONE) {
          // Deleting the TiffWriter object will close the file:
          if (pTiffImage) {
            delete pTiffImage;
            pTiffImage = nullptr;
          }

          PrintMessage("File \"%s\" saved...\n", outFileName);
          break;
        }
      } else if (returnCode == TWRC_CANCEL) {
        PrintError(
          "Canceled transfer while trying to get a strip of data from the source!",
          mpDataSource);
        break;
      } else if (returnCode == TWRC_FAILURE) {
        PrintError(
          "Error while trying to get a strip of data from the source!",
          mpDataSource);
        break;
      }
    }

    if (pTiffImage) {
      delete pTiffImage;
      pTiffImage = nullptr;
    }

    if (returnCode != TWRC_XFERDONE) {
      // We were not able to transfer an image don't try to transfer more:
      break;
    }

    // The transfer is done. Tell the source:
    PrintMessage("Checking to see if there are more images to transfer...\n");
    TW_PENDINGXFERS pendingTransfers;
    memset(&pendingTransfers, 0, sizeof(pendingTransfers));

    returnCode = CallDsm(
      DG_CONTROL,
      DAT_PENDINGXFERS,
      MSG_ENDXFER,
      (TW_MEMREF) &pendingTransfers);

    if (returnCode == TWRC_SUCCESS) {
      PrintMessage("Remaining images to transfer: %u\n", pendingTransfers.Count);
      if (pendingTransfers.Count == 0) {
        // Nothing left to transfer, finished.
        transfersPending = false;
      }
    } else {
      PrintError("Failed to properly end the transfer", mpDataSource);
      transfersPending = false;
    }

  }

  // Check to see if we left the scan loop before we were actually done scanning
  // This will happen if we had an error. We need to let the DS know we are not
  // going to transfer more images:
  if (transfersPending) {
    DoAbortTransfer();
  }

  // Adjust our state now that the scanning session is done:
  CurrentDsmState = DsmState::DataSourceEnabled;

  PrintMessage("Memory transfer complete!\n");
}

TW_UINT16 Application::DoAbortTransfer() {
  PrintMessage("Stop any transfer we may have started but could not finish...\n");

  TW_PENDINGXFERS pendingTransfers;
  memset(&pendingTransfers, 0, sizeof(pendingTransfers));

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_PENDINGXFERS,
    MSG_ENDXFER,
    (TW_MEMREF) &pendingTransfers);

  if (pendingTransfers.Count != 0) {
    memset(&pendingTransfers, 0, sizeof(pendingTransfers));

    returnCode = CallDsm(
      DG_CONTROL,
      DAT_PENDINGXFERS,
      MSG_RESET,
      (TW_MEMREF) &pendingTransfers);
  }

  return returnCode;
}

TW_UINT16 Application::SetCapabilityOneValue(
  TW_UINT16 id,
  const int value,
  TW_UINT16 twainType
) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = (TW_HANDLE) sizeof(TW_ONEVALUE);

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTW_ONEVALUE) capability.hContainer;
  pOneValue->ItemType = twainType;

  switch (twainType) {
    case TWTY_INT8:
      *(TW_INT8 *) &pOneValue->Item = (TW_INT8) value;
      break;

    case TWTY_INT16:
      *(TW_INT16 *) &pOneValue->Item = (TW_INT16) value;
      break;

    case TWTY_INT32:
      *(TW_INT32 *) &pOneValue->Item = (TW_INT32) value;
      break;

    case TWTY_UINT8:
      *(TW_UINT8 *) &pOneValue->Item = (TW_UINT8) value;
      break;

    case TWTY_UINT16:
      *(TW_UINT16 *) &pOneValue->Item = (TW_UINT16) value;
      break;

    case TWTY_UINT32:
      memcpy(&pOneValue->Item, &value, sizeof(TW_UINT32));
      break;

    case TWTY_BOOL:
      memcpy(&pOneValue->Item, &value, sizeof(TW_BOOL));
      break;

    default:
      break;
  }

  TW_INT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_SET,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_FAILURE) {
    PrintError("Could not set capability", mpDataSource);
  }
  return returnCode;
}

TW_UINT16 Application::SetCapabilityOneValue(TW_UINT16 id, const pTW_FIX32 pValue) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = (TW_HANDLE) sizeof(TWOneValueFix32);

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTWOneValueFix32) capability.hContainer;
  pOneValue->ItemType = TWTY_FIX32;
  pOneValue->Item = *pValue;

  TW_INT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_SET,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_FAILURE) {
    PrintError("Could not set capability", mpDataSource);
  }

  return returnCode;
}

TW_UINT16 Application::SetCapabilityOneValue(TW_UINT16 id, const pTW_FRAME pValue) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = (TW_HANDLE) sizeof(TWOneValueFrame);

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTWOneValueFrame) (TW_HANDLE) capability.hContainer;
  pOneValue->ItemType = TWTY_FRAME;
  pOneValue->Item = *pValue;

  TW_INT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_SET,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_FAILURE) {
    PrintError("Could not set capability", mpDataSource);
  }

  return returnCode;
}

TW_UINT16 Application::SetCapabilityArray(
  TW_UINT16 id,
  const int *pValues,
  int count,
  TW_UINT16 twainType
) {
  auto allocSize = sizeof(TW_ARRAY) + GetTwainTypeSize(twainType) * count;
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ARRAY;
  capability.hContainer = (TW_HANDLE) allocSize;

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pArray = (pTW_ARRAY) capability.hContainer;
  pArray->ItemType = twainType;
  pArray->NumItems = count;

  int index = 0;

  switch (twainType) {
    case TWTY_INT8:
      for (index = 0; index < count; index++) {
        ((pTW_INT8) (&pArray->ItemList))[index] = (TW_INT8) pValues[index];
      }
      break;

    case TWTY_INT16:
      for (index = 0; index < count; index++) {
        ((pTW_INT16) (&pArray->ItemList))[index] = (TW_INT16) pValues[index];
      }
      break;

    case TWTY_INT32:
      for (index = 0; index < count; index++) {
        ((pTW_INT32) (&pArray->ItemList))[index] = (TW_INT32) pValues[index];
      }
      break;

    case TWTY_UINT8:
      for (index = 0; index < count; index++) {
        ((pTW_UINT8) (&pArray->ItemList))[index] = (TW_UINT8) pValues[index];
      }
      break;

    case TWTY_UINT16:
      for (index = 0; index < count; index++) {
        ((pTW_UINT16) (&pArray->ItemList))[index] = (TW_UINT16) pValues[index];
      }
      break;

    case TWTY_UINT32:
      for (index = 0; index < count; index++) {
        ((pTW_UINT32) (&pArray->ItemList))[index] = (TW_UINT32) pValues[index];
      }
      break;

    case TWTY_BOOL:
      for (index = 0; index < count; index++) {
        ((pTW_BOOL) (&pArray->ItemList))[index] = (TW_BOOL) pValues[index];
      }
      break;

    default:
      break;
  }

  TW_INT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_SET,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_FAILURE) {
    PrintError("Could not set capability", mpDataSource);
  }

  return returnCode;
}

TW_INT16 Application::GetCapability(TW_CAPABILITY &capability, TW_UINT16 message) {
  switch (message) {
    case MSG_GET:
    case MSG_GETCURRENT:
    case MSG_GETDEFAULT:
    case MSG_RESET:
      break;

    default:
      PrintMessage("Bad message\n");
      return TWCC_BUMMER;
  }

  if (isDataSourceClosed()) {
    return TWCC_SEQERROR;
  }

  // Check if this capability structure has memory already allocated.
  // If it does, free that memory before the call else we'll have a memory
  // leak because the source allocates memory during a MSG_GET:
  if (capability.hContainer != nullptr) {
    capability.hContainer = nullptr;
  }

  capability.ConType = TWON_DONTCARE16;

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    message,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_FAILURE) {
    string errorMessage = "Failed to get the capability: [";
    errorMessage += CapabilityToString(capability.Cap);
    errorMessage += "]";

    return PrintError(errorMessage, mpDataSource);
  }

  return TWCC_SUCCESS;
}

TW_INT16 Application::QuerySupportCapability(
  TW_UINT16 id,
  TW_UINT32 &querySupport
) {
  if (isDataSourceClosed()) {
    return TWCC_SEQERROR;
  }

  TW_CAPABILITY capability = {0};
  capability.Cap = id;
  capability.hContainer = nullptr;
  capability.ConType = TWON_ONEVALUE;
  querySupport = 0;

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_QUERYSUPPORT,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_SUCCESS) {
    if (capability.ConType == TWON_ONEVALUE) {
      auto pOneValue = (pTW_ONEVALUE) capability.hContainer;
      querySupport = pOneValue->ItemType;
    }

  } else {
    string errorMessage = "Failed to query support the capability: [";
    errorMessage += CapabilityToString(id);
    errorMessage += "]";

    PrintError(errorMessage, mpDataSource);
  }

  return returnCode;
}

void Application::PrintAvailableDataSources() {
  if (isDsmClosed()) {
    return;
  }

  for (auto &dataSource : mDataSources) {
    PrintMessage(
      "%d: %.33s by %.33s\n",
      dataSource.Id,
      dataSource.ProductName,
      dataSource.Manufacturer);
  }
}

void Application::PrintIdentityStruct(const TW_IDENTITY &identity) {
  PrintMessage("\n Id: %u\n", identity.Id);
  PrintMessage("Version: %u.%u\n", identity.Version.MajorNum, identity.Version.MinorNum);
  PrintMessage("SupportedGroups: %u\n", identity.SupportedGroups);
  PrintMessage("Manufacturer: %s\n", identity.Manufacturer);
  PrintMessage("ProductFamily: %s\n", identity.ProductFamily);
  PrintMessage("ProductName: %s\n", identity.ProductName);
}

void Application::PrintMatchingIdentityStruct(const TW_UINT32 identityId) {
  for (auto &dataSource : mDataSources) {
    if (*dataSource.Id == identityId) {
      PrintIdentityStruct(dataSource);
      break;
    }
  }
}

void Application::InitiateCapabilities() {
  if (isDsmClosed()) {
    return;
  }

  if (isDataSourceClosed()) {
    return;
  }

  memset(&ImageBitDepth, 0, sizeof(TW_CAPABILITY));
  ImageBitDepth.Cap = ICAP_BITDEPTH;
  GetCapability(ImageBitDepth);

  memset(&ImageCompression, 0, sizeof(TW_CAPABILITY));
  ImageCompression.Cap = ICAP_COMPRESSION;
  GetCapability(ImageCompression);

  memset(&ImageFileFormat, 0, sizeof(TW_CAPABILITY));
  ImageFileFormat.Cap = ICAP_IMAGEFILEFORMAT;
  GetCapability(ImageFileFormat);

  memset(&ImageFrames, 0, sizeof(TW_CAPABILITY));
  ImageFrames.Cap = ICAP_FRAMES;
  GetCapability(ImageFrames);

  memset(&ImagePixelType, 0, sizeof(TW_CAPABILITY));
  ImagePixelType.Cap = ICAP_PIXELTYPE;
  GetCapability(ImagePixelType);

  memset(&ImageTransferMechanism, 0, sizeof(TW_CAPABILITY));
  ImageTransferMechanism.Cap = ICAP_XFERMECH;
  GetCapability(ImageTransferMechanism);

  memset(&ImageUnits, 0, sizeof(TW_CAPABILITY));
  ImageUnits.Cap = ICAP_UNITS;
  GetCapability(ImageUnits);

  memset(&ImageXResolution, 0, sizeof(TW_CAPABILITY));
  ImageXResolution.Cap = ICAP_XRESOLUTION;
  GetCapability(ImageXResolution);

  memset(&ImageYResolution, 0, sizeof(TW_CAPABILITY));
  ImageYResolution.Cap = ICAP_YRESOLUTION;
  GetCapability(ImageYResolution);

  memset(&TransferCount, 0, sizeof(TW_CAPABILITY));
  TransferCount.Cap = CAP_XFERCOUNT;
  GetCapability(TransferCount);
}

void Application::TerminateCapabilities() {
  if (isDsmClosed()) {
    return;
  }

  if (isDataSourceClosed()) {
    return;
  }

  if (ImageBitDepth.hContainer) {
    ImageBitDepth.hContainer = nullptr;
  }

  if (ImageCompression.hContainer) {
    ImageCompression.hContainer = nullptr;
  }

  if (ImageFrames.hContainer) {
    ImageFrames.hContainer = nullptr;
  }

  if (ImageFileFormat.hContainer) {
    ImageFileFormat.hContainer = nullptr;
  }

  if (ImagePixelType.hContainer) {
    ImagePixelType.hContainer = nullptr;
  }

  if (ImageTransferMechanism.hContainer) {
    ImageTransferMechanism.hContainer = nullptr;
  }

  if (ImageUnits.hContainer) {
    ImageUnits.hContainer = nullptr;
  }

  if (ImageXResolution.hContainer) {
    ImageXResolution.hContainer = nullptr;
  }

  if (ImageYResolution.hContainer) {
    ImageYResolution.hContainer = nullptr;
  }

  if (TransferCount.hContainer) {
    TransferCount.hContainer = nullptr;
  }
}

void Application::StartScan() {
  if (CurrentDsmState != DsmState::ReadyToScan) {
    PrintError("A scan cannot be initiated unless we are in state 6", mpDataSource);
    return;
  }

  TW_UINT16 transferMechanism;
  if (!GetImageTransferMechanism(transferMechanism)) {
    PrintError("Error: could not get the transfer mechanism", mpDataSource);
    return;
  }

  switch (transferMechanism) {
    case TWSX_NATIVE:
      break;

    case TWSX_FILE: {
      TW_UINT16 fileFormat = TWFF_TIFF;
      if (!GetImageFileFormat(fileFormat)) {
        fileFormat = TWFF_TIFF;
      }
      InitiateFileTransfer(fileFormat);
    }
      break;

    case TWSX_MEMORY:
      InitiateMemoryTransfer();
      break;

    default:
      break;
  }
}

bool Application::GetImageBitDepth(TW_UINT16 &value) {
  TW_UINT32 bitDepth;
  bool current = GetCurrent(&ImageBitDepth, bitDepth);
  value = (TW_UINT16) bitDepth;
  return current;
}

void Application::SetImageBitDepth(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_BITDEPTH, value, TWTY_UINT16);

  if (GetCapability(ImageBitDepth) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 bitDepth;
  if (GetImageBitDepth(bitDepth) && bitDepth == value) {
    PrintMessage("Image Bit Depth successfully set!\n");
  }
}

bool Application::GetImageCompression(TW_UINT16 &value) {
  TW_UINT32 compression;
  bool current = GetCurrent(&ImageCompression, compression);
  value = (TW_UINT16) compression;
  return current;
}

void Application::SetImageCompression(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_COMPRESSION, value, TWTY_UINT16);

  if (GetCapability(ImageCompression) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 compression;
  if (GetImageCompression(compression) && compression == value) {
    PrintMessage("Image Compression successfully set!\n");
  }
}

bool Application::GetImageFileFormat(TW_UINT16 &value) {
  TW_UINT32 fileFormat;
  bool current = GetCurrent(&ImageFileFormat, fileFormat);
  value = (TW_UINT16) fileFormat;
  return current;
}

void Application::SetImageFileFormat(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_IMAGEFILEFORMAT, value, TWTY_UINT16);

  if (GetCapability(ImageFileFormat) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 fileFormat;
  if (GetImageFileFormat(fileFormat) && fileFormat == value) {
    PrintMessage("Image File Format successfully set!\n");
  }
}

bool Application::GetImageFrames(TW_FRAME &value) {
  return GetCurrent(&ImageFrames, value);
}

void Application::SetImageFrames(const pTW_FRAME pValue) {
  SetCapabilityOneValue(ICAP_FRAMES, pValue);

  if (GetCapability(ImageFrames) != TWCC_SUCCESS) {
    return;
  }

  if (ImageFrames.ConType == TWON_ENUMERATION && ImageFrames.hContainer != nullptr) {
    auto pEnumFrame = (pTWEnumerationFrame) ImageFrames.hContainer;
    auto pFrame = &pEnumFrame->ItemList[pEnumFrame->CurrentIndex];

    if (
      pFrame->Bottom == pValue->Bottom &&
      pFrame->Top == pValue->Top &&
      pFrame->Left == pValue->Left &&
      pFrame->Right == pValue->Right
      ) {
      PrintMessage("Image Frames successfully set!\n");
    }
  }
}

bool Application::GetImagePixelType(TW_INT16 &value) {
  TW_UINT32 pixelType;
  bool current = GetCurrent(&ImagePixelType, pixelType);
  value = (TW_UINT16) pixelType;
  return current;
}

void Application::SetImagePixelType(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_PIXELTYPE, value, TWTY_UINT16);

  if (GetCapability(ImagePixelType) != TWCC_SUCCESS) {
    return;
  }

  if (ImagePixelType.ConType == TWON_ENUMERATION && ImagePixelType.hContainer != nullptr) {
    auto pEnum = (pTW_ENUMERATION) ImagePixelType.hContainer;
    auto currentItem = ((TW_UINT16 *) &pEnum->ItemList)[pEnum->CurrentIndex];

    if (currentItem == value) {
      PrintMessage("Image Pixel Type successfully set!\n");
    }
  }

  GetCapability(ImageBitDepth);
}

bool Application::GetImageTransferMechanism(TW_UINT16 &value) {
  TW_UINT32 mechanism;
  bool current = GetCurrent(&ImageTransferMechanism, mechanism);
  value = (TW_UINT16) mechanism;
  return current;
}

void Application::SetImageTransferMechanism(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_XFERMECH, value, TWTY_UINT16);

  if (GetCapability(ImageTransferMechanism) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 mechanism;
  if (GetImageTransferMechanism(mechanism) && mechanism == value) {
    PrintMessage("Image Transfer Mechanism successfully set!\n");
  }
}

bool Application::GetImageUnits(TW_INT16 &value) {
  TW_UINT32 units;
  bool current = GetCurrent(&ImageUnits, units);
  value = (TW_UINT16) units;
  return current;
}

void Application::SetImageUnits(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_UNITS, value, TWTY_UINT16);

  if (GetCapability(ImageUnits) != TWCC_SUCCESS) {
    return;
  }

  if (ImageUnits.ConType == TWON_ENUMERATION && ImageUnits.hContainer != nullptr) {
    auto pEnum = (pTW_ENUMERATION) ImageUnits.hContainer;

    if (pEnum->ItemList[pEnum->CurrentIndex] == value) {
      PrintMessage("Image Units successfully set!\n");

      GetCapability(ImageXResolution);
      GetCapability(ImageYResolution);
    }
  }
}

bool Application::GetImageXResolution(TW_FIX32 &value) {
  return GetCurrent(&ImageXResolution, value);
}

void Application::SetImageXResolution(const pTW_FIX32 pValue) {
  setImageResolution(ICAP_XRESOLUTION, pValue);
}

bool Application::GetImageYResolution(TW_FIX32 &value) {
  return GetCurrent(&ImageYResolution, value);
}

void Application::SetImageYResolution(const pTW_FIX32 pValue) {
  setImageResolution(ICAP_YRESOLUTION, pValue);
}

bool Application::GetTransferCount(TW_INT16 &value) {
  TW_UINT32 transferCount;
  bool current = GetCurrent(&TransferCount, transferCount);
  value = (TW_UINT16) transferCount;
  return current;
}

void Application::SetTransferCount(const TW_INT16 value) {
  SetCapabilityOneValue(CAP_XFERCOUNT, value, TWTY_INT16);

  if (GetCapability(TransferCount) != TWCC_SUCCESS) {
    return;
  }

  TW_INT16 transferCount;
  if (GetTransferCount(transferCount) && transferCount == value) {
    PrintMessage("Transfer Count successfully set!\n");
  }
}

TW_UINT16 Application::callDsmControl(
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData
) {
  return DsmEntry(
    mIdentity,
    nullptr,
    DG_CONTROL,
    dataArgumentType,
    message,
    pData);
}

bool Application::isDsmClosed() const {
  bool isClosed = CurrentDsmState < DsmState::Open;
  if (isClosed) {
    PrintMessage("The DSM has not been opened, open it first\n");
  }

  return isClosed;
}

bool Application::isDataSourceClosed() const {
  bool isClosed = CurrentDsmState < DsmState::DataSourceLoaded;
  if (isClosed) {
    PrintMessage("You need to open a data source first.\n");
  }

  return isClosed;
}

void Application::setImageResolution(
  const TW_UINT16 capabilityId,
  const pTW_FIX32 pValue
) {
  SetCapabilityOneValue(capabilityId, pValue);

  GetCapability(ImageXResolution);
  GetCapability(ImageYResolution);

  pTW_CAPABILITY pCapability = nullptr;
  if (capabilityId == ICAP_XRESOLUTION) {
    pCapability = &ImageXResolution;
  } else {
    pCapability = &ImageYResolution;
  }

  if (pCapability->ConType == TWON_ENUMERATION && pCapability->hContainer != nullptr) {
    auto pEnum = (pTWEnumerationFix32) pCapability->hContainer;
    auto currentItem = pEnum->ItemList[pEnum->CurrentIndex];

    if (
      pEnum->ItemType == TWTY_FIX32 &&
      currentItem.Whole == pValue->Whole &&
      currentItem.Frac == pValue->Frac
      ) {
      PrintMessage("Resolution successfully set!\n");
    }
  }
}

string Application::validSavePath(string savePath) {
  auto savePathLen = strlen(savePath.c_str());
  if (savePathLen) {
    if (savePath[savePathLen - 1] != kPATH_SEPARATOR) {
      savePath += kPATH_SEPARATOR;
    }
  }

  return savePath;
}
