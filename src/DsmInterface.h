#ifndef SKEPTIC_DSMINTERFACE_H
#define SKEPTIC_DSMINTERFACE_H

#pragma once

#include <TWAIN/TWAIN.h>

#define kDSM_LIBRARY_PATH "/System/Library/Frameworks/TWAIN.framework"

enum class DsmState {
  Unknown = 1,
  Closed = 2,
  Open = 3,
  DataSourceLoaded = 4,
  DataSourceEnabled = 5,
  ReadyToScan = 6,
  StripReceived = 7,
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

#endif //SKEPTIC_DSMINTERFACE_H
