/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMidiExportImportEventsConverter_H
#define GOMidiExportImportEventsConverter_H

#include "GOMidiExportImportEvent.h"

#include "GOMidiExportImportObjectWrapper.h"

class GOMidiConfigurator;
struct GOMidiSenderEventPattern;

class GOMidiExportImportEventsConverter {
private:
  static void EventsFromButton(
    GOMidiConfigurator *object,
    GOMidiExportImportEventVector &sendEvents,
    GOMidiExportImportEventVector &receiveEvents);
  static void EventsFromLabel(
    GOMidiConfigurator *object,
    GOMidiExportImportEventVector &sendEvents,
    GOMidiExportImportEventVector &receiveEvents);
  static void EventsFromEnclosure(
    GOMidiConfigurator *object,
    GOMidiExportImportEventVector &sendEvents,
    GOMidiExportImportEventVector &receiveEvents);
  static void EventsFromManualOrRank(
    GOMidiConfigurator *object,
    GOMidiExportImportEventVector &sendEvents,
    GOMidiExportImportEventVector &receiveEvents);
  static void EventsFromMidiHWEvent(
    const GOMidiSenderEventPattern &event,
    GOMidiExportImportEventVector &sendEvents);
  static void MergeEvents(
    GOMidiExportImportEventVector &sendEvents,
    GOMidiExportImportEventVector &receiveEvents,
    GOMidiExportImportEventVector &events);

public:
  static void EventsFromObject(
    GOMidiConfigurator *object,
    GOMidiExportImportObjectWrapper::ObjectType objectType,
    GOMidiExportImportEventVector &events);
  static void EventsToObject(
    GOMidiConfigurator *object,
    GOMidiExportImportObjectWrapper::ObjectType objectType,
    const GOMidiExportImportEventVector &events);
};

#endif /* GOMidiExportImportEventsConverter_h */
