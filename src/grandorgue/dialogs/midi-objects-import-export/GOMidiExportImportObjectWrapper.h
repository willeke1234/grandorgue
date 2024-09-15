//
//  GOMidiExportImportObjectWrapper.hpp
//  GrandOrgue CLT for Indexing
//
//  Created by Gebruiker on 05/09/2024.
//

#ifndef GOMidiExportImportObjectWrapper_H
#define GOMidiExportImportObjectWrapper_H

#include <wx/wx.h>

#include "GOMidiExportImportEvent.h"

#include "midi/GOMidiConfigurator.h"

/* A wrapper around a MIDI object to hold its events
 */

class GOMidiConfigurator;
class GOManual;
class GOGUIPanel;

class GOMidiExportImportObjectWrapper;
using GOMidiExportImportObjectWrapperPtr
  = std::shared_ptr<GOMidiExportImportObjectWrapper>;
using GOMidiExportImportObjectWrapperVector
  = std::vector<GOMidiExportImportObjectWrapperPtr>;

class GOMidiExportImportObjectWrapper
  : public std::enable_shared_from_this<GOMidiExportImportObjectWrapper> {
public:
  enum ObjectType {
    BUTTON_TYPE,
    LABEL_TYPE,
    ENCLOSURE_TYPE,
    MANUAL_TYPE,
    RANK_TYPE,
    UNKNOWN_TYPE
  };

private:
  GOMidiConfigurator *r_object;
  ObjectType m_objectType;
  const GOManual *r_division;
  std::vector<GOGUIPanel *> m_panels;
  GOMidiExportImportEventVector m_events;
  ObjectType Type();

public:
  GOMidiExportImportObjectWrapper(GOMidiConfigurator *object);
  const GOMidiConfigurator *GetObject() const { return r_object; }
  const ObjectType GetObjectType() const { return m_objectType; }
  const GOManual *GetDivision() const { return r_division; }
  void SetDivision(const GOManual *division) { r_division = division; }
  bool IsDivision(const GOManual *division) const {
    return r_division == division;
  }
  void AddPanel(GOGUIPanel *panel) { m_panels.push_back(panel); }
  void AddEvents(const GOMidiExportImportEventVector &events) {
    for (GOMidiExportImportEventPtr event : events)
      m_events.push_back(event);
  }
  const std::vector<GOGUIPanel *> GetPanels() const { return m_panels; }
  bool IsOnPanel(const GOGUIPanel *panel) const {
    for (GOGUIPanel *objectPanel : m_panels) {
      if (objectPanel == panel)
        return true;
    }
    return false;
  }
  bool IsNotOnAnyPanel() const { return m_panels.size() == 0; }
  const GOMidiExportImportEventVector GetEvents() const { return m_events; }
  bool HasEvents() const { return m_events.size() > 0; }
  const wxString &GetName() const { return r_object->GetMidiName(); }
  const wxString &GetMidiName() const { return r_object->GetMidiName(); }
  const wxString &GetMidiType() const { return r_object->GetMidiType(); }
  const wxString &GetODFSection() const { return r_object->GetODFSection(); }
  const wxString GetDivisionName() const;
  const wxString GetPanelName() const;
};

#endif /* GOMidiExportImportObjectWrapper_h */
