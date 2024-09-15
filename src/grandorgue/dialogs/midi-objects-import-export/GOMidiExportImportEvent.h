/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportEvent_H
#define GOMidiExportImportEvent_H

#include <wx/wx.h>

class GOMidiExportImportEvent;
using GOMidiExportImportEventPtr = std::shared_ptr<GOMidiExportImportEvent>;
using GOMidiExportImportEventVector = std::vector<GOMidiExportImportEventPtr>;

class GOMidiExportImportEvent
  : public std::enable_shared_from_this<GOMidiExportImportEvent> {
public:
  enum EventType {
    UNKNOWN_TYPE,
    NOTE_OFF_TYPE,
    NOTE_ON_TYPE,
    NOTE_ON_NO_VELOCITY_TYPE,
    CONTROL_CHANGE_TYPE,
    PROGRAM_CHANGE_TYPE,
    RPN_TYPE,
    NRPN_TYPE,
    LCD_SYSEX_TYPE,

    CONTENT_CONTROL_CHANGE_BITFIELD_TYPE,
    AHLBORN_GALANTI_SYSEX_TYPE,
    JOHANNUS_9BYTES_SYSEX_TYPE,
    JOHANNUS_11BYTES_SYSEX_TYPE,
    RODGERS_STOP_CHANGE_SYSEX_TYPE,
    VISCOUNT_SYSEX_TYPE,
  };
  enum ReceiveSendFunction {
    NO_FUNCTION,
    UNKNOWN_FUNCTION,
    KEY_PRESS_FUNCTION,              // manual, rank
    KEY_PRESS_SHORT_OCTAVE_FUNCTION, // manual
    KEY_PRESS_NO_MAP_FUNCTION,       // manual
    DIVISION_KEY_PRESS_FUNCTION,     // manual, rank
    ON_FUNCTION,                     // button
    OFF_FUNCTION,                    // button
    TOGGLE_FUNCTION,                 // button
    VALUE_FUNCTION,                  // enclosure, LCD
    NAME_FUNCTION,                   // LCD
    ON_OFF_FUNCTION,                 // CONTENT_CONTROL_CHANGE_BITFIELD_TYPE and
                                     // RODGERS_STOP_CHANGE_SYSEX_TYPE
  };

private:
  unsigned int m_midiDeviceID;
  int m_midiChannel;
  EventType m_eventType;
  //	9x			Bx			Cx
  //(N)RPN		LCD
  unsigned int m_low;    //	key bank
                         // start pos.
  unsigned int m_high;   //	key bank end pos.
  unsigned int m_number; //	transpose	controller
                         // parameter	ID
  unsigned int m_min;    //	velocity	value		program
                         // value		color
  unsigned int m_max;    //	velocity	value		program
                         // value		total length
  unsigned int m_debounceTime;
  /*
  Ahlborn Galanti SysEx:
          m_number = data
  Content Control Change Bitfield:
          m_number = controller, m_min = bit numer
  Johannus 9/11 bytes SysEx:
          m_number = data
  Rogers Stop Change SysEx:
          m_number = device, m_min = stop
  Viscount SysEx:
          m_number = data
  */
  ReceiveSendFunction m_receiveFunction;
  ReceiveSendFunction m_sendFunction;

public:
  // Util
  static wxString GetEventTypeName(EventType eventType);
  static wxString GetReceiveSendFunctionName(ReceiveSendFunction function);
  // getters and setters
  unsigned int GetMidiDeviceID() const { return m_midiDeviceID; }
  void SetMidiDeviceID(unsigned int midiDeviceID) {
    m_midiDeviceID = midiDeviceID;
  }
  wxString GetMidiDeviceName() const {
    return wxString::Format(
      wxT("%d"), (int)m_midiDeviceID); // TODO: name of device
  }
  int GetMidiChannel() const { return m_midiChannel; }
  void SetMidiChannel(int midiChannel) { m_midiChannel = midiChannel; }
  EventType GetEventType() const { return m_eventType; }
  void SetEventType(EventType eventType) { m_eventType = eventType; }
  wxString GetEventTypeName() const {
    return GOMidiExportImportEvent::GetEventTypeName(m_eventType);
  }
  unsigned int GetLow() const { return m_low; }
  void SetLow(unsigned int low) { m_low = low; }
  unsigned int GetHigh() const { return m_high; }
  void SetHigh(unsigned int high) { m_high = high; }
  void SetLowAndHigh(unsigned int value) {
    m_low = value;
    m_high = value;
  }
  unsigned int GetNumber() const { return m_number; }
  void SetNumber(unsigned int number) { m_number = number; }
  unsigned int GetMin() const { return m_min; }
  void SetMin(unsigned int min) { m_min = min; }
  unsigned int GetMax() const { return m_max; }
  void SetMax(unsigned int max) { m_max = max; }
  void SetMinAndMax(unsigned int value) {
    m_min = value;
    m_max = value;
  }
  unsigned int GetDebounceTime() const { return m_debounceTime; }
  void SetDebounceTime(unsigned int debounceTime) {
    m_debounceTime = debounceTime;
  }
  ReceiveSendFunction GetReceiveFunction() const { return m_receiveFunction; }
  void SetReceiveFunction(ReceiveSendFunction receiveFunction) {
    m_receiveFunction = receiveFunction;
  }
  wxString GetReceiveFunctionName() const {
    return GetReceiveSendFunctionName(m_receiveFunction);
  }
  ReceiveSendFunction GetSendFunction() const { return m_sendFunction; }
  void SetSendFunction(ReceiveSendFunction sendFunction) {
    m_sendFunction = sendFunction;
  }
  wxString GetSendFunctionName() const {
    return GetReceiveSendFunctionName(m_sendFunction);
  }
};

#endif
