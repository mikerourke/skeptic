#include "DsmInterface.h"
#include <iostream>
#include <CoreFoundation/CoreFoundation.h>

using namespace std;

CFBundleRef gpDsmLibrary = nullptr;

DSMENTRYPROC gpDsmEntryProc = nullptr;

TW_ENTRYPOINT gDsmEntryPoint = {0};

TW_UINT16 DsmEntry(
  pTW_IDENTITY pOrigin,
  pTW_IDENTITY pDest,
  TW_UINT32 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData) {
  TW_UINT16 ret = TWRC_FAILURE;

  if (gpDsmLibrary == nullptr) {
    if (!LoadDsmLibrary()) {
      cerr << "Could not load the DSM library" << endl;
      return 0;
    }
  }

  if (gpDsmEntryProc != nullptr) {
    ret = gpDsmEntryProc(pOrigin, pDest, dataGroup, dataArgumentType, message, pData);
  }

  return ret;
}

bool LoadDsmLibrary() {
  if (gpDsmLibrary != nullptr) {
    return true;
  }

  CFStringRef cfLibraryPath = CFStringCreateWithCString(
    nullptr,
    kDSM_LIBRARY_PATH,
    kCFStringEncodingUTF8);

  cout << "Got String Ref!" << std::endl;

  CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(
    kCFAllocatorDefault,
    cfLibraryPath,
    kCFURLPOSIXPathStyle,
    true);
  cout << "Got Bundle URL!" << std::endl;

  CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
  if (bundle == nullptr) {
    cerr << "Bundle not loaded!" << endl;
  } else {
    cout << "Got bundle!" << bundle << endl;
  }

  gpDsmLibrary = bundle;

  CFRelease(CFTypeRef(bundleUrl));
  CFRelease(CFTypeRef(cfLibraryPath));

  CFStringRef cfFuncName = CFStringCreateWithCString(
    nullptr,
    "DSM_Entry",
    kCFStringEncodingUTF8);

  void *fp = CFBundleGetFunctionPointerForName(bundle, cfFuncName);
  if (!fp) {
    cerr << "Function not found!" << endl;
  } else {
    cout << "Got function!" << fp << endl;
  }

  gpDsmEntryProc = (DSMENTRYPROC) fp;

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

TW_HANDLE AllocDsmMemory(TW_UINT32 memorySize) {
  if (gDsmEntryPoint.DSM_MemAllocate) {
    return gDsmEntryPoint.DSM_MemAllocate(memorySize);
  }

  return nullptr;
}

void FreeDsmMemory(TW_HANDLE memoryHandle) {
  if (gDsmEntryPoint.DSM_MemFree) {
    gDsmEntryPoint.DSM_MemFree(memoryHandle);
  }
}

TW_MEMREF LockDsmMemory(TW_HANDLE memoryHandle) {
  if (gDsmEntryPoint.DSM_MemLock) {
    return gDsmEntryPoint.DSM_MemLock(memoryHandle);
  }

  return nullptr;
}

void UnlockDsmMemory(TW_HANDLE memoryHandle) {
  if (gDsmEntryPoint.DSM_MemUnlock) {
    gDsmEntryPoint.DSM_MemUnlock(memoryHandle);
  }
}

