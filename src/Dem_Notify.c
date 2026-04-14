/**
 * @file    Dem_Notify.c
 * @brief   Bare-metal IRQ-safe ring buffer for DEM notifications.
 */

#include "Dem_Notify.h"

#ifndef DEM_NOTIFY_QUEUE_SIZE
#define DEM_NOTIFY_QUEUE_SIZE   (32U)
#endif

static volatile uint8  s_head;
static volatile uint8  s_tail;
static volatile uint8  s_count;
static volatile uint16 s_dropped;
static Dem_NotifyEvent s_q[DEM_NOTIFY_QUEUE_SIZE];

void Dem_Notify_Init(void)
{
    DEM_ENTER_CRITICAL();
    s_head = 0U;
    s_tail = 0U;
    s_count = 0U;
    s_dropped = 0U;
    DEM_EXIT_CRITICAL();
}

uint8 Dem_Notify_PushFromIsr(const Dem_NotifyEvent *ev)
{
    uint8 ok = FALSE;

    if (ev == NULL_PTR) {
        return FALSE;
    }

    DEM_ENTER_CRITICAL();
    if (s_count < DEM_NOTIFY_QUEUE_SIZE) {
        s_q[s_head] = *ev;
        s_head = (uint8)((s_head + 1U) % DEM_NOTIFY_QUEUE_SIZE);
        s_count++;
        ok = TRUE;
    } else {
        s_dropped++;
        ok = FALSE;
    }
    DEM_EXIT_CRITICAL();

    return ok;
}

uint8 Dem_Notify_Pop(Dem_NotifyEvent *ev)
{
    uint8 ok = FALSE;

    if (ev == NULL_PTR) {
        return FALSE;
    }

    DEM_ENTER_CRITICAL();
    if (s_count > 0U) {
        *ev = s_q[s_tail];
        s_tail = (uint8)((s_tail + 1U) % DEM_NOTIFY_QUEUE_SIZE);
        s_count--;
        ok = TRUE;
    }
    DEM_EXIT_CRITICAL();

    return ok;
}

uint16 Dem_Notify_GetDroppedCount(void)
{
    uint16 v;
    DEM_ENTER_CRITICAL();
    v = s_dropped;
    DEM_EXIT_CRITICAL();
    return v;
}

uint8 Dem_Notify_GetCount(void)
{
    uint8 v;
    DEM_ENTER_CRITICAL();
    v = s_count;
    DEM_EXIT_CRITICAL();
    return v;
}

