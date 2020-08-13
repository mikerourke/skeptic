#include "TwainString.h"

#define kTEMP_BUFFER_SIZE 1024

char *nextTempBuffer() {
  static char tempBuffer[3][kTEMP_BUFFER_SIZE];
  static int bufferIndex = 0;

  if (++bufferIndex >= 3) {
    bufferIndex = 0;
  }

  tempBuffer[bufferIndex][0] = '\0';

  return tempBuffer[bufferIndex];
}

char *unknownValue(const char *fieldName, const TW_UINT16 value) {
  char *buffer = nextTempBuffer();
  Printf(
    buffer,
    kTEMP_BUFFER_SIZE,
    "%s 0x:%04X",
    fieldName,
    value);
  return buffer;
}

const char *CapabilityToString(const TW_UINT16 capability) {
  switch (capability) {
    case ICAP_AUTODISCARDBLANKPAGES:
      return "ICAP_AUTODISCARDBLANKPAGES";
    case CAP_CUSTOMBASE:
      return "CAP_CUSTOMBASE";

    case CAP_XFERCOUNT:
      return "CAP_XFERCOUNT";

    case ICAP_COMPRESSION:
      return "ICAP_COMPRESSION";

    case ICAP_PIXELTYPE:
      return "ICAP_PIXELTYPE";

    case ICAP_UNITS:
      return "ICAP_UNITS";

    case ICAP_XFERMECH:
      return "ICAP_XFERMECH";

    case CAP_AUTHOR:
      return "CAP_AUTHOR";

    case CAP_CAPTION:
      return "CAP_CAPTION";

    case CAP_FEEDERENABLED:
      return "CAP_FEEDERENABLED";

    case CAP_FEEDERLOADED:
      return "CAP_FEEDERLOADED";

    case CAP_TIMEDATE:
      return "CAP_TIMEDATE";

    case CAP_SUPPORTEDCAPS:
      return "CAP_SUPPORTEDCAPS";

    case CAP_EXTENDEDCAPS:
      return "CAP_EXTENDEDCAPS";

    case CAP_AUTOFEED:
      return "CAP_AUTOFEED";

    case CAP_CLEARPAGE:
      return "CAP_CLEARPAGE";

    case CAP_FEEDPAGE:
      return "CAP_FEEDPAGE";

    case CAP_REWINDPAGE:
      return "CAP_REWINDPAGE";

    case CAP_INDICATORS:
      return "CAP_INDICATORS";

    case CAP_SUPPORTEDCAPSEXT:
      return "CAP_SUPPORTEDCAPSEXT";

    case CAP_PAPERDETECTABLE:
      return "CAP_PAPERDETECTABLE";

    case CAP_UICONTROLLABLE:
      return "CAP_UICONTROLLABLE";

    case CAP_DEVICEONLINE:
      return "CAP_DEVICEONLINE";

    case CAP_AUTOSCAN:
      return "CAP_AUTOSCAN";

    case CAP_THUMBNAILSENABLED:
      return "CAP_THUMBNAILSENABLED";

    case CAP_DUPLEX:
      return "CAP_DUPLEX";

    case CAP_DUPLEXENABLED:
      return "CAP_DUPLEXENABLED";

    case CAP_ENABLEDSUIONLY:
      return "CAP_ENABLEDSUIONLY";

    case CAP_CUSTOMDSDATA:
      return "CAP_CUSTOMDSDATA";

    case CAP_ENDORSER:
      return "CAP_ENDORSER";

    case CAP_JOBCONTROL:
      return "CAP_JOBCONTROL";

    case CAP_ALARMS:
      return "CAP_ALARMS";

    case CAP_ALARMVOLUME:
      return "CAP_ALARMVOLUME";

    case CAP_AUTOMATICCAPTURE:
      return "CAP_AUTOMATICCAPTURE";

    case CAP_TIMEBEFOREFIRSTCAPTURE:
      return "CAP_TIMEBEFOREFIRSTCAPTURE";

    case CAP_TIMEBETWEENCAPTURES:
      return "CAP_TIMEBETWEENCAPTURES";

    case CAP_CLEARBUFFERS:
      return "CAP_CLEARBUFFERS";

    case CAP_MAXBATCHBUFFERS:
      return "CAP_MAXBATCHBUFFERS";

    case CAP_DEVICETIMEDATE:
      return "CAP_DEVICETIMEDATE";

    case CAP_POWERSUPPLY:
      return "CAP_POWERSUPPLY";

    case CAP_CAMERAPREVIEWUI:
      return "CAP_CAMERAPREVIEWUI";

    case CAP_DEVICEEVENT:
      return "CAP_DEVICEEVENT";

    case CAP_SERIALNUMBER:
      return "CAP_SERIALNUMBER";

    case CAP_PRINTER:
      return "CAP_PRINTER";

    case CAP_PRINTERENABLED:
      return "CAP_PRINTERENABLED";

    case CAP_PRINTERINDEX:
      return "CAP_PRINTERINDEX";

    case CAP_PRINTERMODE:
      return "CAP_PRINTERMODE";

    case CAP_PRINTERSTRING:
      return "CAP_PRINTERSTRING";

    case CAP_PRINTERSUFFIX:
      return "CAP_PRINTERSUFFIX";

    case CAP_LANGUAGE:
      return "CAP_LANGUAGE";

    case CAP_FEEDERALIGNMENT:
      return "CAP_FEEDERALIGNMENT";

    case CAP_FEEDERORDER:
      return "CAP_FEEDERORDER";

    case CAP_REACQUIREALLOWED:
      return "CAP_REACQUIREALLOWED";

    case CAP_BATTERYMINUTES:
      return "CAP_BATTERYMINUTES";

    case CAP_BATTERYPERCENTAGE:
      return "CAP_BATTERYPERCENTAGE";

    case CAP_CAMERASIDE:
      return "CAP_CAMERASIDE";

    case CAP_SEGMENTED:
      return "CAP_SEGMENTED";

    case CAP_CAMERAENABLED:
      return "CAP_CAMERAENABLED";

    case CAP_CAMERAORDER:
      return "CAP_CAMERAORDER";

    case CAP_MICRENABLED:
      return "CAP_MICRENABLED";

    case CAP_FEEDERPREP:
      return "CAP_FEEDERPREP";

    case CAP_FEEDERPOCKET:
      return "CAP_FEEDERPOCKET";

    case CAP_AUTOMATICSENSEMEDIUM:
      return "CAP_AUTOMATICSENSEMEDIUM";

    case CAP_CUSTOMINTERFACEGUID:
      return "CAP_CUSTOMINTERFACEGUID";

    case ICAP_AUTOBRIGHT:
      return "ICAP_AUTOBRIGHT";

    case ICAP_BRIGHTNESS:
      return "ICAP_BRIGHTNESS";

    case ICAP_CONTRAST:
      return "ICAP_CONTRAST";

    case ICAP_CUSTHALFTONE:
      return "ICAP_CUSTHALFTONE";

    case ICAP_EXPOSURETIME:
      return "ICAP_EXPOSURETIME";

    case ICAP_FILTER:
      return "ICAP_FILTER";

    case ICAP_FLASHUSED:
      return "ICAP_FLASHUSED";

    case ICAP_GAMMA:
      return "ICAP_GAMMA";

    case ICAP_HALFTONES:
      return "ICAP_HALFTONES";

    case ICAP_HIGHLIGHT:
      return "ICAP_HIGHLIGHT";

    case ICAP_IMAGEFILEFORMAT:
      return "ICAP_IMAGEFILEFORMAT";

    case ICAP_LAMPSTATE:
      return "ICAP_LAMPSTATE";

    case ICAP_LIGHTSOURCE:
      return "ICAP_LIGHTSOURCE";

    case ICAP_ORIENTATION:
      return "ICAP_ORIENTATION";

    case ICAP_PHYSICALWIDTH:
      return "ICAP_PHYSICALWIDTH";

    case ICAP_PHYSICALHEIGHT:
      return "ICAP_PHYSICALHEIGHT";

    case ICAP_SHADOW:
      return "ICAP_SHADOW";

    case ICAP_FRAMES:
      return "ICAP_FRAMES";

    case ICAP_XNATIVERESOLUTION:
      return "ICAP_XNATIVERESOLUTION";

    case ICAP_YNATIVERESOLUTION:
      return "ICAP_YNATIVERESOLUTION";

    case ICAP_XRESOLUTION:
      return "ICAP_XRESOLUTION";

    case ICAP_YRESOLUTION:
      return "ICAP_YRESOLUTION";

    case ICAP_MAXFRAMES:
      return "ICAP_MAXFRAMES";

    case ICAP_TILES:
      return "ICAP_TILES";

    case ICAP_BITORDER:
      return "ICAP_BITORDER";

    case ICAP_CCITTKFACTOR:
      return "ICAP_CCITTKFACTOR";

    case ICAP_LIGHTPATH:
      return "ICAP_LIGHTPATH";

    case ICAP_PIXELFLAVOR:
      return "ICAP_PIXELFLAVOR";

    case ICAP_PLANARCHUNKY:
      return "ICAP_PLANARCHUNKY";

    case ICAP_ROTATION:
      return "ICAP_ROTATION";

    case ICAP_SUPPORTEDSIZES:
      return "ICAP_SUPPORTEDSIZES";

    case ICAP_THRESHOLD:
      return "ICAP_THRESHOLD";

    case ICAP_XSCALING:
      return "ICAP_XSCALING";

    case ICAP_YSCALING:
      return "ICAP_YSCALING";

    case ICAP_BITORDERCODES:
      return "ICAP_BITORDERCODES";

    case ICAP_PIXELFLAVORCODES:
      return "ICAP_PIXELFLAVORCODES";

    case ICAP_JPEGPIXELTYPE:
      return "ICAP_JPEGPIXELTYPE";

    case ICAP_TIMEFILL:
      return "ICAP_TIMEFILL";

    case ICAP_BITDEPTH:
      return "ICAP_BITDEPTH";

    case ICAP_BITDEPTHREDUCTION:
      return "ICAP_BITDEPTHREDUCTION";

    case ICAP_UNDEFINEDIMAGESIZE:
      return "ICAP_UNDEFINEDIMAGESIZE";

    case ICAP_IMAGEDATASET:
      return "ICAP_IMAGEDATASET";

    case ICAP_EXTIMAGEINFO:
      return "ICAP_EXTIMAGEINFO";

    case ICAP_MINIMUMHEIGHT:
      return "ICAP_MINIMUMHEIGHT";

    case ICAP_MINIMUMWIDTH:
      return "ICAP_MINIMUMWIDTH";

    case ICAP_FLIPROTATION:
      return "ICAP_FLIPROTATION";

    case ICAP_BARCODEDETECTIONENABLED:
      return "ICAP_BARCODEDETECTIONENABLED";

    case ICAP_SUPPORTEDBARCODETYPES:
      return "ICAP_SUPPORTEDBARCODETYPES";

    case ICAP_BARCODEMAXSEARCHPRIORITIES:
      return "ICAP_BARCODEMAXSEARCHPRIORITIES";

    case ICAP_BARCODESEARCHPRIORITIES:
      return "ICAP_BARCODESEARCHPRIORITIES";

    case ICAP_BARCODESEARCHMODE:
      return "ICAP_BARCODESEARCHMODE";

    case ICAP_BARCODEMAXRETRIES:
      return "ICAP_BARCODEMAXRETRIES";

    case ICAP_BARCODETIMEOUT:
      return "ICAP_BARCODETIMEOUT";

    case ICAP_ZOOMFACTOR:
      return "ICAP_ZOOMFACTOR";

    case ICAP_PATCHCODEDETECTIONENABLED:
      return "ICAP_PATCHCODEDETECTIONENABLED";

    case ICAP_SUPPORTEDPATCHCODETYPES:
      return "ICAP_SUPPORTEDPATCHCODETYPES";

    case ICAP_PATCHCODEMAXSEARCHPRIORITIES:
      return "ICAP_PATCHCODEMAXSEARCHPRIORITIES";

    case ICAP_PATCHCODESEARCHPRIORITIES:
      return "ICAP_PATCHCODESEARCHPRIORITIES";

    case ICAP_PATCHCODESEARCHMODE:
      return "ICAP_PATCHCODESEARCHMODE";

    case ICAP_PATCHCODEMAXRETRIES:
      return "ICAP_PATCHCODEMAXRETRIES";

    case ICAP_PATCHCODETIMEOUT:
      return "ICAP_PATCHCODETIMEOUT";

    case ICAP_FLASHUSED2:
      return "ICAP_FLASHUSED2";

    case ICAP_IMAGEFILTER:
      return "ICAP_IMAGEFILTER";

    case ICAP_NOISEFILTER:
      return "ICAP_NOISEFILTER";

    case ICAP_OVERSCAN:
      return "ICAP_OVERSCAN";

    case ICAP_AUTOMATICBORDERDETECTION:
      return "ICAP_AUTOMATICBORDERDETECTION";

    case ICAP_AUTOMATICDESKEW:
      return "ICAP_AUTOMATICDESKEW";

    case ICAP_AUTOMATICROTATE:
      return "ICAP_AUTOMATICROTATE";

    case ICAP_JPEGQUALITY:
      return "ICAP_JPEGQUALITY";

    case ICAP_FEEDERTYPE:
      return "ICAP_FEEDERTYPE";

    case ICAP_ICCPROFILE:
      return "ICAP_ICCPROFILE";

    case ICAP_AUTOSIZE:
      return "ICAP_AUTOSIZE";

    case ICAP_AUTOMATICCROPUSESFRAME:
      return "ICAP_AUTOMATICCROPUSESFRAME";

    case ICAP_AUTOMATICLENGTHDETECTION:
      return "ICAP_AUTOMATICLENGTHDETECTION";

    case ICAP_AUTOMATICCOLORENABLED:
      return "ICAP_AUTOMATICCOLORENABLED";

    case ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE:
      return "ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE";

    case ICAP_COLORMANAGEMENTENABLED:
      return "ICAP_COLORMANAGEMENTENABLED";

    case ICAP_IMAGEMERGE:
      return "ICAP_IMAGEMERGE";

    case ICAP_IMAGEMERGEHEIGHTTHRESHOLD:
      return "ICAP_IMAGEMERGEHEIGHTTHRESHOLD";

    case ICAP_SUPPORTEDEXTIMAGEINFO:
      return "ICAP_SUPPORTEDEXTIMAGEINFO";

    case ACAP_AUDIOFILEFORMAT:
      return "ACAP_AUDIOFILEFORMAT";

    case ACAP_XFERMECH:
      return "ACAP_XFERMECH";

    default: {
      auto fieldName = "";
      if (capability < CAP_CUSTOMBASE) {
        fieldName = "Unknown CAP";
      } else if (capability > CAP_CUSTOMBASE) {
        fieldName = "Custom CAP";
      } else {
        fieldName = "Invalid CAP";
      }

      return unknownValue(fieldName, capability);
    }
  }
}

