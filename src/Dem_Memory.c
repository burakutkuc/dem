/**
 * @file    Dem_Memory.c
 * @brief   AUTOSAR CP R23-11 - DEM Primary Event Memory management (§7.7.2).
 *
 *  Responsibilities:
 *    - Store a confirmed event into the primary memory ring buffer.
 *    - Update an existing entry (occurrence counter, freeze frame, ext. data).
 *    - Displace the lowest-priority / oldest entry when the memory is full (§7.7.2.4).
 *    - Clear a single DTC or the entire memory (§7.7.2.2).
 *    - Report overflow and entry count.
 */

#include "Dem_Internal.h"
#include "Dem_Cbk.h"
#include "Dem_FreezeFrameProvider.h"

/* Forward declarations used before their definitions */
extern void Dem_Debounce_Reset(Dem_EventIdType EventId,
                              Dem_DebounceResetStatusType ResetStatus);

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

/** Find the slot index (0-based) that holds EventId. Returns 0xFF if not found. */
static uint8 Dem_Memory_FindEntry(Dem_EventIdType EventId)
{
    uint8 i;
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == EventId) {
            return i;
        }
    }
    return 0xFFU;
}

/** Find a free slot. Returns 0xFF if memory is full. */
static uint8 Dem_Memory_FindFreeSlot(void)
{
    uint8 i;
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            return i;
        }
    }
    return 0xFFU;
}

/** Find the slot with the lowest priority (highest priority number) + oldest timestamp. */
static uint8 Dem_Memory_FindDisplacementCandidate(void)
{
    uint8  candidate = 0U;
    uint8  worstPrio = 0U;
    uint32 oldestTs  = 0xFFFFFFFFUL;
    uint8  i;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        uint8 evPrio = Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Priority;
        if (evPrio > worstPrio ||
           (evPrio == worstPrio && Dem_PrimaryMemory[i].Timestamp < oldestTs))
        {
            worstPrio = evPrio;
            oldestTs  = Dem_PrimaryMemory[i].Timestamp;
            candidate = i;
        }
    }
    return candidate;
}

/** Capture FreezeFrame via external provider callback (ReadDataElement). */
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
static void Dem_Memory_CaptureFreezeFrame(uint8 slotIdx, Dem_EventIdType EventId)
{
    Dem_FreezeFrameType *ff = &Dem_PrimaryMemory[slotIdx].FreezeFrame;

    ff->RecordNumber    = 0x01U;
    /* Default: clear buffer */
    {
        uint8 i;
        for (i = 0U; i < DEM_FREEZE_FRAME_SIZE; i++) {
            ff->Data[i] = 0U;
        }
    }

    /* Always available fields from DEM runtime */
    ff->Data[0] = Dem_EventRuntime[EventId - 1U].UdsStatus;
    Dem_FF_PackU32LE(&ff->Data[1], Dem_TimestampCounter);
    ff->Data[11] = Dem_PrimaryMemory[slotIdx].OccurrenceCounter;

#if (DEM_CB_ENABLE_READ_DATA_ELEMENT == STD_ON)
    /* Ask upper layer for domain-specific data elements */
    {
        uint16 sz;
        uint16 v_u16;
        sint16 v_s16;

        /* BatteryVoltage_mV */
        sz = sizeof(v_u16);
        if (ReadDataElement(EventId, DEM_FF_DEID_BATTERY_MV, (uint8*)&v_u16, &sz) == E_OK &&
            sz == sizeof(v_u16))
        {
            Dem_FF_PackU16LE(&ff->Data[5], v_u16);
        }

        /* Temp1_centiDegC */
        sz = sizeof(v_s16);
        if (ReadDataElement(EventId, DEM_FF_DEID_TEMP1_CENTIDEGC, (uint8*)&v_s16, &sz) == E_OK &&
            sz == sizeof(v_s16))
        {
            Dem_FF_PackS16LE(&ff->Data[7], v_s16);
        }

        /* Temp2_centiDegC */
        sz = sizeof(v_s16);
        if (ReadDataElement(EventId, DEM_FF_DEID_TEMP2_CENTIDEGC, (uint8*)&v_s16, &sz) == E_OK &&
            sz == sizeof(v_s16))
        {
            Dem_FF_PackS16LE(&ff->Data[9], v_s16);
        }
    }
#endif

    ff->Valid           = TRUE;
}
#endif

