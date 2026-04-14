/**
 * @file    Dem_Cbk.c
 * @brief   Default callback implementations for DEM (weak/no-op).
 *
 * Integrators can override these symbols by providing their own definitions.
 * On GCC/Clang, functions are marked weak. On other toolchains, the
 * definitions are still provided, but overriding behaviour depends on the
 * linker.
 */

#include "Dem_Cbk.h"

#if defined(__GNUC__) || defined(__clang__)
  #define DEM_WEAK __attribute__((weak))
#else
  #define DEM_WEAK
#endif

#if (DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED == STD_ON)
DEM_WEAK void DemTriggerOnEventUdsStatus(Dem_EventIdType EventId,
                                        Dem_UdsStatusByteType OldStatus,
                                        Dem_UdsStatusByteType NewStatus)
{
    (void)EventId; (void)OldStatus; (void)NewStatus;
}
#endif

#if (DEM_CB_ENABLE_MONITOR_STATUS_CHANGED == STD_ON)
DEM_WEAK void DemTriggerOnMonitorStatus(Dem_EventIdType EventId,
                                       Dem_MonitorStatusType OldStatus,
                                       Dem_MonitorStatusType NewStatus)
{
    (void)EventId; (void)OldStatus; (void)NewStatus;
}
#endif

#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
DEM_WEAK void DemGeneralTriggerOnEventUdsStatus(Dem_EventIdType EventId,
                                               Dem_UdsStatusByteType OldStatus,
                                               Dem_UdsStatusByteType NewStatus)
{
    (void)EventId; (void)OldStatus; (void)NewStatus;
}

DEM_WEAK void DemGeneralTriggerOnMonitorStatus(Dem_EventIdType EventId,
                                              Dem_MonitorStatusType OldStatus,
                                              Dem_MonitorStatusType NewStatus)
{
    (void)EventId; (void)OldStatus; (void)NewStatus;
}
#endif

#if (DEM_CB_ENABLE_EVENT_DATA_CHANGED == STD_ON)
DEM_WEAK void EventDataChanged(Dem_EventIdType EventId)
{
    (void)EventId;
}
#endif

#if (DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED == STD_ON)
DEM_WEAK Std_ReturnType ClearEventAllowed(Dem_EventIdType EventId)
{
    (void)EventId;
    return E_OK; /* allow by default */
}
#endif

#if (DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION == STD_ON)
DEM_WEAK void ClearDtcNotification(uint32 Dtc, Dem_DTCOriginType Origin)
{
    (void)Dtc; (void)Origin;
}
#endif

#if (DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT == STD_ON)
DEM_WEAK void InitMonitorForEvent(Dem_EventIdType EventId, Dem_InitMonitorReasonType Reason)
{
    (void)EventId; (void)Reason;
}
#endif

#if (DEM_CB_ENABLE_READ_DATA_ELEMENT == STD_ON)
DEM_WEAK Std_ReturnType ReadDataElement(Dem_EventIdType EventId,
                                       uint16 DataElementId,
                                       uint8 *DestBuffer,
                                       uint16 *BufSize)
{
    (void)EventId; (void)DataElementId;
    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) {
        return E_NOT_OK;
    }
    *BufSize = 0U;
    return E_NOT_OK; /* not provided by default */
}
#endif