const char *CapabilityItemToString(
  TW_UINT16 id,
  TW_UINT32 item,
  TW_UINT16 itemType) {
  return "NOT IMPLEMENTED";
}

const char *ConditionCodeToString(const TW_UINT16 conditionCode) {
  switch (conditionCode) {
    case TWCC_SUCCESS:
      return "TWCC_SUCCESS";

    case TWCC_BUMMER:
      return "TWCC_BUMMER";

    case TWCC_LOWMEMORY:
      return "TWCC_LOWMEMORY";

    case TWCC_NODS:
      return "TWCC_NODS";

    case TWCC_MAXCONNECTIONS:
      return "TWCC_MAXCONNECTIONS";

    case TWCC_OPERATIONERROR:
      return "TWCC_OPERATIONERROR";

    case TWCC_BADCAP:
      return "TWCC_BADCAP";

    case TWCC_BADPROTOCOL:
      return "TWCC_BADPROTOCOL";

    case TWCC_BADVALUE:
      return "TWCC_BADVALUE";

    case TWCC_SEQERROR:
      return "TWCC_SEQERROR";

    case TWCC_BADDEST:
      return "TWCC_BADDEST";

    case TWCC_CAPUNSUPPORTED:
      return "TWCC_CAPUNSUPPORTED";

    case TWCC_CAPBADOPERATION:
      return "TWCC_CAPBADOPERATION";

    case TWCC_CAPSEQERROR:
      return "TWCC_CAPSEQERROR";

    case TWCC_DENIED:
      return "TWCC_DENIED";

    case TWCC_FILEEXISTS:
      return "TWCC_FILEEXISTS";

    case TWCC_FILENOTFOUND:
      return "TWCC_FILENOTFOUND";

    case TWCC_NOTEMPTY:
      return "TWCC_NOTEMPTY";

    case TWCC_PAPERJAM:
      return "TWCC_PAPERJAM";

    case TWCC_PAPERDOUBLEFEED:
      return "TWCC_PAPERDOUBLEFEED";

    case TWCC_FILEWRITEERROR:
      return "TWCC_FILEWRITEERROR";

    case TWCC_CHECKDEVICEONLINE:
      return "TWCC_CHECKDEVICEONLINE";

    case TWCC_INTERLOCK:
      return "TWCC_INTERLOCK";

    case TWCC_DAMAGEDCORNER:
      return "TWCC_DAMAGEDCORNER";

    case TWCC_FOCUSERROR:
      return "TWCC_FOCUSERROR";

    case TWCC_DOCTOOLIGHT:
      return "TWCC_DOCTOOLIGHT";

    case TWCC_DOCTOODARK:
      return "TWCC_DOCTOODARK";

    case TWCC_NOMEDIA:
      return "TWCC_NOMEDIA";

    default:
      return unknownValue("Unknown Condition Code", conditionCode);
  }
}

