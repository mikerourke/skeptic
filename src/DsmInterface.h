#ifndef TWAINFUL_DSMINTERFACE_H
#define TWAINFUL_DSMINTERFACE_H

#pragma once

#include "twain.h"

#define kDSM_LIBRARY_PATH "/Library/Frameworks/TWAINDSM.framework"

enum class DsmState {
  Unknown = 1,
  Closed = 2,
  Open = 3,
  DataSourceLoaded = 4,
  DataSourceEnabled = 5,
  ReadyToScan = 6,
};

/**
* Load the DSM library.
* @return true if success.
*/
bool LoadDsmLibrary();

/** Unload the DSM library. */
void UnloadDsmLibrary();

/**
* Initialize and register the entry point into the DSM.
* @param[in] pOrigin Identifies the source module of the message. This could
*           identify an Application, a Source, or the Source Manager.
*
* @param[in] pDest Identifies the destination module for the message.
*           This could identify an application or a data source.
*           If this is NULL, the message goes to the Source Manager.
*
* @param[in] dataGroup The Data Group.
*           Example: DG_IMAGE.
*
* @param[in] dataArgumentType The Data Attribute Type.
*           Example: DAT_IMAGEMEMXFER.
*
* @param[in] message The message. Messages are interpreted by the destination module
*           with respect to the Data Group and the Data Attribute Type.
*           Example: MSG_GET.
*
* @param[in,out] pData A pointer to the data structure or variable identified
*           by the Data Attribute Type.
*           Example: (TW_MEMREF)&ImageMemXfer
*                   where ImageMemXfer is a TW_IMAGEMEMXFER structure.
*
* @return a valid TWRC_xxxx return code.
*          Example: TWRC_SUCCESS.
*/
TW_UINT16 DsmEntry(
  pTW_IDENTITY pOrigin,
  pTW_IDENTITY pDest,
  TW_UINT32 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData);

/**
* Allocate global memory
* @param[in] memorySize Size of memory to allocate.
* @return TW_HANDLE to the memory allocated.
*/
TW_HANDLE AllocDsmMemory(TW_UINT32 memorySize);

/**
* Free previously allocated global memory
* @param[in] memoryHandle TW_HANDLE to the memory needing free.
*/
void FreeDsmMemory(TW_HANDLE memoryHandle);

/**
* Lock global memory from being updated by others. return a pointer to the
* memory so we can update it.
* @param[in] memoryHandle TW_HANDLE to the memory to update.
* @return TW_MEMREF pointer to the memory.
*/
TW_MEMREF LockDsmMemory(TW_HANDLE memoryHandle);

/**
* Unlock global memory after locking. to allow updating by others.
* @param[in] memoryHandle TW_HANDLE to memory returned by AllocDsmMemory
*/
void UnlockDsmMemory(TW_HANDLE memoryHandle);

#endif //TWAINFUL_DSMINTERFACE_H
