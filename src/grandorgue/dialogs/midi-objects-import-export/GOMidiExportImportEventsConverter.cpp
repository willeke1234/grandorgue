/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiExportImportEventsConverter.h"
#include "GOMidiExportImportObjectWrapper.h"
#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiReceiverBase.h"
#include "midi/GOMidiSender.h"

void GOMidiExportImportEventsConverter::EventsFromObject(
  GOMidiConfigurator *object,
  GOMidiExportImportObjectWrapper::ObjectType objectType,
  GOMidiExportImportEventVector &events) {
  /* gets the MIDI events of object,
   * converts the events to GOMidiExportImportEvent,
   * adds the events to events
   */
  GOMidiExportImportEventVector sendEvents;
  GOMidiExportImportEventVector receiveEvents;
  switch (objectType) {
  case GOMidiExportImportObjectWrapper::UNKNOWN_TYPE:
    break;
  case GOMidiExportImportObjectWrapper::BUTTON_TYPE:
    EventsFromButton(object, sendEvents, receiveEvents);
    break;
  case GOMidiExportImportObjectWrapper::LABEL_TYPE:
    EventsFromLabel(object, sendEvents, receiveEvents);
    break;
  case GOMidiExportImportObjectWrapper::ENCLOSURE_TYPE:
    EventsFromEnclosure(object, sendEvents, receiveEvents);
    break;
  case GOMidiExportImportObjectWrapper::MANUAL_TYPE:
  case GOMidiExportImportObjectWrapper::RANK_TYPE:
    EventsFromManualOrRank(object, sendEvents, receiveEvents);
    break;
  }
  MergeEvents(sendEvents, receiveEvents, events);
}

void GOMidiExportImportEventsConverter::MergeEvents(
  GOMidiExportImportEventVector &sendEvents,
  GOMidiExportImportEventVector &receiveEvents,
  GOMidiExportImportEventVector &events) {
  /* merges sendEvents and receiveEvents into events
   * a sendEvent and a receiveEvent with the same properties are merged
   * the send events must stay in the same order
   */
  for (GOMidiExportImportEventPtr receiveEvent : receiveEvents) {
    bool isMerged = false;
    for (GOMidiExportImportEventPtr sendEvent : sendEvents) {
      if (
        receiveEvent->GetMidiDeviceID() == sendEvent->GetMidiDeviceID()
        && receiveEvent->GetMidiChannel() == sendEvent->GetMidiChannel()
        && receiveEvent->GetEventType() == sendEvent->GetEventType()
        && receiveEvent->GetLow() == sendEvent->GetLow()
        && receiveEvent->GetHigh() == sendEvent->GetHigh()
        && receiveEvent->GetNumber() == sendEvent->GetNumber()
        && receiveEvent->GetMin() == sendEvent->GetMin()
        && receiveEvent->GetMax() == sendEvent->GetMax()
        && sendEvent->GetReceiveFunction()
          == GOMidiExportImportEvent::NO_FUNCTION) {
        sendEvent->SetReceiveFunction(receiveEvent->GetReceiveFunction());
        sendEvent->SetDebounceTime(receiveEvent->GetDebounceTime());
        isMerged = true;
      }
    }
    if (!isMerged)
      events.push_back(receiveEvent);
  }
  for (GOMidiExportImportEventPtr sendEvent : sendEvents)
    events.push_back(sendEvent);
}