const char *ImageFileFormatToExtension(const TW_UINT16 imageFileFormat) {
  switch (imageFileFormat) {
    case TWFF_PICT:
      return ".pict";

    case TWFF_BMP:
      return ".bmp";

    case TWFF_XBM:
      return ".xbm";

    case TWFF_JFIF:
      return ".jpeg";

    case TWFF_FPX:
      return ".fpx";

    case TWFF_TIFF:
    case TWFF_TIFFMULTI:
      return ".tiff";

    case TWFF_PNG:
      return ".png";

    case TWFF_SPIFF:
      return ".spiff";

    case TWFF_EXIF:
      return ".exif";

    case TWFF_JP2:
      return ".jp2";

    case TWFF_JPN:
      return ".jpn";

    case TWFF_JPX:
      return ".jpx";

    case TWFF_DEJAVU:
      return ".dejavu";

    case TWFF_PDF:
    case TWFF_PDFA:
    case TWFF_PDFA2:
      return ".pdf";

    default:
      return unknownValue("Unknown Image File Format", imageFileFormat);
  }
}


const char *TwainTypeToString(const TW_UINT16 type) {
  switch (type) {
    case TWTY_INT8:
      return "TWTY_INT8";

    case TWTY_INT16:
      return "TWTY_INT16";

    case TWTY_INT32:
      return "TWTY_INT32";

    case TWTY_UINT8:
      return "TWTY_UINT8";

    case TWTY_UINT16:
      return "TWTY_UINT16";

    case TWTY_UINT32:
      return "TWTY_UINT32";

    case TWTY_BOOL:
      return "TWTY_BOOL";

    case TWTY_FIX32:
      return "TWTY_FIX32";

    case TWTY_FRAME:
      return "TWTY_FRAME";

    case TWTY_STR32:
      return "TWTY_STR32";

    case TWTY_STR64:
      return "TWTY_STR64";

    case TWTY_STR128:
      return "TWTY_STR128";

    case TWTY_STR255:
      return "TWTY_STR255";

    case TWTY_STR1024:
      return "TWTY_STR1024";

    case TWTY_UNI512:
      return "TWTY_UNI512";

    case TWTY_HANDLE:
      return "TWTY_HANDLE";

    default:
      return unknownValue("Unknown Type", type);
  }
}