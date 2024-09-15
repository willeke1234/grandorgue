/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportDataViewModel_H
#define GOMidiExportImportDataViewModel_H

#include <wx/dataview.h>

#include "GOMidiExportImportNode.h"

class GOMidiExportImportDataViewModel : public wxDataViewModel {
private:
  GOMidiExportImportNodeVector m_rootNodes;

public:
  void SetRootNodes(GOMidiExportImportNodeVector &nodes);
  void SetRootNodes(GOMidiExportImportObjectNodeVector &nodes);

  // wxDataViewModel
  bool IsContainer(const wxDataViewItem &item) const;
  wxDataViewItem GetParent(const wxDataViewItem &item) const;
  unsigned int GetChildren(
    const wxDataViewItem &item, wxDataViewItemArray &children) const;
  bool HasContainerColumns(const wxDataViewItem &item) const;
  void GetValue(
    wxVariant &variant, const wxDataViewItem &item, unsigned int col) const;
  bool SetValue(
    const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

#endif
