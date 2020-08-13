#include <iostream>
#include <sstream>

#include "TwainString.h"
#include "UserInterface.h"

using namespace std;

string GetUnexpectedTypeErrorString(const TW_UINT16 invalidType) {
  ostringstream errorMessage;

  errorMessage
    << "The type is an unexpected value. "
    << " got " << TwainTypeToString(invalidType);

  return errorMessage.str();
}

void PrintOptions() {
  cout
    << "\n"
    << "Please enter an option\n"
    << "----------------------\n"
    << "q    - quit\n"
    << "h    - help\n"
    << "cdsm - connect to the dsm\n"
    << "xdsm - disconnect from the dsm\n"
    << "lds  - list data sources available\n"
    << "pds# - print identity structure for data source with id#. ex: pds2\n"
    << "cds# - connect to data source with id#. ex: cds2\n"
    << "xds  - disconnect from data source\n"
    << "caps - negotiate capabilities\n"
    << "scan - start the scan\n"
    << endl;
}

void PrintMainCapabilities() {
  cout
    << "\n"
    << "Capabilites\n"
    << "-----------\n"
    << "q - done negotiating, go back to main menu\n"
    << "h - help\n"
    << "1 - ICAP_XFERMECH\n"
    << "2 - ICAP_PIXELTYPE\n"
    << "3 - ICAP_BITDEPTH\n"
    << "4 - ICAP_XRESOLUTION\n"
    << "5 - ICAP_YRESOLUTION\n"
    << "6 - ICAP_FRAMES\n"
    << "7 - ICAP_UNITS\n"
    << endl;
}

void PrintCapability(const TW_UINT16 id, pTW_ONEVALUE pCapability) {
  if (pCapability == nullptr) {
    return;
  }

  cout
    << "\n"
    << CapabilityToString(id) << "\n"
    << "--------------\n"
    << "Showing supported types. * indicates current setting.\n\n"
    << "q - done\n";

  switch (pCapability->ItemType) {
    case TWTY_FIX32: {
      auto pFix32 = (pTW_FIX32) &pCapability->Item;
      cout << "1 - " << pFix32->Whole << "." << pFix32->Frac << "*\n" << endl;
    }
      break;

    case TWTY_FRAME: {
      auto pFrame = (pTW_FRAME) &pCapability->Item;

      cout
        << "1 - Frame Data:*\n"
        << "\tLeft,\tTop,\tRight,\tBottom\n"
        << "\t" << Fix32ToFloat(pFrame->Left) << ",\t"
        << Fix32ToFloat(pFrame->Top) << ",\t"
        << Fix32ToFloat(pFrame->Right) << ",\t"
        << Fix32ToFloat(pFrame->Bottom) << "\n"
        << "\n"
        << endl;
    }
      break;

    case TWTY_INT8:
    case TWTY_INT16:
    case TWTY_INT32:
    case TWTY_UINT8:
    case TWTY_UINT16:
    case TWTY_UINT32:
    case TWTY_BOOL: {
      cout << "1 - " << CapabilityItemToString(id, pCapability->Item, pCapability->ItemType)
           << "*" << endl;
    }
      break;

    case TWTY_STR32: {
      cout << "1 - " << (pTW_STR32) &pCapability->ItemType << "*" << endl;
    }
      break;

    case TWTY_STR64: {
      cout << "1 - " << (pTW_STR64) &pCapability->ItemType << "*" << endl;
    }
      break;

    case TWTY_STR128: {
      cout << "1 - " << (pTW_STR128) &pCapability->ItemType << "*" << endl;
    }
      break;

    case TWTY_STR255: {
      cout << "1 - " << (pTW_STR255) &pCapability->ItemType << "*" << endl;
    }
      break;

    default: {
      cerr << GetUnexpectedTypeErrorString(pCapability->ItemType) << endl;
    }
      break;

  }
}

void PrintCapability(const TW_UINT16 id, pTW_ENUMERATION pCapability) {
  if (pCapability == nullptr) {
    return;
  }

  cout
    << "\n"
    << CapabilityToString(id) << "\n"
    << "--------------\n"
    << "Showing supported types. * indicates current setting.\n\n"
    << "q - done\n";
}