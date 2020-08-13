#include <iostream>
#include <semaphore.h>
#include "CommonTwain.h"
#include "TwainApp.h"
#include "UserInterface.h"

using namespace std;

TwainApp *gpTwainApplication;

/**
 * Callback function for DS. This is a callback function that will be called by
 * the source when it is ready for the application to start a scan. This
 * callback needs to be registered with the DSM before it can be called.
 * It is important that the application returns right away after receiving this
 * message. Set a flag and return. Do not process the callback in this function.
 */
FAR PASCAL TW_UINT16 DSMCallback(
  pTW_IDENTITY pOrigin,
  pTW_IDENTITY pDest,
  TW_UINT32 dataGroup,
  TW_UINT16 dataArgumentType,
  TW_UINT16 message,
  TW_MEMREF pData
) {
  UNUSEDARG(pDest);
  UNUSEDARG(dataGroup);
  UNUSEDARG(dataArgumentType);
  UNUSEDARG(pData);

  if (pOrigin == nullptr || pOrigin->Id != gpTwainApplication->GetDataSource()->Id) {
    return TWRC_FAILURE;
  }

  switch (message) {
    case MSG_XFERREADY:
    case MSG_CLOSEDSREQ:
    case MSG_CLOSEDSOK:
    case MSG_NULL:
      gpTwainApplication->DataSourceMessage = message;
      sem_post(&(gpTwainApplication->TwainEvent));
      break;

    default:
      cerr << "Error - Unknown message in callback routine" << endl;
      return TWRC_FAILURE;
  }

  return TWRC_SUCCESS;
}

void onSigInt(int signal) {
  UNUSEDARG(signal);
  cout << "\nGoodbye!" << endl;
  exit(0);
}

int main(int argc, char *argv[]) {
  UNUSEDARG(argc);
  UNUSEDARG(argv);
  int ret = EXIT_SUCCESS;

  cout << "YAY";

  gpTwainApplication = new TwainApp();

  signal(SIGINT, &onSigInt);

  string input;

  PrintOptions();

  // Start the main event loop:
  for (;;) {
    cout << "\n(h for help) > ";
    cin >> input;
    cout << endl;

    if (input == "q") {
      break;
    } else if (input == "h") {
      PrintOptions();
    } else if (input == "cdsm") {
      gpTwainApplication->ConnectDsm();
    } else if (input == "xdsm") {
      gpTwainApplication->DisconnectDsm();
    } else {
      PrintOptions();
    }
  }

  gpTwainApplication->Exit();
  delete gpTwainApplication;
  gpTwainApplication = nullptr;

  return ret;
}
