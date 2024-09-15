/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiExportImportDataViewModel.h"

void GOMidiExportImportDataViewModel::SetRootNodes(
  GOMidiExportImportNodeVector &nodes) {
  m_rootNodes.clear();
  for (GOMidiExportImportNodePtr node : nodes) {
    m_rootNodes.push_back(node);
  }
}

void GOMidiExportImportDataViewModel::SetRootNodes(
  GOMidiExportImportObjectNodeVector &nodes) {
  m_rootNodes.clear();
  for (const GOMidiExportImportObjectNodePtr node : nodes) {
    m_rootNodes.push_back(node);
  }
}

bool GOMidiExportImportDataViewModel::IsContainer(
  const wxDataViewItem &item) const {
  if (!item.IsOk()) { // root
    return true;
  } else {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    return node->IsContainer();
  }
}

wxDataViewItem GOMidiExportImportDataViewModel::GetParent(
  const wxDataViewItem &item) const {
  if (!item.IsOk()) { // root
    return wxDataViewItem();
  } else {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    GOMidiExportImportOptionalNodePtr optionalParentNode = node->GetParent();
    if (optionalParentNode) {
      GOMidiExportImportNodePtr *parentNode
        = std::addressof(*optionalParentNode);
      return wxDataViewItem((void *)parentNode->get());
    } else
      return wxDataViewItem();
  }
}

unsigned int GOMidiExportImportDataViewModel::GetChildren(
  const wxDataViewItem &item, wxDataViewItemArray &children) const {
  if (!item.IsOk()) { // root
    for (const GOMidiExportImportNodePtr childNode : m_rootNodes) {
      children.Add(wxDataViewItem((void *)childNode.get()));
    }
    return m_rootNodes.size();
  } else {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    std::optional<GOMidiExportImportNodeVector> childNodes
      = node->GetChildren();
    if (childNodes) {
      for (GOMidiExportImportNodePtr childNode : *childNodes) {
        children.Add(wxDataViewItem((void *)childNode.get()));
      }
      return childNodes->size();
    } else
      return 0;
  }
}

bool GOMidiExportImportDataViewModel::HasContainerColumns(
  const wxDataViewItem &item) const {
  if (item.IsOk()) {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    return node->HasContainerColumns();
  }
  return false;
}

void GOMidiExportImportDataViewModel::GetValue(
  wxVariant &variant, const wxDataViewItem &item, unsigned int col) const {
  if (item.IsOk()) {
    GOMidiExportImportNode *node = (GOMidiExportImportNode *)(item.GetID());
    node->GetValue(variant, col);
  }
}

bool GOMidiExportImportDataViewModel::SetValue(
  const wxVariant &variant, const wxDataViewItem &item, unsigned intcol) {
  // changing values is not supported
  return false;
}
