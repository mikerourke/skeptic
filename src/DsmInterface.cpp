#include <iostream>
#include <CoreFoundation/CoreFoundation.h>

#include "DsmInterface.h"

using namespace std;

CFBundleRef gpDsmLibrary = nullptr;

DSMENTRYPROC gpDsmEntryProc = nullptr;

TW_UINT16 DsmEntry(
  pTW_IDENTITY pOrigin,
  pTW_IDENTITY pDest,
  TW_UINT32 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData) {
  TW_UINT16 returnCode = TWRC_FAILURE;

  if (!LoadDsmLibrary()) {
    cerr << "Could not load the DSM library" << endl;
    return 0;
  }

  returnCode = gpDsmEntryProc(
    pOrigin,
    pDest,
    dataGroup,
    dataArgumentType,
    message,
    pData);

  return returnCode;
}

bool LoadDsmLibrary() {
  if (gpDsmLibrary != nullptr) {
    return true;
  }

  CFStringRef cfLibraryPath = CFStringCreateWithCString(
    nullptr,
    kDSM_LIBRARY_PATH,
    kCFStringEncodingUTF8);

//  cout << "Got String Ref!" << std::endl;

  CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(
    kCFAllocatorDefault,
    cfLibraryPath,
    kCFURLPOSIXPathStyle,
    true);
//  cout << "Got Bundle URL!" << std::endl;

  CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
  if (bundle == nullptr) {
    cerr << "Bundle not loaded!" << endl;
  } else {
//    cout << "Got bundle!" << endl;
  }

  gpDsmLibrary = bundle;

  CFRelease(CFTypeRef(bundleUrl));
  CFRelease(CFTypeRef(cfLibraryPath));

  CFStringRef cfFuncName = CFStringCreateWithCString(
    nullptr,
    "DSM_Entry",
    kCFStringEncodingUTF8);

  auto fp = CFBundleGetFunctionPointerForName(bundle, cfFuncName);
  if (!fp) {
    cerr << "Function not found!" << endl;
  } else {
//    cout << "Got function!" << endl;
  }

  gpDsmEntryProc = (DSMENTRYPROC)fp;

  CFRelease(CFTypeRef(cfFuncName));

  return true;
}

void UnloadDsmLibrary() {
  if (gpDsmLibrary != nullptr) {
    CFRelease(CFTypeRef(CFBundleRef(gpDsmLibrary)));
    gpDsmLibrary = nullptr;
    gpDsmEntryProc = nullptr;

    std::cout << "Released!" << std::endl;
  }
}
