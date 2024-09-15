/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiExportImportDialog.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include "GOEvent.h"
#include "GOOrganController.h"
#include "combinations/control/GODivisionalButtonControl.h"
#include "gui/GOGUIControl.h"
#include "gui/GOGUIPanel.h"
#include "midi/GOMidiConfigurator.h"
#include "model/GOCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

#include "GOMidiExportImportEventsConverter.h"
#include "GOMidiExportImportNode.h"

BEGIN_EVENT_TABLE(GOMidiExportImportDialog, GOSimpleDialog)
/*
EVT_LIST_ITEM_SELECTED(ID_LIST, GOMidiExportImportDialog::OnObjectClick)
EVT_LIST_ITEM_ACTIVATED(ID_LIST, GOMidiExportImportDialog::OnObjectDoubleClick)
EVT_BUTTON(ID_STATUS, GOMidiExportImportDialog::OnStatus)
EVT_BUTTON(ID_EDIT, GOMidiExportImportDialog::OnEdit)
EVT_COMMAND_RANGE(
        ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON,
GOMidiExportImportDialog::OnButton)
*/
// TODO: events
EVT_DATAVIEW_SELECTION_CHANGED(
  ID_TREE, GOMidiExportImportDialog::OnSelectionChanged)
EVT_DATAVIEW_ITEM_BEGIN_DRAG(ID_TREE, GOMidiExportImportDialog::OnItemBeginDrag)
EVT_DATAVIEW_ITEM_DROP_POSSIBLE(
  ID_TREE, GOMidiExportImportDialog::OnItemDropPossible)
EVT_DATAVIEW_ITEM_DROP(ID_TREE, GOMidiExportImportDialog::OnItemDrop)
END_EVENT_TABLE()

GOMidiExportImportDialog::GOMidiExportImportDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  GODialogSizeSet &dialogSizes,
  GOOrganModel &organModel,
  GOOrganController &organController)
  : GOSimpleDialog(
      parent,
      wxT("MIDI Objects Export/Import"),
      _("MIDI Objects Export/Import"),
      dialogSizes,
      wxEmptyString,
      0,
      wxAPPLY | wxCANCEL | wxHELP),
    GOView(doc, this),
    r_organModel(organModel),
    r_organController(organController),
    m_drag_drop_dataFormat(wxDataFormat(wxT("GOMidiExportImportDialog"))) {

  CollectObjects();
  Group(GROUP_BY_PANEL_DIVISION);

  // views
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  /*
  m_TreeView = new wxDataViewCtrl(this, ID_TREE,
          wxDefaultPosition, wxSize(500, 300),
          wxDV_MULTIPLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);
  */
  m_TreeView = new wxDataViewCtrl(
    this,
    ID_TREE,
    wxDefaultPosition,
    wxSize(500, 300),
    wxDV_MULTIPLE | wxDV_ROW_LINES);

  bool dataViewModelOK = m_TreeView->AssociateModel(&m_data_model);
  // columns in order and don't skip any
  AddTextColumn(GOMidiExportImportNode::NAME_COLUMN, _("Name"));
  AddTextColumn(GOMidiExportImportNode::TYPE_COLUMN, _("Type"));
  AddTextColumn(GOMidiExportImportNode::DIVISION_COLUMN, _("Division"));
  AddTextColumn(GOMidiExportImportNode::PANEL_COLUMN, _("Panel"));
  AddTextColumn(GOMidiExportImportNode::ODF_SECTION_COLUMN, _("ODF Section"));
  AddTextColumn(GOMidiExportImportNode::RECEIVE_FUNCTION_COLUMN, _("Receive"));
  AddTextColumn(GOMidiExportImportNode::SEND_FUNCTION_COLUMN, _("Send"));
  AddTextColumn(GOMidiExportImportNode::MIDI_DEVICE_COLUMN, _("Device"));
  AddIntColumn(GOMidiExportImportNode::MIDI_CHANNEL_COLUMN, _("Ch."));
  AddTextColumn(GOMidiExportImportNode::MIDI_EVENT_TYPE_COLUMN, _("Event"));
  AddIntColumn(GOMidiExportImportNode::LOW_COLUMN, _("Low"));
  AddIntColumn(GOMidiExportImportNode::HIGH_COLUMN, _("High"));
  AddIntColumn(GOMidiExportImportNode::NUMBER_COLUMN, _("No."));
  AddIntColumn(GOMidiExportImportNode::MIN_COLUMN, _("Min"));
  AddIntColumn(GOMidiExportImportNode::MAX_COLUMN, _("Max"));
  AddIntColumn(GOMidiExportImportNode::DEBOUNCE_TIME_COLUMN, _("Deb."));

  bool canDrop = m_TreeView->EnableDropTarget(m_drag_drop_dataFormat);

  topSizer->Add(m_TreeView, 1, wxEXPAND | wxALL, 5);

  /*
  m_EventPanel = new GOMidiExportImportEventPanel();
  topSizer->Add(m_EventPanel, 1, wxEXPAND | wxLeft | wxRight, 5);
  */

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  for (unsigned id = ID_BUTTON; id <= ID_BUTTON_LAST; id++) {
    wxButton *button = new wxButton(this, id, wxEmptyString);
    sizer->Add(button, 0, id == ID_BUTTON ? wxRESERVE_SPACE_EVEN_IF_HIDDEN : 0);
    button->Hide();
    m_Buttons.push_back(button);
  }
  topSizer->Add(sizer, 0, wxALIGN_LEFT | wxALL, 1);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  m_Edit = new wxButton(this, ID_EDIT, _("C&onfigure..."));
  m_Edit->Disable();
  buttons->Add(m_Edit);
  m_Status = new wxButton(this, ID_STATUS, _("&Status"));
  m_Status->Disable();
  buttons->Add(m_Status);
  topSizer->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 1);

  topSizer->AddSpacer(5);
  LayoutWithInnerSizer(topSizer);
  /* werkt niet
  m_TreeView->ExpandAncestors(wxDataViewItem());
  */
}