/** Capture extended data (occurrence + aging counters). */
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
static void Dem_Memory_CaptureExtendedData(uint8 slotIdx, Dem_EventIdType EventId)
{
    Dem_ExtendedDataType *ed = &Dem_PrimaryMemory[slotIdx].ExtendedData;

    ed->RecordNumber = 0x01U;
    ed->Data[0]      = Dem_PrimaryMemory[slotIdx].OccurrenceCounter;
    ed->Data[1]      = Dem_PrimaryMemory[slotIdx].AgingCounter;
    ed->Data[2]      = Dem_PrimaryMemory[slotIdx].FailedCycleCounter;
    ed->Data[3]      = Dem_EventRuntime[EventId - 1U].UdsStatus;
    ed->Valid        = TRUE;
}
#endif

/* =========================================================================
 * Public: Store or update an event memory entry (§7.7.2)
 * ========================================================================= */

/**
 * @brief  Store a FAILED/CONFIRMED event into primary memory.
 *         If already present: update counters and data.
 *         If memory is full: displace the lowest-priority entry.
 * @return TRUE if successfully stored, FALSE if storage conditions blocked it.
 */
uint8 Dem_Memory_StoreEvent(Dem_EventIdType EventId)
{
    uint8 slotIdx;
    uint8 wroteOrUpdated = FALSE;

    /* Storage conditions check */
    if (Dem_DTCSettingDisabled != FALSE) {
        return FALSE;
    }
    if (!Dem_Internal_StorageConditionsMet(EventId)) {
        return FALSE;
    }

    slotIdx = Dem_Memory_FindEntry(EventId);

    if (slotIdx != 0xFFU) {
        /* --- Update existing entry --- */
        if (Dem_PrimaryMemory[slotIdx].OccurrenceCounter < 0xFFU) {
            Dem_PrimaryMemory[slotIdx].OccurrenceCounter++;
        }
        Dem_PrimaryMemory[slotIdx].UdsStatus    = Dem_EventRuntime[EventId - 1U].UdsStatus;
        Dem_PrimaryMemory[slotIdx].Timestamp    = Dem_TimestampCounter;
        Dem_PrimaryMemory[slotIdx].ConsecutiveFailed = Dem_EventRuntime[EventId - 1U].ConfirmCounter;
        wroteOrUpdated = TRUE;
    }
    else {
        /* --- Allocate new slot --- */
        slotIdx = Dem_Memory_FindFreeSlot();

        if (slotIdx == 0xFFU) {
            /* Memory full: check if incoming event has higher priority than worst entry */
            uint8 candidate = Dem_Memory_FindDisplacementCandidate();
            uint8 incomingPrio = Dem_EventCfg[EventId - 1U].Priority;
            uint8 candidatePrio = Dem_EventCfg[Dem_PrimaryMemory[candidate].EventId - 1U].Priority;

            if (incomingPrio < candidatePrio) {
                /* Displace: remove candidate event's CDTC / PDTC bits */
                Dem_EventIdType displaced = Dem_PrimaryMemory[candidate].EventId;
                Dem_EventRuntime[displaced - 1U].UdsStatus &=
                    (Dem_UdsStatusByteType)(~(DEM_UDS_STATUS_CDTC | DEM_UDS_STATUS_PDTC));
                slotIdx = candidate;
            } else {
                return FALSE; /* Cannot displace; incoming event has lower priority */
            }
        }

        /* Initialise the new slot */
        Dem_PrimaryMemory[slotIdx].EventId              = EventId;
        Dem_PrimaryMemory[slotIdx].UdsStatus            = Dem_EventRuntime[EventId - 1U].UdsStatus;
        Dem_PrimaryMemory[slotIdx].OccurrenceCounter    = 1U;
        Dem_PrimaryMemory[slotIdx].AgingCounter         = 0U;
        Dem_PrimaryMemory[slotIdx].FailedCycleCounter   = 0U;
        Dem_PrimaryMemory[slotIdx].ConsecutiveFailed    = 1U;
        Dem_PrimaryMemory[slotIdx].Timestamp            = Dem_TimestampCounter;
        wroteOrUpdated = TRUE;
    }

#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
    Dem_Memory_CaptureFreezeFrame(slotIdx, EventId);
#endif
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
    Dem_Memory_CaptureExtendedData(slotIdx, EventId);
#endif

#if (DEM_CB_ENABLE_EVENT_DATA_CHANGED == STD_ON)
    if (wroteOrUpdated) {
        EventDataChanged(EventId);
    }
#endif

    return TRUE;
}

