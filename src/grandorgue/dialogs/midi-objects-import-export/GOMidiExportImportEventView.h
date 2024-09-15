/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportEventPanel_H
#define GOMidiExportImportEventPanel_H

#include <wx/wx.h>

#include "GOMidiExportImportEvent.h"

class GOMidiExportImportEventPanel : public wxPanel {
private:
  GOOrganModel &r_events;
}

#endif /* GOMidiExportImportEventPanel_H */