GOMidiExportImportDialog::~GOMidiExportImportDialog() {
  bool dataViewModelOK = m_TreeView->AssociateModel(NULL);
}

void GOMidiExportImportDialog::AddTextColumn(
  GOMidiExportImportNode::Column column, wxString title) {
  wxDataViewTextRenderer *renderer = new wxDataViewTextRenderer(
    wxDataViewTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_INERT,
    wxDVR_DEFAULT_ALIGNMENT);
  wxDataViewColumn *col = new wxDataViewColumn(
    title,
    renderer,
    column,
    wxDVC_DEFAULT_WIDTH,
    wxALIGN_LEFT,
    wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
  bool insertOK = m_TreeView->wxDataViewCtrl::InsertColumn(column, col);
}

void GOMidiExportImportDialog::AddIntColumn(
  GOMidiExportImportNode::Column column, wxString title) {
  wxDataViewTextRenderer *renderer = new wxDataViewTextRenderer(
    wxT("long"), wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
  wxDataViewColumn *col = new wxDataViewColumn(
    title,
    renderer,
    column,
    55,
    wxALIGN_RIGHT,
    wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
  bool insertOK = m_TreeView->wxDataViewCtrl::InsertColumn(column, col);
}

void GOMidiExportImportDialog::CollectObjects() {
  const std::vector<GOMidiConfigurator *> &midiObjects
    = r_organModel.GetMidiConfigurators();
  for (unsigned i = 0; i < midiObjects.size(); i++) {
    GOMidiConfigurator *midiObject = midiObjects[i];
    GOMidiExportImportObjectWrapperPtr object
      = std::make_shared<GOMidiExportImportObjectWrapper>(midiObject);
    m_objects.push_back(object);
    GOMidiExportImportEventVector events;
    GOMidiExportImportEventsConverter::EventsFromObject(
      midiObject, object->GetObjectType(), events);
    if (events.size() > 0)
      object->AddEvents(events);
  }

  // Find  division of objects
  for (unsigned i = r_organModel.GetFirstManualIndex();
       i <= r_organModel.GetManualAndPedalCount();
       i++) {
    GOManual *division = r_organModel.GetManual(i);
    m_divisions.push_back(division);
    for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
      if (object->GetObject() == division)
        object->SetDivision(division);
    }
    for (unsigned j = 0; j < division->GetCouplerCount(); j++) {
      const GOCoupler *coupler = division->GetCoupler(j);
      for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
        if (object->GetObject() == coupler)
          object->SetDivision(division);
      }
    }
    for (unsigned j = 0; j < division->GetDivisionalCount(); j++) {
      const GODivisionalButtonControl *divisional = division->GetDivisional(j);
      for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
        if (object->GetObject() == divisional)
          object->SetDivision(division);
      }
    }
    for (unsigned j = 0; j < division->GetStopCount(); j++) {
      const GOStop *stop = division->GetStop(j);
      for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
        if (object->GetObject() == stop)
          object->SetDivision(division);
      }
    }
    for (unsigned j = 0; j < division->GetSwitchCount(); j++) {
      const GOSwitch *localSwitch = division->GetSwitch(j);
      for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
        if (object->GetObject() == localSwitch)
          object->SetDivision(division);
      }
    }
    for (unsigned j = 0; j < division->GetTremulantCount(); j++) {
      const GOTremulant *localTremulant = division->GetTremulant(j);
      for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
        if (object->GetObject() == localTremulant)
          object->SetDivision(division);
      }
    }
  }

  // Find panels of objects
  for (unsigned i = 0; i < r_organController.GetPanelCount(); i++) {
    GOGUIPanel *panel = r_organController.GetPanel(i);
    m_panels.push_back(panel);
    const ptr_vector<GOGUIControl> &guiControls = panel->GetControls();
    for (GOGUIControl *guiControl : guiControls) {
      const void *control = guiControl->GetControl();
      if (control) {
        const GOMidiConfigurator *controlObject
          = dynamic_cast<GOMidiConfigurator *>((GOControl *)control);
        if (controlObject) {
          for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
            const GOMidiConfigurator *midiObject = object->GetObject();
            if (midiObject == controlObject)
              object->AddPanel(panel);
          }
        }
      }
    }
  }
}

