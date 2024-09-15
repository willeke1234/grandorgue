/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiExportImportNode.h"
#include "midi/GOMidiConfigurator.h"

#pragma mark GOMidiExportImportNode

GOMidiExportImportNode::GOMidiExportImportNode(const wxString &name)
  : m_name(name), m_childNodes(), r_parentNode() {}

GOMidiExportImportNode::~GOMidiExportImportNode() { // TODO: test
}

void GOMidiExportImportNode::GetValue(
  wxVariant &variant, unsigned int col) const {
  switch (col) {
  case NAME_COLUMN:
    variant = wxVariant(GetName());
    break;
  }
}

#pragma mark -
#pragma mark GOMidiExportImportObjectNode

GOMidiExportImportObjectNode::GOMidiExportImportObjectNode(
  const GOMidiExportImportObjectWrapperPtr object)
  : GOMidiExportImportNode(object->GetName()), m_object(object) {}

const GOMidiExportImportNodeVector &GOMidiExportImportObjectNode::
  GetChildren() {
  if (m_childNodes.size() == 0) {
    for (GOMidiExportImportEventPtr event : m_object->GetEvents()) {
      GOMidiExportImportEventNodePtr eventNode
        = std::make_shared<GOMidiExportImportEventNode>(event);
      AddChildNode(eventNode);
    }
  }
  return m_childNodes;
}

void GOMidiExportImportObjectNode::GetValue(
  wxVariant &variant, unsigned int col) const {
  switch (col) {
  case NAME_COLUMN:
    variant = wxVariant(m_object->GetMidiName());
    break;
  case TYPE_COLUMN:
    variant = wxVariant(m_object->GetMidiType());
    break;
  case PANEL_COLUMN:
    variant = wxVariant(m_object->GetPanelName());
    break;
  case DIVISION_COLUMN:
    variant = wxVariant(m_object->GetDivisionName());
    break;
  case ODF_SECTION_COLUMN:
    variant = wxVariant(m_object->GetODFSection());
    break;
  default:
    GOMidiExportImportNode::GetValue(variant, col);
    break;
  }
}

#pragma mark -
#pragma mark GOMidiExportImportEventNode

GOMidiExportImportEventNode::GOMidiExportImportEventNode(
  GOMidiExportImportEventPtr event)
  : GOMidiExportImportNode() {
  m_event = event;
}

wxString GOMidiExportImportEventNode::GetName() const {
  wxString name = wxT("");
  GOMidiExportImportEvent::ReceiveSendFunction receiveFunction
    = m_event->GetReceiveFunction();
  if (receiveFunction != GOMidiExportImportEvent::NO_FUNCTION)
    name = m_event->GetReceiveFunctionName();
  else
    name = wxT("-");
  name += wxT("/");
  GOMidiExportImportEvent::ReceiveSendFunction sendFunction
    = m_event->GetSendFunction();
  if (sendFunction != GOMidiExportImportEvent::NO_FUNCTION)
    name += m_event->GetSendFunctionName();
  else
    name += wxT("-");
  return name;
}

void GOMidiExportImportEventNode::GetValue(
  wxVariant &variant, unsigned int col) const {
  switch (col) {
  case RECEIVE_FUNCTION_COLUMN: {
    GOMidiExportImportEvent::ReceiveSendFunction receiveFunction
      = m_event->GetReceiveFunction();
    if (receiveFunction != GOMidiExportImportEvent::NO_FUNCTION)
      variant = wxVariant(m_event->GetReceiveFunctionName());
    break;
  }
  case SEND_FUNCTION_COLUMN: {
    GOMidiExportImportEvent::ReceiveSendFunction sendFunction
      = m_event->GetSendFunction();
    if (sendFunction != GOMidiExportImportEvent::NO_FUNCTION)
      variant = wxVariant(m_event->GetSendFunctionName());
    break;
  }
  case MIDI_DEVICE_COLUMN:
    variant = wxVariant(m_event->GetMidiDeviceName());
    break;
  case MIDI_CHANNEL_COLUMN:
    variant = wxVariant((long)m_event->GetMidiChannel());
    break;
  case MIDI_EVENT_TYPE_COLUMN:
    variant = wxVariant(m_event->GetEventTypeName());
    break;
  case LOW_COLUMN:
    variant = wxVariant((long)m_event->GetLow());
    break;
  case HIGH_COLUMN:
    variant = wxVariant((long)m_event->GetHigh());
    break;
  case NUMBER_COLUMN:
    variant = wxVariant((long)m_event->GetNumber());
    break;
  case MIN_COLUMN:
    variant = wxVariant((long)m_event->GetMin());
    break;
  case MAX_COLUMN:
    variant = wxVariant((long)m_event->GetMax());
    break;
  case DEBOUNCE_TIME_COLUMN:
    variant = wxVariant((long)m_event->GetDebounceTime());
    break;
  default:
    GOMidiExportImportNode::GetValue(variant, col);
    break;
  }
}
