/**
 * @file    Dem_Notify.h
 * @brief   Bare-metal IRQ-safe notify queue for DEM callbacks.
 *
 * Purpose:
 *  - Keep callback bodies short and deterministic.
 *  - Convert callback calls into queued events that the upper layer can drain.
 *
 * Concurrency model:
 *  - Push may happen from ISR or task context.
 *  - Pop happens from task context.
 *  - Both operations enter a critical section using DEM_ENTER_CRITICAL /
 *    DEM_EXIT_CRITICAL macros (override in Dem_Cfg.h integration).
 */

#ifndef DEM_NOTIFY_H
#define DEM_NOTIFY_H

#include "Dem_Types.h"
#include "Dem_Cfg.h"

typedef uint8 Dem_NotifyEventType;
#define DEM_NOTIFY_UDS_STATUS_CHANGED    (0U)
#define DEM_NOTIFY_DTC_CLEARED           (1U)
#define DEM_NOTIFY_EVENT_DATA_CHANGED    (2U)

typedef struct {
    Dem_NotifyEventType     Type;
    Dem_EventIdType         EventId;
    Dem_UdsStatusByteType   OldUdsStatus;
    Dem_UdsStatusByteType   NewUdsStatus;
    uint32                  Dtc;
    Dem_DTCOriginType       Origin;
} Dem_NotifyEvent;

void  Dem_Notify_Init(void);
uint8 Dem_Notify_PushFromIsr(const Dem_NotifyEvent *ev);
uint8 Dem_Notify_Pop(Dem_NotifyEvent *ev);

uint16 Dem_Notify_GetDroppedCount(void);
uint8  Dem_Notify_GetCount(void);

#endif /* DEM_NOTIFY_H */

