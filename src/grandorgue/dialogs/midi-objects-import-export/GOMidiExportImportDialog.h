/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportDialog_H
#define GOMidiExportImportDialog_H

#include <vector>

#include <wx/dataview.h>

#include "GOMidiExportImportDataViewModel.h"
#include "GOMidiExportImportObjectWrapper.h"
#include "dialogs/common/GOSimpleDialog.h"
#include "document-base/GOView.h"

class wxButton;
class wxListEvent;
class wxListView;

class GODocument;
class GOOrganModel;
class GOOrganController;
class GOManual;
class GOGUIPanel;

class GOMidiExportImportDialog : public GOSimpleDialog, public GOView {
private:
  GOOrganModel &r_organModel;
  GOOrganController &r_organController;
  wxDataViewCtrl *m_TreeView;
  wxButton *m_Edit;
  wxButton *m_Status;
  std::vector<wxButton *> m_Buttons;
  std::vector<GOManual *> m_divisions;
  std::vector<GOGUIPanel *> m_panels;
  GOMidiExportImportObjectWrapperVector m_objects;
  GOMidiExportImportDataViewModel m_data_model;
  const wxDataFormat m_drag_drop_dataFormat;

  enum {
    ID_TREE = 200,
    ID_EDIT,
    ID_STATUS,
    ID_BUTTON,
    ID_BUTTON_LAST = ID_BUTTON + 2,
  };

  enum GroupBy {
    GROUP_BY_NONE,
    GROUP_BY_PANEL,
    GROUP_BY_DIVISION,
    GROUP_BY_PANEL_DIVISION,
  };

  void CollectObjects();
  void AddTextColumn(GOMidiExportImportNode::Column column, wxString title);
  void AddIntColumn(GOMidiExportImportNode::Column column, wxString title);
  GOMidiExportImportOptionalNodePtr NodeWithObjectNodes(
    const wxString name,
    bool groupByPanel,
    const GOGUIPanel *panel,
    bool groupByDivision,
    const GOManual *division) const;
  void EnumeratePanelsAndNULL(std::function<void(GOGUIPanel *)> block);
  void EnumerateDivisionsAndNULL(std::function<void(GOManual *)> block);
  void Group(GroupBy groupBy);

  void OnObjectClick(wxListEvent &event);
  void OnObjectDoubleClick(wxListEvent &event);
  void OnEdit(wxCommandEvent &event);
  void OnStatus(wxCommandEvent &event);
  void OnButton(wxCommandEvent &event);

  void OnSelectionChanged(wxDataViewEvent &event);
  void OnItemBeginDrag(wxDataViewEvent &event);
  void OnItemDropPossible(wxDataViewEvent &event);
  void OnItemDrop(wxDataViewEvent &event);

public:
  GOMidiExportImportDialog(
    GODocumentBase *doc,
    wxWindow *parent,
    GODialogSizeSet &dialogSizes,
    GOOrganModel &organModel,
    GOOrganController &organController);
  ~GOMidiExportImportDialog();

  DECLARE_EVENT_TABLE()
};

#endif
