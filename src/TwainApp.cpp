#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>

#include "FreeImage.h"
#include "TiffWriter.h"
#include "TwainApp.h"
#include "TwainString.h"

extern TW_ENTRYPOINT gDsmEntryPoint;

FAR PASCAL TW_UINT16 DSMCallback(
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

  cout << buffer;
}

/**
 * Output error messages for Free Image Format.
 * @param[in] format Free Image Format
 * @param[in] message error string to display
 */
void FreeImageErrorHandler(FREE_IMAGE_FORMAT format, const char *message) {
  PrintMessage(
    "\n*** %s Format\n%s ***\n", FreeImage_GetFormatFromFIF(format),
    message);
}

TW_IDENTITY AppIdentity() {
  TW_IDENTITY identity;

  identity.Id = nullptr;
  identity.Version.MajorNum = 2;
  identity.Version.MinorNum = 0;
  identity.Version.Language = TWLG_USA;
  identity.Version.Country = TWCY_USA;
  // FIXME: This is causing a segfault.
//  strcpy(reinterpret_cast<char *>(identity.Version.Info), "2.0.9");
  identity.ProtocolMajor = TWON_PROTOCOLMAJOR;
  identity.ProtocolMinor = TWON_PROTOCOLMINOR;
  identity.SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL;
  // TODO: Change these to Epson specifics.
  strcpy(reinterpret_cast<char *>(identity.Manufacturer), "Manufacturer");
  strcpy(reinterpret_cast<char *>(identity.ProductFamily), "Product Family");
  strcpy(reinterpret_cast<char *>(identity.ProductName), "Product Name");

  return identity;
}

TwainApp::TwainApp() :
  DataSourceMessage((TW_UINT16) -1),
  CurrentDsmState(DsmState::Unknown),
  TwainEvent(0),
  mGetHelpSupported(TWCC_SUCCESS),
  mGetLabelSupported(TWCC_SUCCESS),
  mIdentity(AppIdentity()),
  mpDataSource(nullptr),
  mpExtImageInfo(nullptr),
  mTransferCount(0) {
  FreeImage_Initialise();
  FreeImage_SetOutputMessage(FreeImageErrorHandler);
}

TwainApp::~TwainApp() {
  FreeImage_DeInitialise();
  UnloadDsmLibrary();
  mDataSources.erase(mDataSources.begin(), mDataSources.end());
}

TW_UINT16 TwainApp::CallDsm(
  TW_UINT16 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData
) {
  return DsmEntry(
    &mIdentity,
    mpDataSource,
    dataGroup,
    dataArgumentType,
    message,
    pData);
}

TW_INT16 TwainApp::PrintError(const string &errorMessage, pTW_IDENTITY pDestId) {
  TW_INT16 conditionCode = TWCC_SUCCESS;

  cerr << "TwainApp: ";
  if (errorMessage.length() > 0) {
    cerr << errorMessage;
  } else {
    cerr << "An error has occurred.";
  }

  if (GetConditionCode(pDestId, conditionCode) == TWRC_SUCCESS) {
    cerr << "The condition code is: " << ConditionCodeToString(conditionCode) << endl;
  }

  return conditionCode;
}