GOMidiExportImportOptionalNodePtr GOMidiExportImportDialog::NodeWithObjectNodes(
  const wxString name,
  bool groupByPanel,
  const GOGUIPanel *panel,
  bool groupByDivision,
  const GOManual *division) const {
  GOMidiExportImportObjectNodeVector objectNodes;
  for (GOMidiExportImportObjectWrapperPtr object : m_objects) {
    if ((!groupByDivision || object->IsDivision(division)) &&
			(!groupByPanel || (panel && object->IsOnPanel(panel)) || (!panel && object->IsNotOnAnyPanel()))) {
      GOMidiExportImportObjectNodePtr objectNode
        = std::make_shared<GOMidiExportImportObjectNode>(object);
      objectNodes.push_back(objectNode);
    }
  }
  if (objectNodes.size() > 0) {
    GOMidiExportImportNodePtr node
      = std::make_shared<GOMidiExportImportNode>(name);
    for (GOMidiExportImportNodePtr objectNode : objectNodes) {
      node->AddChildNode(objectNode);
    }
    return node;
  }
  return std::nullopt;
}

void GOMidiExportImportDialog::EnumeratePanelsAndNULL(
  std::function<void(GOGUIPanel *)> block) {
  for (GOGUIPanel *panel : m_panels)
    block(panel);
  block(NULL);
}

void GOMidiExportImportDialog::EnumerateDivisionsAndNULL(
  std::function<void(GOManual *)> block) {
  for (GOManual *division : m_divisions)
    block(division);
  block(NULL);
}

void GOMidiExportImportDialog::Group(GroupBy groupBy) {
  switch (groupBy) {
  case GROUP_BY_NONE: {
    GOMidiExportImportOptionalNodePtr optionalNode
      = NodeWithObjectNodes(wxT(""), false, NULL, false, NULL);
    if (optionalNode) {
      GOMidiExportImportNodePtr *node = std::addressof(*optionalNode);
      std::optional<GOMidiExportImportNodeVector> childNodes
        = (*node)->GetChildren();
      if (childNodes) {
        m_data_model.SetRootNodes(*childNodes);
        break;
      }
    }
    // no MIDI objects, this shouldn't happen
    GOMidiExportImportNodeVector nodes;
    m_data_model.SetRootNodes(nodes);
    break;
  }
  case GROUP_BY_PANEL: {
    // group by panel and division
    GOMidiExportImportNodeVector panelNodes;
    EnumeratePanelsAndNULL([this, &panelNodes](GOGUIPanel *panel) {
      GOMidiExportImportOptionalNodePtr optionalNode = NodeWithObjectNodes(
        panel ? panel->GetName() : _("No Panel"), true, panel, false, NULL);
      if (optionalNode) {
        GOMidiExportImportNodePtr *node = std::addressof(*optionalNode);
        panelNodes.push_back(*node);
      }
    });
    m_data_model.SetRootNodes(panelNodes);
    break;
  }
  case GROUP_BY_DIVISION: {
    // group by division
    GOMidiExportImportNodeVector divisionNodes;
    EnumerateDivisionsAndNULL([this, &divisionNodes](GOManual *division) {
      GOMidiExportImportOptionalNodePtr optionalNode = NodeWithObjectNodes(
        division ? division->GetName() : _("Organ"),
        false,
        NULL,
        true,
        division);
      if (optionalNode) {
        GOMidiExportImportNodePtr *node = std::addressof(*optionalNode);
        divisionNodes.push_back(*node);
      }
    });
    m_data_model.SetRootNodes(divisionNodes);
    break;
  }
  case GROUP_BY_PANEL_DIVISION: {
    // group by panel and division
    GOMidiExportImportNodeVector panelNodes;
    EnumeratePanelsAndNULL([this, &panelNodes](GOGUIPanel *panel) {
      GOMidiExportImportNodeVector divisionNodes;
      EnumerateDivisionsAndNULL(
        [this, &divisionNodes, panel](GOManual *division) {
          GOMidiExportImportOptionalNodePtr optionalNode = NodeWithObjectNodes(
            division ? division->GetName() : _("Organ"),
            true,
            panel,
            true,
            division);
          if (optionalNode) {
            GOMidiExportImportNodePtr *node = std::addressof(*optionalNode);
            divisionNodes.push_back(*node);
          }
        });
      if (divisionNodes.size() > 0) {
        GOMidiExportImportNodePtr panelNode
          = std::make_shared<GOMidiExportImportNode>(
            panel ? panel->GetName() : _("No Panel"));
        panelNodes.push_back(panelNode);
        for (GOMidiExportImportNodePtr divisionNode : divisionNodes) {
          panelNode->AddChildNode(divisionNode);
        }
      }
    });
    m_data_model.SetRootNodes(panelNodes);
    break;
  }
  }
}

