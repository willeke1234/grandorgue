/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiExportImportEvent.h"

#include "GOMidiExportImportEvent.h"

wxString GOMidiExportImportEvent::GetEventTypeName(EventType eventType) {
  switch (eventType) {
  case UNKNOWN_TYPE:
    return wxT("Unkown");
  case NOTE_OFF_TYPE:
    return wxT("8x Note Off");
  case NOTE_ON_TYPE:
    return wxT("9x Note On");
  case NOTE_ON_NO_VELOCITY_TYPE:
    return wxT("9x Note On No Velocity");
  case CONTROL_CHANGE_TYPE:
    return wxT("Bx Control Change");
  case PROGRAM_CHANGE_TYPE:
    return wxT("Cx Program Change");
  case RPN_TYPE:
    return wxT("RPN");
  case NRPN_TYPE:
    return wxT("NRPN");
  case LCD_SYSEX_TYPE:
    return wxT("LCD SysEx");
  case CONTENT_CONTROL_CHANGE_BITFIELD_TYPE:
    return wxT("Content Control Change Bitfield");
  case AHLBORN_GALANTI_SYSEX_TYPE:
    return wxT("Ahlborn Galanti SysEx");
  case JOHANNUS_9BYTES_SYSEX_TYPE:
    return wxT("Johannus 9 bytes SysEx");
  case JOHANNUS_11BYTES_SYSEX_TYPE:
    return wxT("Johannus 11 bytes SysEx");
  case RODGERS_STOP_CHANGE_SYSEX_TYPE:
    return wxT("Rodgers Stop Change SysEx");
  case VISCOUNT_SYSEX_TYPE:
    return wxT("Viscount SysEx");
  }
  return wxT("Error");
}

wxString GOMidiExportImportEvent::GetReceiveSendFunctionName(
  ReceiveSendFunction function) {
  switch (function) {
  case NO_FUNCTION:
    return wxT("No");
  case UNKNOWN_FUNCTION:
    return wxT("Unkown");
  case KEY_PRESS_FUNCTION:
    return wxT("Key Press");
  case KEY_PRESS_SHORT_OCTAVE_FUNCTION:
    return wxT("Key Press Short Octave");
  case KEY_PRESS_NO_MAP_FUNCTION:
    return wxT("Key Press No Map");
  case DIVISION_KEY_PRESS_FUNCTION:
    return wxT("Division Key Press");
  case ON_FUNCTION:
    return wxT("On");
  case OFF_FUNCTION:
    return wxT("Off");
  case TOGGLE_FUNCTION:
    return wxT("Toggle");
  case VALUE_FUNCTION:
    return wxT("Value");
  case NAME_FUNCTION:
    return wxT("Name");
  case ON_OFF_FUNCTION:
    return wxT("On/Off");
  }
  return wxT("Error");
}