TW_UINT16 TwainApp::GetConditionCode(pTW_IDENTITY pDestId, TW_INT16 &conditionCode) {
  TW_STATUS status;
  memset(&status, 0, sizeof(TW_STATUS));

  TW_UINT16 returnCode = DsmEntry(
    &mIdentity,
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

void TwainApp::Exit() {
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

void TwainApp::ConnectDsm() {
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

  if ((mIdentity.SupportedGroups & DF_DSM2) == DF_DSM2) {
    gDsmEntryPoint.Size = sizeof(TW_ENTRYPOINT);

    callResult = callDsmControl(
      DAT_ENTRYPOINT,
      MSG_OPENDSM,
      (TW_MEMREF) &gDsmEntryPoint);
    if (callResult != TWRC_SUCCESS) {
      PrintMessage("DG_CONTROL / DAT_ENTRYPOINT / MSG_GET Failed: %d\n", callResult);
      return;
    }
  }

  PrintMessage("Successfully opened the DSM\n");
  CurrentDsmState = DsmState::Open;

  mDataSources.erase(mDataSources.begin(), mDataSources.end());
  GetDataSources();
}

void TwainApp::DisconnectDsm() {
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

TW_IDENTITY gSource;

pTW_IDENTITY TwainApp::GetDefaultDataSource() {
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

pTW_IDENTITY TwainApp::SetDefaultDataSource(unsigned int index) {
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

pTW_IDENTITY TwainApp::SelectDefaultDataSource() {
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

void TwainApp::LoadDataSource(const TW_INT32 id) {
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

void TwainApp::UnloadDataSource() {
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

pTW_IDENTITY TwainApp::GetDataSource(TW_INT16 index) const {
  if (index < 0) {
    return mpDataSource;
  }

  if ((unsigned int) index < mDataSources.size()) {
    return (pTW_IDENTITY) &mDataSources[index];
  }

  return nullptr;
}

void TwainApp::GetDataSources() {
  if (isDsmClosed()) {
    return;
  }

  assert(mDataSources.empty() == true);

  auto pushDataSource = [this](TW_UINT16 getMessage) {
    TW_IDENTITY dataSource;
    memset(&dataSource, 0, sizeof(TW_IDENTITY));

    TW_UINT16 returnCode = callDsmControl(
      DAT_IDENTITY,
      getMessage,
      (TW_MEMREF) &dataSource);
    if (returnCode == TWRC_SUCCESS) {
      mDataSources.push_back(dataSource);
    } else if (returnCode == TWRC_FAILURE) {
      PrintError("Failed to get the data source info!");
    }

    return returnCode;
  };

  TW_UINT16 returnCode = pushDataSource(MSG_GETFIRST);
  if (returnCode == TWRC_ENDOFLIST) {
    return;
  }

  do {
    returnCode = pushDataSource(MSG_GETNEXT);
  } while (returnCode == TWRC_SUCCESS);
}

bool TwainApp::EnableDataSource() {
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

void TwainApp::DisableDataSource() {
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

bool TwainApp::UpdateImageInfo() {
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

void TwainApp::InitiateNativeTransfer() {
  PrintMessage("Starting a native transfer...\n");

  TW_STR255 outFileName;
  bool transfersPending = true;
  TW_UINT16 returnCode = TWRC_SUCCESS;
  string savePath = validSavePath(mSavePath);

  while (transfersPending) {
    mTransferCount++;
    memset(outFileName, 0, sizeof(outFileName));

    if (!UpdateImageInfo()) {
      break;
    }

    TW_MEMREF imageHandle = nullptr;
    PrintMessage("Starting the transfer...\n");

    returnCode = CallDsm(
      DG_IMAGE,
      DAT_IMAGENATIVEXFER,
      MSG_GET,
      (TW_MEMREF) &imageHandle);
    switch (returnCode) {
      case TWRC_XFERDONE: {
        /*
       * Here we get a handle to a DIB. Save it to disk as a BMP. After saving
       * it to disk, I could open it up again using FreeImage if I wanted to do
       * more transforms on it or save it as a different format.
       */
        auto pDIB = (PBITMAPINFOHEADER) LockDsmMemory((TW_HANDLE) imageHandle);

        if (pDIB == nullptr) {
          PrintError("Unable to lock memory, transfer failed", mpDataSource);
          break;
        }

        Printf(
          reinterpret_cast<char *>(outFileName),
          sizeof(outFileName),
          "%sFROM_SCANNER_%06dN.bmp",
          savePath.c_str(),
          mTransferCount);

        auto outputFile = fopen(reinterpret_cast<const char *>(outFileName), "wb");
        if (outputFile == 0) {
          perror("fopen");
        } else {
          DWORD paletteSize = 0;

          switch (pDIB->biBitCount) {
            case 1:
              paletteSize = 2;
              break;
            case 8:
              paletteSize = 256;
              break;

            case 24:
              break;

            default:
              assert(0); // Not going to work!
          }

          // If the driver did not fill in the biSizeImage field, then compute it
          // Each scan line of the image is aligned on a DWORD (32bit) boundary:
          if (pDIB->biSizeImage == 0) {
            auto widthBitCount = pDIB->biWidth * pDIB->biBitCount;
            pDIB->biSizeImage = ((widthBitCount + 31 & ~31) / 8) * pDIB->biHeight;

            // If a compression scheme is used the result may in fact be larger
            // Increase the size to account for this:
            if (pDIB->biCompression != 0) {
              pDIB->biSizeImage = (pDIB->biSizeImage * 3) / 2;
            }
          }

          auto rgbPaletteSize = sizeof(RGBQUAD) * paletteSize;
          auto infoHeaderSize = sizeof(BITMAPINFOHEADER);
          int imageSize = pDIB->biSizeImage + rgbPaletteSize + infoHeaderSize;

          BitmapFileHeader bmpFileHeader = {0};
          auto fileHeaderSize = sizeof(BitmapFileHeader);
          bmpFileHeader.FileType = ((WORD) ('M' << 8) | 'B');
          bmpFileHeader.FileSize = imageSize + fileHeaderSize;
          bmpFileHeader.OffsetBits = fileHeaderSize + infoHeaderSize + rgbPaletteSize;

          fwrite(&bmpFileHeader, 1, sizeof(BitmapFileHeader), outputFile);
          fwrite(pDIB, 1, imageSize, outputFile);
          fclose(outputFile);

          PrintMessage("File \"%s\" saved...\n", outFileName);
        }

        UnlockDsmMemory((TW_HANDLE) imageHandle);
        FreeDsmMemory((TW_HANDLE) imageHandle);
        pDIB = nullptr;

        UpdateExtImageInfo();

        PrintMessage("Checking to see if there are more images to transfer...\n");
        TW_PENDINGXFERS pendingTransfers;
        memset(&pendingTransfers, 0, sizeof(pendingTransfers));

        TW_UINT16 checkReturnCode = CallDsm(
          DG_CONTROL,
          DAT_PENDINGXFERS,
          MSG_ENDXFER,
          (TW_MEMREF) &pendingTransfers);
        if (checkReturnCode == TWRC_SUCCESS) {
          PrintMessage("Remaining images to transfer: %u\n", pendingTransfers.Count);

          if (pendingTransfers.Count == 0) {
            transfersPending = false;
          }
        } else {
          PrintError("Failed to properly end the transfer", mpDataSource);
          transfersPending = false;
        }
      }
        break;

      case TWRC_CANCEL:
        PrintError("Canceled transfer image", mpDataSource);
        break;

      case TWRC_FAILURE:
        PrintError("Failed to transfer image", mpDataSource);
        break;

      default:
        break;

    }
  }

  if (transfersPending) {
    DoAbortTransfer();
  }

  CurrentDsmState = DsmState::DataSourceEnabled;
  PrintMessage("Done!\n");
}

void TwainApp::InitiateFileTransfer(TW_UINT16 fileFormat) {
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

        UpdateExtImageInfo();

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

void TwainApp::InitiateMemoryTransfer() {
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

    auto memoryHandle = (TW_HANDLE) AllocDsmMemory(sourcesBufferSizes.Preferred);
    if (memoryHandle == nullptr) {
      PrintError("Error allocating memory");
      break;
    }

    imageMemTransfer.Memory.TheMem = (TW_MEMREF) LockDsmMemory(memoryHandle);

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
          UpdateExtImageInfo();
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

    // Cleanup memory used to transfer image:
    UnlockDsmMemory(memoryHandle);
    FreeDsmMemory(memoryHandle);

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

TW_UINT16 TwainApp::DoAbortTransfer() {
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

void TwainApp::UpdateExtImageInfo() {
  int tableBarCodeExtImgInfo[] = {
    TWEI_BARCODETYPE,
    TWEI_BARCODETEXTLENGTH,
    TWEI_BARCODETEXT,
    TWEI_BARCODEX,
    TWEI_BARCODEY,
    TWEI_BARCODEROTATION,
    TWEI_BARCODECONFIDENCE
  };

  int tableOtherExtImgInfo[] = {
    TWEI_BOOKNAME,
    TWEI_CHAPTERNUMBER,
    TWEI_DOCUMENTNUMBER,
    TWEI_PAGENUMBER,
    TWEI_PAGESIDE,
    TWEI_CAMERA,
    TWEI_FRAMENUMBER,
    TWEI_FRAME,
    TWEI_PIXELFLAVOR,
    TWEI_ENDORSEDTEXT,
    TWEI_MAGTYPE,
    TWEI_MAGDATA
  };

  int barCodeInfosCount = sizeof(tableBarCodeExtImgInfo) / sizeof(tableBarCodeExtImgInfo[0]);
  int otherInfosCount = sizeof(tableOtherExtImgInfo) / sizeof(tableOtherExtImgInfo[0]);

  TW_UINT16 returnCode = TWRC_SUCCESS;
  mExtImageInfoString = "";

  try {
    TW_EXTIMAGEINFO extImageInfo;
    extImageInfo.NumInfos = 1;
    extImageInfo.Info[0].InfoID = TWEI_BARCODECOUNT;
    extImageInfo.Info[0].NumItems = 0;
    extImageInfo.Info[0].ItemType = TWTY_UINT32;
    extImageInfo.Info[0].Item = 0;
    extImageInfo.Info[0].ReturnCode = 0;

    returnCode = CallDsm(
      DG_IMAGE,
      DAT_EXTIMAGEINFO,
      MSG_GET,
      (TW_MEMREF) &extImageInfo);
    if (returnCode != TWRC_SUCCESS) {
      mExtImageInfoString = "Not Supported";
      return;
    }

    int currentInfo = 0;
    int infoCount = 0;

    if (extImageInfo.Info[0].ReturnCode == TWRC_SUCCESS) {
      auto firstItem = (TW_UINT32) extImageInfo.Info[0].Item;
      infoCount = (barCodeInfosCount * firstItem) + 1;
    }

    TW_CAPABILITY supportedExtImageInfo;
    pTWArrayUInt16 pSupportedExtImageInfos = nullptr;

    supportedExtImageInfo.Cap = ICAP_SUPPORTEDEXTIMAGEINFO;
    supportedExtImageInfo.hContainer = nullptr;

    GetCapability(supportedExtImageInfo);

    if (supportedExtImageInfo.ConType == TWON_ARRAY) {
      pSupportedExtImageInfos = (pTWArrayUInt16) LockDsmMemory(supportedExtImageInfo.hContainer);

      if (pSupportedExtImageInfos->ItemType != TWTY_UINT16) {
        UnlockDsmMemory(supportedExtImageInfo.hContainer);
        pSupportedExtImageInfos = nullptr;
      } else {
        int extInfoCount = pSupportedExtImageInfos->NumItems;
        bool wasAdded;

        for (int index = 0; index < extInfoCount; index++) {
          if (pSupportedExtImageInfos->ItemList[index] == TWEI_BARCODECOUNT) {
            continue;
          }

          wasAdded = true;
          for (int barcodeIndex = 0; barcodeIndex < barCodeInfosCount; barcodeIndex++) {
            if (pSupportedExtImageInfos->ItemList[barcodeIndex] ==
                tableBarCodeExtImgInfo[barcodeIndex]) {
              wasAdded = false;
              break;
            }
          }

          if (wasAdded) {
            extInfoCount++;
          }
        }
      }
    }

    if (!pSupportedExtImageInfos) {
      infoCount = otherInfosCount;
    }

    auto allocSize = sizeof(TW_EXTIMAGEINFO) + sizeof(TW_INFO) * (infoCount - 1);
    TW_HANDLE extInfoHandle = AllocDsmMemory(allocSize);
    auto *pExtImageInfo = (TW_EXTIMAGEINFO *) LockDsmMemory(extInfoHandle);
    memset(pExtImageInfo, 0, allocSize);
    pExtImageInfo->NumInfos = infoCount;

    if (pSupportedExtImageInfos) {
      int itemCount = pSupportedExtImageInfos->NumItems;
      bool isAdded;

      for (int index = 0; index < itemCount; index++) {
        if (pSupportedExtImageInfos->ItemList[index] == TWEI_BARCODECOUNT) {
          continue;
        }

        isAdded = true;
        for (int barcodeIndex = 0; barcodeIndex < barCodeInfosCount; barcodeIndex++) {
          if (pSupportedExtImageInfos->ItemList[index] == tableBarCodeExtImgInfo[barcodeIndex]) {
            isAdded = false;
            break;
          }
        }

        if (isAdded) {
          pExtImageInfo->Info[currentInfo++].InfoID = pSupportedExtImageInfos->ItemList[index];
        }
      }
    } else {
      for (int itemIndex = 0; itemIndex < otherInfosCount; itemIndex++) {
        pExtImageInfo->Info[currentInfo++].InfoID = (TW_UINT16) tableOtherExtImgInfo[itemIndex];
      }
    }

    if (extImageInfo.Info[0].ReturnCode == TWRC_SUCCESS) {
      // Inform the DS how many Barcode items we can handle for each type:
      pExtImageInfo->Info[currentInfo++] = extImageInfo.Info[0];

      auto firstInfoCount = (unsigned int) extImageInfo.Info[0].Item;
      for (unsigned int count = 0; count < firstInfoCount; count++) {
        for (int barcodeIndex = 0; barcodeIndex < barCodeInfosCount; barcodeIndex++) {
          pExtImageInfo->Info[currentInfo++].InfoID = (TW_UINT16) tableBarCodeExtImgInfo[barcodeIndex];
        }
      }
    }

    returnCode = CallDsm(
      DG_IMAGE,
      DAT_EXTIMAGEINFO,
      MSG_GET,
      (TW_MEMREF) pExtImageInfo);
    if (returnCode != TWRC_SUCCESS) {
      mExtImageInfoString = "Not Supported";
      return;
    }

    for (int index = 0; index < infoCount; index++) {
      if (pExtImageInfo->Info[index].ReturnCode != TWRC_INFONOTSUPPORTED) {
        mExtImageInfoString = "";
        // TODO: Finish this (line 1206).
      }
    }
  } catch (const exception &e) {
    cerr << "Error getting extended image info";
    cerr << e.what();
  }
}

TW_UINT16 TwainApp::SetCapabilityOneValue(
  TW_UINT16 id,
  const int value,
  TW_UINT16 twainType
) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = AllocDsmMemory(sizeof(TW_ONEVALUE));

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTW_ONEVALUE) LockDsmMemory(capability.hContainer);
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

  UnlockDsmMemory(capability.hContainer);
  FreeDsmMemory(capability.hContainer);

  return returnCode;
}

TW_UINT16 TwainApp::SetCapabilityOneValue(TW_UINT16 id, const pTW_FIX32 pValue) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = AllocDsmMemory(sizeof(TWOneValueFix32));

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTWOneValueFix32) LockDsmMemory(capability.hContainer);
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

  UnlockDsmMemory(capability.hContainer);
  FreeDsmMemory(capability.hContainer);

  return returnCode;
}

TW_UINT16 TwainApp::SetCapabilityOneValue(TW_UINT16 id, const pTW_FRAME pValue) {
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ONEVALUE;
  capability.hContainer = AllocDsmMemory(sizeof(TWOneValueFrame));

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pOneValue = (pTWOneValueFrame) LockDsmMemory(capability.hContainer);
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

  UnlockDsmMemory(capability.hContainer);
  FreeDsmMemory(capability.hContainer);

  return returnCode;
}

TW_UINT16 TwainApp::SetCapabilityArray(
  TW_UINT16 id,
  const int *pValues,
  int count,
  TW_UINT16 twainType
) {
  auto allocSize = sizeof(TW_ARRAY) + GetTwainTypeSize(twainType) * count;
  TW_CAPABILITY capability;
  capability.Cap = id;
  capability.ConType = TWON_ARRAY;
  capability.hContainer = AllocDsmMemory(allocSize);

  if (capability.hContainer == nullptr) {
    PrintError("Error allocating memory");
    return TWRC_FAILURE;
  }

  auto pArray = (pTW_ARRAY) LockDsmMemory(capability.hContainer);
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

  UnlockDsmMemory(capability.hContainer);
  FreeDsmMemory(capability.hContainer);

  return returnCode;
}

TW_INT16 TwainApp::GetCapability(TW_CAPABILITY &capability, TW_UINT16 message) {
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
    FreeDsmMemory(capability.hContainer);
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

TW_INT16 TwainApp::QuerySupportCapability(
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
      auto pOneValue = (pTW_ONEVALUE) LockDsmMemory(capability.hContainer);
      querySupport = pOneValue->ItemType;
      UnlockDsmMemory(capability.hContainer);
    }

    FreeDsmMemory(capability.hContainer);
  } else {
    string errorMessage = "Failed to query support the capability: [";
    errorMessage += CapabilityToString(id);
    errorMessage += "]";

    PrintError(errorMessage, mpDataSource);
  }

  return returnCode;
}

TW_INT16 TwainApp::GetLabel(TW_UINT16 capabilityId, string &label) {
  if (mGetLabelSupported == TWCC_BADPROTOCOL) {
    return TWRC_FAILURE;
  }

  if (isDataSourceClosed()) {
    return TWCC_SEQERROR;
  }

  TW_CAPABILITY capability = {0};
  capability.Cap = capabilityId;
  capability.hContainer = nullptr;
  capability.ConType = TWON_ONEVALUE;

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_GETLABEL,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_SUCCESS) {
    if (capability.ConType == TWON_ONEVALUE) {
      auto pOneValue = (pTW_ONEVALUE) LockDsmMemory(capability.hContainer);
      TW_UINT16 type = pOneValue->ItemType;
      UnlockDsmMemory(capability.hContainer);

      switch (type) {
        case TWTY_STR32:
        case TWTY_STR64:
        case TWTY_STR128:
          PrintError("Wrong STR type for MSG_GETLABEL", mpDataSource);

        case TWTY_STR255:
          GetCurrent(&capability, label);
          break;

        default:
          returnCode = TWRC_FAILURE;
          break;
      }
    }

    FreeDsmMemory(capability.hContainer);
  } else {
    string errorMessage = "Failed to GetLabel for the capability: [";
    errorMessage += CapabilityToString(capabilityId);
    errorMessage += "]";

    auto errorCode = PrintError(errorMessage, mpDataSource);
    if (errorCode == TWCC_BADPROTOCOL) {
      mGetLabelSupported = TWCC_BADPROTOCOL;
    }
  }

  return returnCode;
}

TW_INT16 TwainApp::GetHelp(TW_UINT16 capabilityId, string &help) {
  if (mGetHelpSupported == TWCC_BADPROTOCOL) {
    return TWRC_FAILURE;
  }

  if (isDataSourceClosed()) {
    return TWCC_SEQERROR;
  }

  TW_CAPABILITY capability = {0};
  capability.Cap = capabilityId;
  capability.hContainer = nullptr;
  capability.ConType = TWON_ONEVALUE;

  TW_UINT16 returnCode = CallDsm(
    DG_CONTROL,
    DAT_CAPABILITY,
    MSG_GETHELP,
    (TW_MEMREF) &capability);
  if (returnCode == TWRC_SUCCESS) {
    if (capability.ConType == TWON_ONEVALUE) {
      auto pOneValue = (pTW_ONEVALUE) LockDsmMemory(capability.hContainer);
      TW_UINT16 type = pOneValue->ItemType;
      UnlockDsmMemory(capability.hContainer);

      switch (type) {
        case TWTY_STR32:
        case TWTY_STR64:
        case TWTY_STR128:
          PrintError("Wrong STR type for MSG_GETHELP", mpDataSource);

        case TWTY_STR255:
          // TODO: Find out if this breaks anything. The reference code didn't make sense.
          GetCurrent(&capability, help);
          break;

        default:
          returnCode = TWRC_FAILURE;
          break;
      }
    }

    FreeDsmMemory(capability.hContainer);
  } else {
    string errorMessage = "Failed to GetHelp for the capability: [";
    errorMessage += CapabilityToString(capabilityId);
    errorMessage += "]";

    auto errorCode = PrintError(errorMessage, mpDataSource);
    if (errorCode == TWCC_BADPROTOCOL) {
      mGetLabelSupported = TWCC_BADPROTOCOL;
    }
  }

  return returnCode;
}

void TwainApp::PrintAvailableDataSources() {
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

void TwainApp::PrintIdentityStruct(const TW_IDENTITY &identity) {
  PrintMessage("\n Id: %u\n", identity.Id);
  PrintMessage("Version: %u.%u\n", identity.Version.MajorNum, identity.Version.MinorNum);
  PrintMessage("SupportedGroups: %u\n", identity.SupportedGroups);
  PrintMessage("Manufacturer: %s\n", identity.Manufacturer);
  PrintMessage("ProductFamily: %s\n", identity.ProductFamily);
  PrintMessage("ProductName: %s\n", identity.ProductName);
}

void TwainApp::PrintMatchingIdentityStruct(const TW_UINT32 identityId) {
  for (auto &dataSource : mDataSources) {
    if (*dataSource.Id == identityId) {
      PrintIdentityStruct(dataSource);
      break;
    }
  }
}

void TwainApp::InitiateCapabilities() {
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

void TwainApp::TerminateCapabilities() {
  if (isDsmClosed()) {
    return;
  }

  if (isDataSourceClosed()) {
    return;
  }

  if (ImageBitDepth.hContainer) {
    FreeDsmMemory(ImageBitDepth.hContainer);
    ImageBitDepth.hContainer = nullptr;
  }

  if (ImageCompression.hContainer) {
    FreeDsmMemory(ImageCompression.hContainer);
    ImageCompression.hContainer = nullptr;
  }

  if (ImageFrames.hContainer) {
    FreeDsmMemory(ImageFrames.hContainer);
    ImageFrames.hContainer = nullptr;
  }

  if (ImageFileFormat.hContainer) {
    FreeDsmMemory(ImageFileFormat.hContainer);
    ImageFileFormat.hContainer = nullptr;
  }

  if (ImagePixelType.hContainer) {
    FreeDsmMemory(ImagePixelType.hContainer);
    ImagePixelType.hContainer = nullptr;
  }

  if (ImageTransferMechanism.hContainer) {
    FreeDsmMemory(ImageTransferMechanism.hContainer);
    ImageTransferMechanism.hContainer = nullptr;
  }

  if (ImageUnits.hContainer) {
    FreeDsmMemory(ImageUnits.hContainer);
    ImageUnits.hContainer = nullptr;
  }

  if (ImageXResolution.hContainer) {
    FreeDsmMemory(ImageXResolution.hContainer);
    ImageXResolution.hContainer = nullptr;
  }

  if (ImageYResolution.hContainer) {
    FreeDsmMemory(ImageYResolution.hContainer);
    ImageYResolution.hContainer = nullptr;
  }

  if (TransferCount.hContainer) {
    FreeDsmMemory(TransferCount.hContainer);
    TransferCount.hContainer = nullptr;
  }
}

void TwainApp::StartScan() {
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
      InitiateNativeTransfer();
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

bool TwainApp::GetImageBitDepth(TW_UINT16 &value) {
  TW_UINT32 bitDepth;
  bool current = GetCurrent(&ImageBitDepth, bitDepth);
  value = (TW_UINT16) bitDepth;
  return current;
}

void TwainApp::SetImageBitDepth(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_BITDEPTH, value, TWTY_UINT16);

  if (GetCapability(ImageBitDepth) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 bitDepth;
  if (GetImageBitDepth(bitDepth) && bitDepth == value) {
    PrintMessage("Image Bit Depth successfully set!\n");
  }
}

bool TwainApp::GetImageCompression(TW_UINT16 &value) {
  TW_UINT32 compression;
  bool current = GetCurrent(&ImageCompression, compression);
  value = (TW_UINT16) compression;
  return current;
}

void TwainApp::SetImageCompression(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_COMPRESSION, value, TWTY_UINT16);

  if (GetCapability(ImageCompression) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 compression;
  if (GetImageCompression(compression) && compression == value) {
    PrintMessage("Image Compression successfully set!\n");
  }
}

bool TwainApp::GetImageFileFormat(TW_UINT16 &value) {
  TW_UINT32 fileFormat;
  bool current = GetCurrent(&ImageFileFormat, fileFormat);
  value = (TW_UINT16) fileFormat;
  return current;
}

void TwainApp::SetImageFileFormat(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_IMAGEFILEFORMAT, value, TWTY_UINT16);

  if (GetCapability(ImageFileFormat) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 fileFormat;
  if (GetImageFileFormat(fileFormat) && fileFormat == value) {
    PrintMessage("Image File Format successfully set!\n");
  }
}

bool TwainApp::GetImageFrames(TW_FRAME &value) {
  return GetCurrent(&ImageFrames, value);
}

void TwainApp::SetImageFrames(const pTW_FRAME pValue) {
  SetCapabilityOneValue(ICAP_FRAMES, pValue);

  if (GetCapability(ImageFrames) != TWCC_SUCCESS) {
    return;
  }

  if (ImageFrames.ConType == TWON_ENUMERATION && ImageFrames.hContainer != nullptr) {
    auto pEnumFrame = (pTWEnumerationFrame) LockDsmMemory(ImageFrames.hContainer);
    auto pFrame = &pEnumFrame->ItemList[pEnumFrame->CurrentIndex];

    if (
      pFrame->Bottom == pValue->Bottom &&
      pFrame->Top == pValue->Top &&
      pFrame->Left == pValue->Left &&
      pFrame->Right == pValue->Right
      ) {
      PrintMessage("Image Frames successfully set!\n");
    }

    UnlockDsmMemory(ImageFrames.hContainer);
  }
}

bool TwainApp::GetImagePixelType(TW_INT16 &value) {
  TW_UINT32 pixelType;
  bool current = GetCurrent(&ImagePixelType, pixelType);
  value = (TW_UINT16) pixelType;
  return current;
}

void TwainApp::SetImagePixelType(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_PIXELTYPE, value, TWTY_UINT16);

  if (GetCapability(ImagePixelType) != TWCC_SUCCESS) {
    return;
  }

  if (ImagePixelType.ConType == TWON_ENUMERATION && ImagePixelType.hContainer != nullptr) {
    auto pEnum = (pTW_ENUMERATION) LockDsmMemory(ImagePixelType.hContainer);
    auto currentItem = ((TW_UINT16 *) &pEnum->ItemList)[pEnum->CurrentIndex];

    if (currentItem == value) {
      PrintMessage("Image Pixel Type successfully set!\n");
    }

    UnlockDsmMemory(ImagePixelType.hContainer);
  }

  GetCapability(ImageBitDepth);
}

bool TwainApp::GetImageTransferMechanism(TW_UINT16 &value) {
  TW_UINT32 mechanism;
  bool current = GetCurrent(&ImageTransferMechanism, mechanism);
  value = (TW_UINT16) mechanism;
  return current;
}

void TwainApp::SetImageTransferMechanism(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_XFERMECH, value, TWTY_UINT16);

  if (GetCapability(ImageTransferMechanism) != TWCC_SUCCESS) {
    return;
  }

  TW_UINT16 mechanism;
  if (GetImageTransferMechanism(mechanism) && mechanism == value) {
    PrintMessage("Image Transfer Mechanism successfully set!\n");
  }
}

bool TwainApp::GetImageUnits(TW_INT16 &value) {
  TW_UINT32 units;
  bool current = GetCurrent(&ImageUnits, units);
  value = (TW_UINT16) units;
  return current;
}

void TwainApp::SetImageUnits(const TW_UINT16 value) {
  SetCapabilityOneValue(ICAP_UNITS, value, TWTY_UINT16);

  if (GetCapability(ImageUnits) != TWCC_SUCCESS) {
    return;
  }

  if (ImageUnits.ConType == TWON_ENUMERATION && ImageUnits.hContainer != nullptr) {
    auto pEnum = (pTW_ENUMERATION) LockDsmMemory(ImageUnits.hContainer);

    if (pEnum->ItemList[pEnum->CurrentIndex] == value) {
      PrintMessage("Image Units successfully set!\n");

      GetCapability(ImageXResolution);
      GetCapability(ImageYResolution);
    }

    UnlockDsmMemory(ImageUnits.hContainer);
  }
}

bool TwainApp::GetImageXResolution(TW_FIX32 &value) {
  return GetCurrent(&ImageXResolution, value);
}

void TwainApp::SetImageXResolution(const pTW_FIX32 pValue) {
  setImageResolution(ICAP_XRESOLUTION, pValue);
}

bool TwainApp::GetImageYResolution(TW_FIX32 &value) {
  return GetCurrent(&ImageYResolution, value);
}

void TwainApp::SetImageYResolution(const pTW_FIX32 pValue) {
  setImageResolution(ICAP_YRESOLUTION, pValue);
}

bool TwainApp::GetTransferCount(TW_INT16 &value) {
  TW_UINT32 transferCount;
  bool current = GetCurrent(&TransferCount, transferCount);
  value = (TW_UINT16) transferCount;
  return current;
}

void TwainApp::SetTransferCount(const TW_INT16 value) {
  SetCapabilityOneValue(CAP_XFERCOUNT, value, TWTY_INT16);

  if (GetCapability(TransferCount) != TWCC_SUCCESS) {
    return;
  }

  TW_INT16 transferCount;
  if (GetTransferCount(transferCount) && transferCount == value) {
    PrintMessage("Transfer Count successfully set!\n");
  }
}

TW_UINT16 TwainApp::callDsmControl(
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData
) {
  return DsmEntry(
    &mIdentity,
    nullptr,
    DG_CONTROL,
    dataArgumentType,
    message,
    pData);
}

bool TwainApp::isDsmClosed() const {
  bool isClosed = CurrentDsmState < DsmState::Open;
  if (isClosed) {
    PrintMessage("The DSM has not been opened, open it first\n");
  }

  return isClosed;
}

bool TwainApp::isDataSourceClosed() const {
  bool isClosed = CurrentDsmState < DsmState::DataSourceLoaded;
  if (isClosed) {
    PrintMessage("You need to open a data source first.\n");
  }

  return isClosed;
}

void TwainApp::setImageResolution(
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

string TwainApp::validSavePath(string savePath) {
  auto savePathLen = strlen(savePath.c_str());
  if (savePathLen) {
    if (savePath[savePathLen - 1] != kPATH_SEPARATOR) {
      savePath += kPATH_SEPARATOR;
    }
  }

  return savePath;
}
