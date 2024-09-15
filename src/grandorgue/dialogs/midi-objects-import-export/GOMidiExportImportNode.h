/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportNode_H
#define GOMidiExportImportNode_H

#include <optional>

#include <wx/wx.h>

#include "GOMidiExportImportEvent.h"

#include "GOMidiExportImportObjectWrapper.h"

/* a tree node wrapper class representing a manual/division, panel, MIDI object
 * or MIDI event/message
 */

class GOMidiExportImportNode;
using GOMidiExportImportNodePtr = std::shared_ptr<GOMidiExportImportNode>;
using GOMidiExportImportOptionalNodePtr
  = std::optional<GOMidiExportImportNodePtr>;
using GOMidiExportImportNodeVector = std::vector<GOMidiExportImportNodePtr>;
class GOMidiExportImportObjectNode;
using GOMidiExportImportObjectNodePtr
  = std::shared_ptr<GOMidiExportImportObjectNode>;
using GOMidiExportImportObjectNodeVector
  = std::vector<GOMidiExportImportObjectNodePtr>;
class GOMidiExportImportEventNode;
using GOMidiExportImportEventNodePtr
  = std::shared_ptr<GOMidiExportImportEventNode>;
using GOMidiExportImportEventNodeVector
  = std::vector<GOMidiExportImportEventNodePtr>;
class GOMidiConfigurator;

class GOMidiExportImportNode
  : public std::enable_shared_from_this<GOMidiExportImportNode> {
public:
  enum Column {
    NAME_COLUMN = 0,
    TYPE_COLUMN,
    DIVISION_COLUMN,
    PANEL_COLUMN,
    ODF_SECTION_COLUMN,
    RECEIVE_FUNCTION_COLUMN,
    SEND_FUNCTION_COLUMN,
    MIDI_DEVICE_COLUMN,
    MIDI_CHANNEL_COLUMN,
    MIDI_EVENT_TYPE_COLUMN,
    LOW_COLUMN,
    HIGH_COLUMN,
    NUMBER_COLUMN,
    MIN_COLUMN,
    MAX_COLUMN,
    DEBOUNCE_TIME_COLUMN,
    COLUMN_COUNT
  };

private:
  wxString m_name;
  wxString m_description;
  std::weak_ptr<GOMidiExportImportNode> r_parentNode;

protected:
  GOMidiExportImportNodeVector m_childNodes;

public:
  GOMidiExportImportNode(const wxString &name = wxT(""));
  ~GOMidiExportImportNode();
  // getters and setters
  virtual wxString GetName() const { return m_name; }
  virtual void SetName(wxString name) { m_name = name; }
  virtual void SetParentNode(GOMidiExportImportNodePtr parentNode) {
    r_parentNode = parentNode;
  }
  virtual void AddChildNode(GOMidiExportImportNodePtr childNode) {
    childNode->SetParentNode(
      std::static_pointer_cast<GOMidiExportImportNode>(shared_from_this()));
    m_childNodes.push_back(childNode);
  }
  // Data View Model
  virtual bool IsContainer() const { return m_childNodes.size() > 0; }
  virtual const GOMidiExportImportOptionalNodePtr GetParent() const {
    if (auto parentNode = r_parentNode.lock())
      return parentNode;
    else
      return std::nullopt;
  }
  virtual const GOMidiExportImportNodeVector &GetChildren() {
    return m_childNodes;
  }
  virtual bool HasContainerColumns() const { return false; }
  virtual void GetValue(wxVariant &variant, unsigned int col) const;
};

class GOMidiExportImportObjectNode : public GOMidiExportImportNode {
private:
  const GOMidiExportImportObjectWrapperPtr m_object;

public:
  GOMidiExportImportObjectNode(
    const GOMidiExportImportObjectWrapperPtr m_object);
  // Data View Model
  // lazy event children
  virtual bool IsContainer() const override { return m_object->HasEvents(); }
  virtual const GOMidiExportImportNodeVector &GetChildren() override;
  virtual bool HasContainerColumns() const override { return true; }
  virtual void GetValue(wxVariant &variant, unsigned int col) const override;
};

class GOMidiExportImportEventNode : public GOMidiExportImportNode {
private:
  GOMidiExportImportEventPtr m_event;

public:
  GOMidiExportImportEventNode(GOMidiExportImportEventPtr event);
  wxString GetName() const override;
  // Data View Model
  virtual bool HasContainerColumns() const override { return true; }
  virtual void GetValue(wxVariant &variant, unsigned int col) const override;
};

#endif
