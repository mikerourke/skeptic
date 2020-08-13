#include <iostream>
#include <semaphore.h>
#include "CommonTwain.h"
#include "Application.h"
#include "UserInterface.h"

using namespace std;

Application *gpApplication;

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

  if (pOrigin == nullptr || pOrigin->Id != gpApplication->GetDataSource()->Id) {
    return TWRC_FAILURE;
  }

  switch (message) {
    case MSG_XFERREADY:
    case MSG_CLOSEDSREQ:
    case MSG_CLOSEDSOK:
    case MSG_NULL:
      gpApplication->DataSourceMessage = message;
      sem_post(&(gpApplication->TwainEvent));
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

  gpApplication = new Application();

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
      gpApplication->ConnectDsm();
    } else if (input == "xdsm") {
      gpApplication->DisconnectDsm();
    } else {
      PrintOptions();
    }
  }

  gpApplication->Exit();
  delete gpApplication;
  gpApplication = nullptr;

  return ret;
}
