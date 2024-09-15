//
//  GOMidiExportImportObjectWrapper.cpp
//  GrandOrgue CLT for Indexing
//
//  Created by Gebruiker on 05/09/2024.
//

#include "GOMidiExportImportObjectWrapper.h"

#include "gui/GOGUIPanel.h"
#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiReceiverBase.h"
#include "midi/GOMidiSender.h"
#include "model/GOManual.h"

GOMidiExportImportObjectWrapper::GOMidiExportImportObjectWrapper(
  GOMidiConfigurator *object)
  : r_object(object) {
  m_objectType = Type();
}

GOMidiExportImportObjectWrapper::ObjectType GOMidiExportImportObjectWrapper::
  Type() {
  /* determines the type of object based on the types of MIDI receiver, sender
   * and division
   */
  int midiReceiverType = -1;
  int midiSenderType = -1;
  int midiDivisionSenderType = -1;
  GOMidiReceiverBase *midiReceiver = r_object->GetMidiReceiver();
  if (midiReceiver) {
    midiReceiverType = midiReceiver->GetType();
  }
  const GOMidiSender *midiSender = r_object->GetMidiSender();
  if (midiSender) {
    midiSenderType = midiSender->GetType();
  }
  const GOMidiSender *midiDivisionSender = r_object->GetDivision();
  if (midiDivisionSender) {
    midiDivisionSenderType = midiDivisionSender->GetType();
  }
  switch (midiSenderType) {
  case MIDI_SEND_BUTTON:
    if (
      midiReceiverType == MIDI_RECV_DRAWSTOP
      || midiReceiverType == MIDI_RECV_BUTTON
      || midiReceiverType == MIDI_RECV_SETTER)
      return BUTTON_TYPE;
    break;
  case MIDI_SEND_LABEL:
    if (midiReceiverType == -1)
      return LABEL_TYPE;
    break;
  case MIDI_SEND_ENCLOSURE:
    if (midiReceiverType == MIDI_RECV_ENCLOSURE)
      return ENCLOSURE_TYPE;
    break;
  case MIDI_SEND_MANUAL:
    if (midiReceiverType == MIDI_RECV_MANUAL)
      return MANUAL_TYPE;
    if (midiReceiverType == -1) // no receiver
      return RANK_TYPE;
    break;
  }
  return UNKNOWN_TYPE;
}

const wxString GOMidiExportImportObjectWrapper::GetDivisionName() const {
  if (r_division)
    return r_division->GetName();
  return wxT("");
}
const wxString GOMidiExportImportObjectWrapper::GetPanelName() const {
  wxString panelName = wxT("");
  if (m_panels.size() > 0) {
    panelName += m_panels[0]->GetName();
    for (unsigned int i = 1; i < m_panels.size(); i++) {
      panelName += wxT(", ");
      panelName += m_panels[i]->GetName();
    }
  }
  return panelName;
}
