/**
 * @file    Dem_Debounce.c
 * @brief   AUTOSAR CP R23-11 - DEM Debounce algorithms (§7.7.3).
 *
 *  Three algorithms:
 *    1. Counter-based  (§7.7.3.1) - increment/decrement a signed counter.
 *    2. Time-based     (§7.7.3.2) - accumulate elapsed ms in a direction.
 *    3. Monitor-internal (§7.7.3.3) - PASSED/FAILED pass straight through.
 *
 *  The public entry point is Dem_Debounce_ProcessStatus().
 *  It takes a PREFAILED/PREPASSED/FAILED/PASSED input and returns the
 *  qualified status (FAILED or PASSED) or DEM_EVENT_STATUS_PREFAILED /
 *  DEM_EVENT_STATUS_PREPASSED when not yet qualified.
 */

#include "Dem_Internal.h"

/* =========================================================================
 * Counter-based debounce  (§7.7.3.1)
 * ========================================================================= */

/**
 * @brief  Run one step of the counter-based debounce algorithm.
 * @param  EventId   1-based event identifier
 * @param  Status    raw monitor report (PREFAILED / PREPASSED / FAILED / PASSED)
 * @return qualified status
 */
static Dem_EventStatusType Dem_Debounce_Counter(Dem_EventIdType EventId,
                                                 Dem_EventStatusType Status)
{
    const Dem_EventCfgType *cfg = &Dem_EventCfg[EventId - 1U];
    Dem_DebounceRuntimeType *deb = &Dem_EventRuntime[EventId - 1U].Debounce;
    Dem_EventStatusType result;

    if ((Status == DEM_EVENT_STATUS_PREFAILED) ||
        (Status == DEM_EVENT_STATUS_FAILED))
    {
        /* Step counter toward fail threshold */
        if (deb->Counter < cfg->CounterThreshFail) {
            deb->Counter++;
        }

        if (deb->Counter >= cfg->CounterThreshFail) {
            result = DEM_EVENT_STATUS_FAILED;
        } else {
            result = DEM_EVENT_STATUS_PREFAILED;
        }
    }
    else /* PREPASSED or PASSED */
    {
        /* Step counter toward pass threshold */
        if (deb->Counter > cfg->CounterThreshPass) {
            deb->Counter--;
        }

        if (deb->Counter <= cfg->CounterThreshPass) {
            result = DEM_EVENT_STATUS_PASSED;
        } else {
            result = DEM_EVENT_STATUS_PREPASSED;
        }
    }

    return result;
}

/* =========================================================================
 * Time-based debounce  (§7.7.3.2)
 * Called from Dem_MainFunction() every DEM_MAIN_PERIOD_MS milliseconds.
 * ========================================================================= */

#define DEM_MAIN_PERIOD_MS  (10U)   /* assumed task period in ms            */

/**
 * @brief  Run one step of the time-based debounce algorithm.
 *         Called periodically; accumulates time while the same direction
 *         is being reported.
 */
static Dem_EventStatusType Dem_Debounce_Time(Dem_EventIdType EventId,
                                              Dem_EventStatusType Status)
{
    const Dem_EventCfgType *cfg = &Dem_EventCfg[EventId - 1U];
    Dem_DebounceRuntimeType *deb = &Dem_EventRuntime[EventId - 1U].Debounce;
    uint8 newDirection;
    Dem_EventStatusType result;

    if ((Status == DEM_EVENT_STATUS_PREFAILED) ||
        (Status == DEM_EVENT_STATUS_FAILED))
    {
        newDirection = 1U; /* fail direction */
    }
    else
    {
        newDirection = 0U; /* pass direction */
    }

    /* Direction change resets the timer */
    if (newDirection != deb->Direction) {
        deb->TimeCounter = 0U;
        deb->Direction   = newDirection;
    }

    /* Accumulate time */
    if (deb->TimeCounter < 0xFFFFU) {
        deb->TimeCounter = (uint16)(deb->TimeCounter + DEM_MAIN_PERIOD_MS);
    }

    /* Evaluate threshold */
    if (newDirection == 1U) {
        if (deb->TimeCounter >= cfg->TimeThreshFailMs) {
            result = DEM_EVENT_STATUS_FAILED;
        } else {
            result = DEM_EVENT_STATUS_PREFAILED;
        }
    } else {
        if (deb->TimeCounter >= cfg->TimeThreshPassMs) {
            result = DEM_EVENT_STATUS_PASSED;
        } else {
            result = DEM_EVENT_STATUS_PREPASSED;
        }
    }

    return result;
}

/* =========================================================================
 * Monitor-internal debounce  (§7.7.3.3)
 * The SW-C is responsible for debouncing; DEM accepts FAILED/PASSED only.
 * PREFAILED/PREPASSED are treated as the base direction without qualification.
 * ========================================================================= */

static Dem_EventStatusType Dem_Debounce_MonitorInternal(Dem_EventStatusType Status)
{
    if ((Status == DEM_EVENT_STATUS_FAILED) ||
        (Status == DEM_EVENT_STATUS_PREFAILED))
    {
        return DEM_EVENT_STATUS_FAILED;
    }
    return DEM_EVENT_STATUS_PASSED;
}

/* =========================================================================
 * Public entry point
 * ========================================================================= */