/*
void GOMidiExportImportDialog::OnButton(wxCommandEvent &event) {
        GOMidiConfigurator *obj = (GOMidiConfigurator *)m_TreeView->GetItemData(
                m_TreeView->GetFirstSelected());
        obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}

void GOMidiExportImportDialog::OnStatus(wxCommandEvent &event) {
        GOMidiConfigurator *obj = (GOMidiConfigurator *)m_TreeView->GetItemData(
                m_TreeView->GetFirstSelected());
        wxString status = obj->GetElementStatus();
        GOMessageBox(wxString::Format(_("Status: %s"), status),
                obj->GetMidiType() + _(" ") + obj->GetMidiName(), wxOK);
}

void GOMidiExportImportDialog::OnObjectClick(wxListEvent &event) {
        m_Edit->Enable();
        m_Status->Enable();
        GOMidiConfigurator *obj = (GOMidiConfigurator *)m_TreeView->GetItemData(
                m_TreeView->GetFirstSelected());
        std::vector<wxString> actions = obj->GetElementActions();
        for (unsigned i = 0; i < m_Buttons.size(); i++)
                if (i < actions.size()) {
                        m_Buttons[i]->SetLabel(actions[i]);
                        m_Buttons[i]->Show();
                } else
                        m_Buttons[i]->Hide();
        Layout();
}

void GOMidiExportImportDialog::OnObjectDoubleClick(wxListEvent &event) {
        GOMidiConfigurator *obj = (GOMidiConfigurator *)m_TreeView->GetItemData(
                m_TreeView->GetFirstSelected());
        obj->ShowConfigDialog();
}

void GOMidiExportImportDialog::OnEdit(wxCommandEvent &event) {
        wxListEvent listevent;
        OnObjectDoubleClick(listevent);
}
*/

void GOMidiExportImportDialog::OnSelectionChanged(wxDataViewEvent &event) {
  wxDataViewItem item = event.GetItem();
  if (item.IsOk()) {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    wxString s = node->GetName();
    printf("OnSelectionChanged %s\n", static_cast<const char *>(s.c_str()));
  } else
    printf("OnSelectionChanged\n");
}