/* =========================================================================
 * Public: Clear a single DTC (§7.7.2.2)
 * ========================================================================= */

/**
 * @brief  Clear the memory entry and reset UDS status for the given EventId.
 */
void Dem_Memory_ClearEntry(Dem_EventIdType EventId)
{
    uint8 slotIdx = Dem_Memory_FindEntry(EventId);
    uint8 i;

    if (slotIdx != 0xFFU) {
        /* Wipe the slot */
        Dem_PrimaryMemory[slotIdx].EventId            = DEM_EVENT_INVALID;
        Dem_PrimaryMemory[slotIdx].UdsStatus          = 0x00U;
        Dem_PrimaryMemory[slotIdx].OccurrenceCounter  = 0U;
        Dem_PrimaryMemory[slotIdx].AgingCounter       = 0U;
        Dem_PrimaryMemory[slotIdx].FailedCycleCounter = 0U;
        Dem_PrimaryMemory[slotIdx].ConsecutiveFailed  = 0U;
        Dem_PrimaryMemory[slotIdx].Timestamp          = 0U;
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
        for (i = 0U; i < DEM_FREEZE_FRAME_SIZE; i++) {
            Dem_PrimaryMemory[slotIdx].FreezeFrame.Data[i] = 0U;
        }
        Dem_PrimaryMemory[slotIdx].FreezeFrame.Valid = FALSE;
#endif
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
        for (i = 0U; i < DEM_EXTENDED_DATA_SIZE; i++) {
            Dem_PrimaryMemory[slotIdx].ExtendedData.Data[i] = 0U;
        }
        Dem_PrimaryMemory[slotIdx].ExtendedData.Valid = FALSE;
#endif
    }

    /* Reset UDS status byte to default (TNCLC | TNCTOC) */
    Dem_EventRuntime[EventId - 1U].UdsStatus       = DEM_UDS_STATUS_DEFAULT;
    Dem_EventRuntime[EventId - 1U].ConfirmCounter  = 0U;
    Dem_EventRuntime[EventId - 1U].HealingCounter  = 0U;
    Dem_Debounce_Reset(EventId, DEM_DEBOUNCE_STATUS_RESET);

    (void)i; /* suppress warning if freeze frame & ext data both off */
}

/* =========================================================================
 * Public: Clear ALL entries (§7.7.2.2)
 * ========================================================================= */

void Dem_Memory_ClearAll(void)
{
    uint8 i;
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_Memory_ClearEntry((Dem_EventIdType)(i + 1U));
    }
}

/* =========================================================================
 * Public: Overflow indication (§7.7.2.3)
 * ========================================================================= */

boolean Dem_Memory_IsOverflow(void)
{
    return (Dem_Memory_FindFreeSlot() == 0xFFU) ? TRUE : FALSE;
}

/* =========================================================================
 * Public: Count stored entries
 * ========================================================================= */

uint8 Dem_Memory_GetNumberOfEntries(void)
{
    uint8 count = 0U;
    uint8 i;
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId != DEM_EVENT_INVALID) {
            count++;
        }
    }
    return count;
}

/* =========================================================================
 * Public: Aging update (§7.7.8) — called on operation cycle end if PASSED
 * ========================================================================= */

void Dem_Memory_UpdateAging(void)
{
    uint8 i;
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        Dem_EventIdType evId = Dem_PrimaryMemory[i].EventId;
        if (evId == DEM_EVENT_INVALID) {
            continue;
        }
        /* Aging only when TF=0 */
        if ((Dem_EventRuntime[evId - 1U].UdsStatus & DEM_UDS_STATUS_TF) == 0U) {
            if (Dem_PrimaryMemory[i].AgingCounter <
                Dem_EventCfg[evId - 1U].AgingThreshold)
            {
                Dem_PrimaryMemory[i].AgingCounter++;
            }
            if (Dem_PrimaryMemory[i].AgingCounter >=
                Dem_EventCfg[evId - 1U].AgingThreshold)
            {
                /* Aged out: clear CDTC */
                Dem_EventRuntime[evId - 1U].UdsStatus &=
                    (Dem_UdsStatusByteType)(~DEM_UDS_STATUS_CDTC);
                Dem_Memory_ClearEntry(evId);
            }
        } else {
            Dem_PrimaryMemory[i].AgingCounter = 0U; /* reset if failed again */
        }
    }
}

/* (Other debounce symbols are declared in Dem.c; no need here.) */