/**
 * @brief  Apply the configured debounce algorithm for an event.
 * @param  EventId   1-based event identifier
 * @param  Status    raw monitor status from Dem_SetEventStatus()
 * @return qualified Dem_EventStatusType (FAILED, PASSED, PREFAILED, PREPASSED)
 */
Dem_EventStatusType Dem_Debounce_ProcessStatus(Dem_EventIdType EventId,
                                                Dem_EventStatusType Status)
{
    const Dem_EventCfgType *cfg = &Dem_EventCfg[EventId - 1U];
    Dem_EventStatusType result;

    switch (cfg->DebounceType) {
        case DEM_DEBOUNCE_COUNTER:
            result = Dem_Debounce_Counter(EventId, Status);
            break;
        case DEM_DEBOUNCE_TIME:
            result = Dem_Debounce_Time(EventId, Status);
            break;
        case DEM_DEBOUNCE_MONITOR_INTERNAL:
        default:
            result = Dem_Debounce_MonitorInternal(Status);
            break;
    }

    return result;
}

/* =========================================================================
 * Reset debounce state  (§7.7.3.4)
 * ========================================================================= */

/**
 * @brief  Reset or freeze the debounce counter/timer for an event.
 */
void Dem_Debounce_Reset(Dem_EventIdType EventId,
                         Dem_DebounceResetStatusType ResetStatus)
{
    if (ResetStatus == DEM_DEBOUNCE_STATUS_RESET) {
        Dem_EventRuntime[EventId - 1U].Debounce.Counter      = 0;
        Dem_EventRuntime[EventId - 1U].Debounce.TimeCounter  = 0U;
        Dem_EventRuntime[EventId - 1U].Debounce.Direction    = 0U;
    }
    /* DEM_DEBOUNCE_STATUS_FREEZE: do nothing (keep current state) */
}

/* =========================================================================
 * Fault detection counter retrieval  (§7.7.3.5)
 * Returns a normalised value in range [-128 .. +127]:
 *   +127 = about to confirm FAILED
 *   -128 = about to confirm PASSED
 * ========================================================================= */

sint8 Dem_Debounce_GetFDC(Dem_EventIdType EventId)
{
    const Dem_EventCfgType *cfg = &Dem_EventCfg[EventId - 1U];
    const Dem_DebounceRuntimeType *deb = &Dem_EventRuntime[EventId - 1U].Debounce;
    sint8 fdc = 0;

    switch (cfg->DebounceType) {
        case DEM_DEBOUNCE_COUNTER:
            /* Map counter linearly to [-128, 127] */
            if (cfg->CounterThreshFail != 0) {
                fdc = (sint8)((deb->Counter * 127) / cfg->CounterThreshFail);
            }
            break;

        case DEM_DEBOUNCE_TIME:
            if (deb->Direction == 1U) {
                if (cfg->TimeThreshFailMs != 0U) {
                    fdc = (sint8)((deb->TimeCounter * 127U) / cfg->TimeThreshFailMs);
                }
            } else {
                if (cfg->TimeThreshPassMs != 0U) {
                    fdc = (sint8)(-((sint16)(deb->TimeCounter * 128U) /
                                    (sint16)cfg->TimeThreshPassMs));
                }
            }
            break;

        case DEM_DEBOUNCE_MONITOR_INTERNAL:
        default:
            /* Already qualified; return extreme values */
            if ((Dem_EventRuntime[EventId - 1U].UdsStatus & DEM_UDS_STATUS_TF) != 0U) {
                fdc = 127;
            } else {
                fdc = -128;
            }
            break;
    }

    return fdc;
}

/* =========================================================================
 * Get debouncing state bitmask  (§8.3.3.8)
 * ========================================================================= */

Dem_DebouncingStateType Dem_Debounce_GetState(Dem_EventIdType EventId)
{
    const Dem_DebounceRuntimeType *deb = &Dem_EventRuntime[EventId - 1U].Debounce;
    const Dem_EventCfgType        *cfg = &Dem_EventCfg[EventId - 1U];
    Dem_DebouncingStateType state = 0U;

    if (cfg->DebounceType == DEM_DEBOUNCE_COUNTER) {
        if (deb->Counter > 0) {
            state |= DEM_DEBOUNCESTATUS_TEMPORARILY_DEFECTIVE;
        }
        if (deb->Counter >= cfg->CounterThreshFail) {
            state |= DEM_DEBOUNCESTATUS_FINALLY_DEFECTIVE;
        }
        if (deb->Counter > 0) {
            state |= DEM_DEBOUNCESTATUS_FREQLY_DETECTED;
        }
    } else if (cfg->DebounceType == DEM_DEBOUNCE_TIME) {
        if ((deb->Direction == 1U) && (deb->TimeCounter > 0U)) {
            state |= DEM_DEBOUNCESTATUS_TEMPORARILY_DEFECTIVE;
        }
        if ((deb->Direction == 1U) && (deb->TimeCounter >= cfg->TimeThreshFailMs)) {
            state |= DEM_DEBOUNCESTATUS_FINALLY_DEFECTIVE;
        }
    }

    /* TestComplete if UDS TF or status passed */
    if ((Dem_EventRuntime[EventId - 1U].UdsStatus &
        (DEM_UDS_STATUS_TF | DEM_UDS_STATUS_TNCTOC)) != DEM_UDS_STATUS_TNCTOC) {
        state |= DEM_DEBOUNCESTATUS_TEST_COMPLETE;
    }

    return state;
}