void GOMidiExportImportDialog::OnItemBeginDrag(wxDataViewEvent &event) {
  printf("OnItemBeginDrag\n");
  wxDataViewItem draggedItem = event.GetItem();
  if (!draggedItem.IsOk()) {
    return;
  }
  bool moveAllowed = false;
  wxString draggedText = wxT("");
  GOMidiExportImportEventNode **draggedNodes = NULL;
  int draggedNodesCount = 0;
  if (m_TreeView->IsSelected(draggedItem)) {
    // drag selected items
    // if multiple items are selected then check if they are all event nodes of
    // the same parent
    wxDataViewItemArray selectedItems;
    draggedNodesCount = m_TreeView->GetSelections(selectedItems);
    if (draggedNodesCount > 0) {
      draggedNodes = new GOMidiExportImportEventNode *[draggedNodesCount];
      moveAllowed = true;
      wxDataViewItem firstItem = selectedItems[0];
      GOMidiExportImportEventNode *firstNode
        = (GOMidiExportImportEventNode *)(firstItem.GetID());
      draggedNodes[0] = firstNode;
      if (firstNode) {
        draggedText += firstNode->GetName();
        wxDataViewItem parentItem = m_data_model.GetParent(firstItem);
        for (unsigned int i = 1; i < draggedNodesCount; i++) {
          wxDataViewItem item = selectedItems[i];
          GOMidiExportImportEventNode *node
            = (GOMidiExportImportEventNode *)(item.GetID());
          draggedNodes[i] = node;
          if (node) {
            draggedText += wxT("\n");
            draggedText += node->GetName();
          } else if (m_data_model.GetParent(item) != parentItem) {
            moveAllowed = false;
          }
        }
      }
    }
  } else {
    // drag dragged item
    GOMidiExportImportEventNode *node
      = (GOMidiExportImportEventNode *)(draggedItem.GetID());
    if (node) {
      wxString name = node->GetName();
      printf("OnItemBeginDrag %s\n", static_cast<const char *>(name.c_str()));
      draggedText += name;
      draggedNodesCount = 1;
      draggedNodes = new GOMidiExportImportEventNode *[1];
      draggedNodes[0] = node;
      moveAllowed = true;
    }
  }

  wxDataObjectComposite *dataObject = new wxDataObjectComposite();
  if (moveAllowed) {
    wxCustomDataObject *nodesDataObject
      = new wxCustomDataObject(m_drag_drop_dataFormat);
    // bool b = nodesDataObject->SetData(draggedNodesCount *
    // sizeof(GOMidiExportImportNode *), &draggedNodes);
    bool b = nodesDataObject->SetData(sizeof(draggedNodes), &draggedNodes);
    dataObject->Add(nodesDataObject, true);
  }
  dataObject->Add(new wxTextDataObject(draggedText), !moveAllowed);
  event.SetDataObject(dataObject);
  event.SetDragFlags(wxDrag_AllowMove); // allows both copy and move
  if (draggedNodes)
    delete[] draggedNodes;
}

void GOMidiExportImportDialog::OnItemDropPossible(wxDataViewEvent &event) {
  // check if the dragged items are dropped inside the same parent
  // TODO: get nodes from pasteboard, kan niet

  /* werkt niet
  wxDataObjectComposite *dataObject = (wxDataObjectComposite
  *)event.GetDataObject(); if (dataObject &&
  dataObject->IsSupported(m_drag_drop_dataFormat)) { size_t dataSize =
  dataObject->GetDataSize(m_drag_drop_dataFormat); int i = 0;
  }
  */

  wxDataViewItemArray selectedItems;
  int draggedNodesCount = m_TreeView->GetSelections(selectedItems);
  bool moveAllowed = false;
  wxDataViewItem dropItem = event.GetItem();
  if (dropItem.IsOk() && draggedNodesCount > 0) {
    wxDataViewItem firstItem = selectedItems[0];
    if (m_data_model.GetParent(firstItem) == m_data_model.GetParent(dropItem))
      moveAllowed = true;
  }

  if (dropItem.IsOk()) {
    if (m_data_model.IsContainer(dropItem)) {
      printf(
        "dropped in container (proposed index = %i)\n",
        event.GetProposedDropIndex());
    } else
      printf("dropped on item\n");
  } else
    printf(
      "dropped on background (proposed index = %i)\n",
      event.GetProposedDropIndex());

  /*
  int dropIndex = event.GetProposedDropIndex();	// index in parent
  wxDragResult result = event.GetDropEffect();
  wxDataViewItem item = event.GetItem();
  if (item.IsOk()) {
          GOMidiExportImportNode *node =
  (GOMidiExportImportNode*)(item.GetID()); wxString s = node->GetName();
          printf("OnItemDropPossible %i %i %s\n", dropIndex, result,
  static_cast<const char*>(s.c_str()));
  }
  else
          printf("OnItemDropPossible %i\n", dropIndex);

  if (event.GetDataFormat() != wxDF_UNICODETEXT)
  event.Veto();
  else
  event.SetDropEffect(wxDragMove); // check 'move' drop effect
  */
}

void GOMidiExportImportDialog::OnItemDrop(wxDataViewEvent &event) {
  int dropIndex = event.GetProposedDropIndex();
  wxDragResult result = event.GetDropEffect();
  wxDataViewItem item = event.GetItem();
  if (item.IsOk()) {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    wxString s = node->GetName();
    printf(
      "OnItemDrop %i %i %s\n",
      dropIndex,
      result,
      static_cast<const char *>(s.c_str()));
  } else
    printf("OnItemDrop %i %i\n", dropIndex, result);
}