void GOMidiExportImportEventsConverter::EventsFromButton(
  GOMidiConfigurator *object,
  GOMidiExportImportEventVector &sendEvents,
  GOMidiExportImportEventVector &receiveEvents) {
  const GOMidiReceiverBase *midiReceiver = object->GetMidiReceiver();
  const GOMidiSender *midiSender = object->GetMidiSender();
  if (midiSender) {
    for (unsigned i = 0; i < midiSender->GetEventCount(); i++) {
      const GOMidiSenderEventPattern &eventPattern = midiSender->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      GOMidiExportImportEventPtr event2
        = std::make_shared<GOMidiExportImportEvent>();
      event2->SetMidiDeviceID(eventPattern.deviceId);
      event2->SetMidiChannel(eventPattern.channel);
      switch (eventPattern.type) {
      case MIDI_S_NONE:
        break;
      case MIDI_S_NOTE:
        // two events, on and off
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value); // velocity
        sendEvents.push_back(event);
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_OFF_TYPE);
        event2->SetLowAndHigh(eventPattern.key);
        if (eventPattern.low_value == 0 && eventPattern.useNoteOff) {
          event2->SetMinAndMax(0); // velocity
        } else {
          event2->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
          event2->SetMinAndMax(eventPattern.low_value); // velocity
        }
        sendEvents.push_back(event2);
        break;
      case MIDI_S_CTRL:
        // two events, on and off
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_RPN:
        // two events, on and off
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_NRPN:
        // two events, on and off
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_RPN_RANGE:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.high_value);
        event->SetMinAndMax(eventPattern.key);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event2->SetNumber(eventPattern.low_value);
        event2->SetMinAndMax(eventPattern.key);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_NRPN_RANGE:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.high_value);
        event->SetMinAndMax(eventPattern.key);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event2->SetNumber(eventPattern.low_value);
        event2->SetMinAndMax(eventPattern.key);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_PGM_RANGE:
        // two events, on and off
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLowAndHigh(eventPattern.high_value >> 7);
        event->SetMinAndMax(eventPattern.high_value & 0x7F);
        sendEvents.push_back(event);
        event2->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event2->SetLowAndHigh(eventPattern.low_value >> 7);
        event2->SetMinAndMax(eventPattern.low_value & 0x7F);
        sendEvents.push_back(event2);
        break;
      case MIDI_S_PGM_ON:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLowAndHigh(eventPattern.key >> 7);
        event->SetMinAndMax(eventPattern.key & 0x7F);
        sendEvents.push_back(event);
        break;
      case MIDI_S_PGM_OFF:
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLowAndHigh(eventPattern.key >> 7);
        event->SetMinAndMax(eventPattern.key & 0x7F);
        sendEvents.push_back(event);
        break;
        break;
      case MIDI_S_NOTE_ON:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetLowAndHigh(eventPattern.high_value); // velocity
        sendEvents.push_back(event);
        break;
      case MIDI_S_NOTE_OFF:
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetLowAndHigh(eventPattern.key);
        if (eventPattern.low_value == 0) {
          event->SetEventType(GOMidiExportImportEvent::NOTE_OFF_TYPE);
          event->SetMinAndMax(0); // velocity
        } else {
          event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
          event->SetMinAndMax(eventPattern.low_value); // velocity
        }
        sendEvents.push_back(event);
        break;
      case MIDI_S_CTRL_ON:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_CTRL_OFF:
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_RPN_ON:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_RPN_OFF:
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_NRPN_ON:
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_NRPN_OFF:
        event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.low_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_RODGERS_STOP_CHANGE:
        // TODO: implement
        break;
      case MIDI_S_HW_NAME_STRING:
      case MIDI_S_HW_NAME_LCD:
      case MIDI_S_HW_STRING:
      case MIDI_S_HW_LCD:
        EventsFromMidiHWEvent(eventPattern, sendEvents);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
  if (midiReceiver) {
    for (unsigned i = 0; i < midiReceiver->GetEventCount(); i++) {
      const GOMidiReceiverEventPattern &eventPattern
        = midiReceiver->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      event->SetDebounceTime(eventPattern.debounce_time);
      GOMidiExportImportEventPtr event2
        = std::make_shared<GOMidiExportImportEvent>();
      event2->SetMidiDeviceID(eventPattern.deviceId);
      event2->SetMidiChannel(eventPattern.channel);
      event2->SetDebounceTime(eventPattern.debounce_time);
      switch (eventPattern.type) {
      case MIDI_M_NONE:
        break;
      case MIDI_M_NOTE:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMin(eventPattern.high_value); // velocity
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event2->SetLowAndHigh(eventPattern.key);
        event2->SetMin(0); // velocity
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_CTRL_CHANGE:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_PGM_CHANGE:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLowAndHigh(eventPattern.key >> 7);
        event->SetMinAndMax(eventPattern.key & 0x7F);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_PGM_RANGE:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLowAndHigh(eventPattern.high_value >> 7);
        event->SetMinAndMax(eventPattern.high_value & 0x7F);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event2->SetLowAndHigh(eventPattern.low_value >> 7);
        event2->SetMinAndMax(eventPattern.low_value & 0x7F);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_SYSEX_JOHANNUS_9:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(
          GOMidiExportImportEvent::JOHANNUS_9BYTES_SYSEX_TYPE);
        event->SetNumber(eventPattern.key);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_SYSEX_JOHANNUS_11:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(
          GOMidiExportImportEvent::JOHANNUS_11BYTES_SYSEX_TYPE);
        event->SetMin(eventPattern.low_value);  // Lower Bank
        event->SetMax(eventPattern.high_value); // Upper Bank
        receiveEvents.push_back(event);
        break;
      case MIDI_M_SYSEX_VISCOUNT:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::VISCOUNT_SYSEX_TYPE);
        event->SetNumber(eventPattern.high_value);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::VISCOUNT_SYSEX_TYPE);
        event2->SetNumber(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_SYSEX_VISCOUNT_TOGGLE:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::VISCOUNT_SYSEX_TYPE);
        event->SetNumber(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_SYSEX_RODGERS_STOP_CHANGE:
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_OFF_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::VISCOUNT_SYSEX_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_SYSEX_AHLBORN_GALANTI:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(
          GOMidiExportImportEvent::AHLBORN_GALANTI_SYSEX_TYPE);
        event->SetNumber(eventPattern.high_value);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(
          GOMidiExportImportEvent::AHLBORN_GALANTI_SYSEX_TYPE);
        event2->SetNumber(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE:
        // two events
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(
          GOMidiExportImportEvent::AHLBORN_GALANTI_SYSEX_TYPE);
        event->SetNumber(eventPattern.high_value);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(
          GOMidiExportImportEvent::AHLBORN_GALANTI_SYSEX_TYPE);
        event2->SetNumber(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_CTRL_BIT:
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_OFF_FUNCTION);
        event->SetEventType(
          GOMidiExportImportEvent::CONTENT_CONTROL_CHANGE_BITFIELD_TYPE);
        event->SetNumber(eventPattern.low_value);
        event->SetMin(eventPattern.key); // bit
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_FIXED:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMinAndMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_RPN:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_NRPN:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_RPN_RANGE:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.high_value);
        event->SetMinAndMax(eventPattern.key);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event2->SetNumber(eventPattern.low_value);
        event2->SetMinAndMax(eventPattern.key);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_NRPN_RANGE:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::ON_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.high_value);
        event->SetMinAndMax(eventPattern.key);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::OFF_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event2->SetNumber(eventPattern.low_value);
        event2->SetMinAndMax(eventPattern.key);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_NOTE_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMin(eventPattern.high_value); // velocity
        event->SetMax(127);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMin(0); // velocity
        event->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_ON_OFF:
        // two events, on and off
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMin(eventPattern.high_value); // velocity
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event2->SetLowAndHigh(eventPattern.key);
        event2->SetMin(0); // velocity
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_NOTE_FIXED_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value); // velocity
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_FIXED_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetLowAndHigh(eventPattern.key);
        event->SetMinAndMax(eventPattern.low_value); // velocity
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(0);
        event->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_ON_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_CTRL_CHANGE_FIXED_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_FIXED_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_CTRL_CHANGE_FIXED_ON_OFF:
        // two events
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMinAndMax(eventPattern.high_value);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMinAndMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_RPN_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_RPN_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(0);
        event->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_RPN_ON_OFF:
        // two events
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      case MIDI_M_NRPN_ON:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NRPN_OFF:
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(0);
        event->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NRPN_ON_OFF:
        // two events
        event->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.high_value);
        event->SetMax(127);
        receiveEvents.push_back(event);
        event2->SetReceiveFunction(GOMidiExportImportEvent::TOGGLE_FUNCTION);
        event2->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        event2->SetNumber(eventPattern.key);
        event2->SetMin(0);
        event2->SetMax(eventPattern.low_value);
        receiveEvents.push_back(event2);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
}

