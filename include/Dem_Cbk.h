/**
 * @file    Dem_Cbk.h
 * @brief   DEM callback prototypes (configurable hook points).
 *
 * This header provides the *symbols* that an integrator may implement to
 * connect DEM to the rest of the stack (SW-C / BSW). The DEM module will
 * call these functions when the corresponding feature macro is enabled.
 *
 * Default implementations are provided in Dem_Cbk.c (as weak/no-op).
 */

#ifndef DEM_CBK_H
#define DEM_CBK_H

#include "Dem_Types.h"

/* =========================================================================
 * Callback enable switches
 * ========================================================================= */
#ifndef DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED
#define DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED       STD_ON
#endif

#ifndef DEM_CB_ENABLE_MONITOR_STATUS_CHANGED
#define DEM_CB_ENABLE_MONITOR_STATUS_CHANGED         STD_ON
#endif

#ifndef DEM_CB_ENABLE_EVENT_DATA_CHANGED
#define DEM_CB_ENABLE_EVENT_DATA_CHANGED             STD_ON
#endif

#ifndef DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED
#define DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED            STD_ON
#endif

#ifndef DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION
#define DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION          STD_ON
#endif

#ifndef DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT
#define DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT          STD_ON
#endif

#ifndef DEM_CB_ENABLE_READ_DATA_ELEMENT
#define DEM_CB_ENABLE_READ_DATA_ELEMENT               STD_ON
#endif

#ifndef DEM_CB_ENABLE_GENERAL_INTERFACE
#define DEM_CB_ENABLE_GENERAL_INTERFACE               STD_ON
#endif

/* =========================================================================
 * Callback prototypes
 * ========================================================================= */

/* Status changed notifications */
#if (DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED == STD_ON)
void DemTriggerOnEventUdsStatus(Dem_EventIdType EventId,
                                Dem_UdsStatusByteType OldStatus,
                                Dem_UdsStatusByteType NewStatus);
#endif

#if (DEM_CB_ENABLE_MONITOR_STATUS_CHANGED == STD_ON)
void DemTriggerOnMonitorStatus(Dem_EventIdType EventId,
                               Dem_MonitorStatusType OldStatus,
                               Dem_MonitorStatusType NewStatus);
#endif

/* General interface notifications (global, not per-event specific) */
#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
void DemGeneralTriggerOnEventUdsStatus(Dem_EventIdType EventId,
                                       Dem_UdsStatusByteType OldStatus,
                                       Dem_UdsStatusByteType NewStatus);

void DemGeneralTriggerOnMonitorStatus(Dem_EventIdType EventId,
                                      Dem_MonitorStatusType OldStatus,
                                      Dem_MonitorStatusType NewStatus);
#endif

/* Event-related data changed (FreezeFrame / ExtendedData) */
#if (DEM_CB_ENABLE_EVENT_DATA_CHANGED == STD_ON)
void EventDataChanged(Dem_EventIdType EventId);
#endif

/* Clear control: allow/deny clearing of a specific event */
#if (DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED == STD_ON)
Std_ReturnType ClearEventAllowed(Dem_EventIdType EventId);
#endif

/* Clear notification: informs application that a clear has happened */
#if (DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION == STD_ON)
void ClearDtcNotification(uint32 Dtc, Dem_DTCOriginType Origin);
#endif

/* Init monitor callback */
#if (DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT == STD_ON)
void InitMonitorForEvent(Dem_EventIdType EventId, Dem_InitMonitorReasonType Reason);
#endif

/* Data element read callback: used as external FreezeFrame provider */
#if (DEM_CB_ENABLE_READ_DATA_ELEMENT == STD_ON)
Std_ReturnType ReadDataElement(Dem_EventIdType EventId,
                               uint16 DataElementId,
                               uint8 *DestBuffer,
                               uint16 *BufSize);
#endif

#endif /* DEM_CBK_H */