void GOMidiExportImportEventsConverter::EventsFromLabel(
  GOMidiConfigurator *object,
  GOMidiExportImportEventVector &sendEvents,
  GOMidiExportImportEventVector &receiveEvents) {
  const GOMidiSender *midiSender = object->GetMidiSender();
  if (midiSender) {
    for (unsigned i = 0; i < midiSender->GetEventCount(); i++) {
      const GOMidiSenderEventPattern &eventPattern = midiSender->GetEvent(i);
      switch (eventPattern.type) {
      case MIDI_S_NONE:
        break;
      case MIDI_S_HW_NAME_STRING:
      case MIDI_S_HW_NAME_LCD:
      case MIDI_S_HW_STRING:
      case MIDI_S_HW_LCD:
        EventsFromMidiHWEvent(eventPattern, sendEvents);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
}

void GOMidiExportImportEventsConverter::EventsFromEnclosure(
  GOMidiConfigurator *object,
  GOMidiExportImportEventVector &sendEvents,
  GOMidiExportImportEventVector &receiveEvents) {
  const GOMidiReceiverBase *midiReceiver = object->GetMidiReceiver();
  const GOMidiSender *midiSender = object->GetMidiSender();

  auto CopyValueEventParams
    = [&](GOMidiEventPattern eventPattern, GOMidiExportImportEventPtr event) {
        event->SetNumber(eventPattern.key);
        event->SetMin(eventPattern.low_value);
        event->SetMax(eventPattern.high_value);
      };

  if (midiSender) {
    for (unsigned i = 0; i < midiSender->GetEventCount(); i++) {
      const GOMidiSenderEventPattern &eventPattern = midiSender->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      event->SetSendFunction(GOMidiExportImportEvent::VALUE_FUNCTION);
      switch (eventPattern.type) {
      case MIDI_S_NONE:
        break;
      case MIDI_S_CTRL:
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        CopyValueEventParams(eventPattern, event);
        sendEvents.push_back(event);
        break;
      case MIDI_S_RPN:
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        CopyValueEventParams(eventPattern, event);
        sendEvents.push_back(event);
        break;
      case MIDI_S_NRPN:
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        CopyValueEventParams(eventPattern, event);
        sendEvents.push_back(event);
        break;
      case MIDI_S_PGM_RANGE:
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLow(eventPattern.low_value >> 7);
        event->SetHigh(eventPattern.high_value >> 7);
        event->SetMin(eventPattern.low_value & 0x7F);
        event->SetMax(eventPattern.high_value & 0x7F);
        sendEvents.push_back(event);
        break;
      case MIDI_S_HW_NAME_STRING:
      case MIDI_S_HW_NAME_LCD:
      case MIDI_S_HW_STRING:
      case MIDI_S_HW_LCD:
        EventsFromMidiHWEvent(eventPattern, sendEvents);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
  if (midiReceiver) {
    for (unsigned i = 0; i < midiReceiver->GetEventCount(); i++) {
      const GOMidiReceiverEventPattern &eventPattern
        = midiReceiver->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      event->SetDebounceTime(eventPattern.debounce_time);
      event->SetReceiveFunction(GOMidiExportImportEvent::VALUE_FUNCTION);
      switch (eventPattern.type) {
      case MIDI_M_NONE:
        break;
      case MIDI_M_CTRL_CHANGE:
        event->SetEventType(GOMidiExportImportEvent::CONTROL_CHANGE_TYPE);
        CopyValueEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_RPN:
        event->SetEventType(GOMidiExportImportEvent::RPN_TYPE);
        CopyValueEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NRPN:
        event->SetEventType(GOMidiExportImportEvent::NRPN_TYPE);
        CopyValueEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_PGM_RANGE:
        event->SetEventType(GOMidiExportImportEvent::PROGRAM_CHANGE_TYPE);
        event->SetLow(eventPattern.low_value >> 7);
        event->SetHigh(eventPattern.high_value >> 7);
        event->SetMin(eventPattern.low_value & 0x7F);
        event->SetMax(eventPattern.high_value & 0x7F);
        receiveEvents.push_back(event);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
}

void GOMidiExportImportEventsConverter::EventsFromManualOrRank(
  GOMidiConfigurator *object,
  GOMidiExportImportEventVector &sendEvents,
  GOMidiExportImportEventVector &receiveEvents) {
  const GOMidiSender *midiSender = object->GetMidiSender();

  auto CopyKeyPressEventParams = [&](
                                   GOMidiReceiverEventPattern eventPattern,
                                   GOMidiExportImportEventPtr event) {
    event->SetNumber(eventPattern.key);  // transpose
    event->SetLow(eventPattern.low_key); // key
    event->SetHigh(eventPattern.high_key);
    event->SetMin(eventPattern.low_value); // velocity
    event->SetMax(eventPattern.high_value);
  };

  if (midiSender) {
    for (unsigned i = 0; i < midiSender->GetEventCount(); i++) {
      const GOMidiSenderEventPattern &eventPattern = midiSender->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      if (eventPattern.useNoteOff) {
        // two events, on and off
        // TODO: divisional note off event
        event->SetSendFunction(GOMidiExportImportEvent::ON_FUNCTION);
        switch (eventPattern.type) {
        case MIDI_S_NONE:
          break;
        case MIDI_S_NOTE:
          event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
          event->SetMax(eventPattern.high_value); // velocity
          sendEvents.push_back(event);
          break;
        case MIDI_S_NOTE_NO_VELOCITY:
          event->SetEventType(
            GOMidiExportImportEvent::NOTE_ON_NO_VELOCITY_TYPE);
          event->SetMax(eventPattern.high_value); // velocity
          sendEvents.push_back(event);
          break;
        case MIDI_S_HW_NAME_STRING:
        case MIDI_S_HW_NAME_LCD:
          EventsFromMidiHWEvent(eventPattern, sendEvents);
          break;
        default:
          // TODO: error
          break;
        }
        if (
          eventPattern.type == MIDI_S_NOTE
          || eventPattern.type == MIDI_S_NOTE_NO_VELOCITY) {
          // note off
          GOMidiExportImportEventPtr event
            = std::make_shared<GOMidiExportImportEvent>();
          event->SetMidiDeviceID(eventPattern.deviceId);
          event->SetMidiChannel(eventPattern.channel);
          event->SetSendFunction(GOMidiExportImportEvent::OFF_FUNCTION);
          event->SetEventType(GOMidiExportImportEvent::NOTE_OFF_TYPE);
          event->SetMax(0); // velocity
          sendEvents.push_back(event);
        }
      } else {
        event->SetSendFunction(GOMidiExportImportEvent::KEY_PRESS_FUNCTION);
        switch (eventPattern.type) {
        case MIDI_S_NONE:
          break;
        case MIDI_S_NOTE:
          event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
          event->SetMin(eventPattern.low_value); // velocity
          event->SetMax(eventPattern.high_value);
          sendEvents.push_back(event);
          break;
        case MIDI_S_NOTE_NO_VELOCITY:
          event->SetEventType(
            GOMidiExportImportEvent::NOTE_ON_NO_VELOCITY_TYPE);
          event->SetMin(eventPattern.low_value); // velocity
          event->SetMax(eventPattern.high_value);
          sendEvents.push_back(event);
          break;
        case MIDI_S_HW_NAME_STRING:
        case MIDI_S_HW_NAME_LCD:
          EventsFromMidiHWEvent(eventPattern, sendEvents);
          break;
        default:
          // TODO: error
          break;
        }
      }
    }
  }
  const GOMidiSender *midiDivisionSender = object->GetDivision();
  if (midiDivisionSender) {
    for (unsigned i = 0; i < midiDivisionSender->GetEventCount(); i++) {
      const GOMidiSenderEventPattern &eventPattern
        = midiDivisionSender->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      event->SetSendFunction(
        GOMidiExportImportEvent::DIVISION_KEY_PRESS_FUNCTION);
      switch (eventPattern.type) {
      case MIDI_S_NONE:
        break;
      case MIDI_S_NOTE:
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        event->SetMin(eventPattern.low_value); // velocity
        event->SetMax(eventPattern.high_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_NOTE_NO_VELOCITY:
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_NO_VELOCITY_TYPE);
        event->SetMin(eventPattern.low_value); // velocity
        event->SetMax(eventPattern.high_value);
        sendEvents.push_back(event);
        break;
      case MIDI_S_HW_NAME_STRING:
      case MIDI_S_HW_NAME_LCD:
        EventsFromMidiHWEvent(eventPattern, sendEvents);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
  const GOMidiReceiverBase *midiReceiver = object->GetMidiReceiver();
  if (midiReceiver) {
    for (unsigned i = 0; i < midiReceiver->GetEventCount(); i++) {
      const GOMidiReceiverEventPattern &eventPattern
        = midiReceiver->GetEvent(i);
      GOMidiExportImportEventPtr event
        = std::make_shared<GOMidiExportImportEvent>();
      event->SetMidiDeviceID(eventPattern.deviceId);
      event->SetMidiChannel(eventPattern.channel);
      switch (eventPattern.type) {
      case MIDI_M_NONE:
        break;
      case MIDI_M_NOTE:
        event->SetReceiveFunction(GOMidiExportImportEvent::KEY_PRESS_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        CopyKeyPressEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_NO_VELOCITY:
        event->SetReceiveFunction(GOMidiExportImportEvent::KEY_PRESS_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_NO_VELOCITY_TYPE);
        CopyKeyPressEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_SHORT_OCTAVE:
        event->SetReceiveFunction(
          GOMidiExportImportEvent::KEY_PRESS_SHORT_OCTAVE_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        CopyKeyPressEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      case MIDI_M_NOTE_NORMAL:
        event->SetReceiveFunction(
          GOMidiExportImportEvent::KEY_PRESS_NO_MAP_FUNCTION);
        event->SetEventType(GOMidiExportImportEvent::NOTE_ON_TYPE);
        CopyKeyPressEventParams(eventPattern, event);
        receiveEvents.push_back(event);
        break;
      default:
        // TODO: error
        break;
      }
    }
  }
}

void GOMidiExportImportEventsConverter::EventsFromMidiHWEvent(
  const GOMidiSenderEventPattern &eventPattern,
  GOMidiExportImportEventVector &sendEvents) {
  GOMidiExportImportEventPtr event
    = std::make_shared<GOMidiExportImportEvent>();
  event->SetMidiDeviceID(eventPattern.deviceId);
  event->SetMidiChannel(eventPattern.channel);
  event->SetEventType(GOMidiExportImportEvent::LCD_SYSEX_TYPE);
  switch (eventPattern.type) {
  case MIDI_S_NONE:
    break;
  case MIDI_S_HW_NAME_STRING:
    event->SetSendFunction(GOMidiExportImportEvent::NAME_FUNCTION);
    event->SetNumber(eventPattern.key); // ID
    event->SetLow(eventPattern.start);
    event->SetHigh(eventPattern.start + eventPattern.length - 1);
    event->SetMax(16);
    sendEvents.push_back(event);
    break;
  case MIDI_S_HW_NAME_LCD:
    event->SetSendFunction(GOMidiExportImportEvent::NAME_FUNCTION);
    event->SetNumber(eventPattern.key);    // ID
    event->SetMin(eventPattern.low_value); // color
    event->SetLow(eventPattern.start);
    event->SetHigh(eventPattern.start + eventPattern.length - 1);
    event->SetMax(32);
    sendEvents.push_back(event);
    break;
  case MIDI_S_HW_STRING:
    event->SetSendFunction(GOMidiExportImportEvent::VALUE_FUNCTION);
    event->SetNumber(eventPattern.key); // ID
    event->SetLow(eventPattern.start);
    event->SetHigh(eventPattern.start + eventPattern.length - 1);
    event->SetMax(16);
    sendEvents.push_back(event);
    break;
  case MIDI_S_HW_LCD:
    event->SetSendFunction(GOMidiExportImportEvent::VALUE_FUNCTION);
    event->SetNumber(eventPattern.key);    // ID
    event->SetMin(eventPattern.low_value); // color
    event->SetLow(eventPattern.start);
    event->SetHigh(eventPattern.start + eventPattern.length - 1);
    event->SetMax(32);
    sendEvents.push_back(event);
    break;
  default:
    // TODO: error
    break;
  }
}

void GOMidiExportImportEventsConverter::EventsToObject(
  GOMidiConfigurator *object,
  GOMidiExportImportObjectWrapper::ObjectType objectType,
  const GOMidiExportImportEventVector &events) {}
