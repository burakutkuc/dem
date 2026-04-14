/**
 * @file    Dem_Nvm.c
 * @brief   AUTOSAR CP R23-11 - DEM NvM integration (§7.11.5).
 *
 *  The DEM stores its persistent data in a single NvM block:
 *    - Primary event memory (Dem_MemoryEntryType array)
 *    - UDS status bytes (one per event)
 *    - Debounce counters (counter-based only)
 *    - Monotonic timestamp + XOR CRC for validity check
 *
 *  NvM abstraction:
 *    In a real project these calls map to NvM_ReadBlock / NvM_WriteBlock.
 *    Here we use a simple RAM-mirror pattern so the module compiles standalone.
 *    Replace NVM_READ_BLOCK / NVM_WRITE_BLOCK macros with your BSW calls.
 */

#include "Dem_Internal.h"
#include <string.h>   /* memcpy */

/* Forward declarations used before their definitions */
Std_ReturnType Dem_NvM_InitBlockCallback(void);

/* =========================================================================
 * NvM abstraction macros
 * Replace with NvM_ReadBlock() / NvM_WriteBlock() in real BSW context.
 * ========================================================================= */
#ifndef NVM_READ_BLOCK
    /* Stub: just return E_OK (RAM mirror already populated) */
    #define NVM_READ_BLOCK(blockId, dataPtr)   (E_OK)
#endif

#ifndef NVM_WRITE_BLOCK
    /* Stub: just return E_OK */
    #define NVM_WRITE_BLOCK(blockId, dataPtr)  (E_OK)
#endif

/* =========================================================================
 * Internal: pack runtime state → NvData
 * ========================================================================= */

static void Dem_Nvm_Pack(void)
{
    uint8 i;
    uint8 j;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        Dem_NvData.PrimaryMemory[i] = Dem_PrimaryMemory[i];
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_NvData.EventUdsStatus[i] = Dem_EventRuntime[i].UdsStatus;
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if (Dem_EventCfg[i].DebounceType == DEM_DEBOUNCE_COUNTER) {
            Dem_NvData.DebounceCounters[i] = Dem_EventRuntime[i].Debounce.Counter;
        } else {
            Dem_NvData.DebounceCounters[i] = 0;
        }
    }

    Dem_NvData.Timestamp = Dem_TimestampCounter;

    /* CRC covers everything except the CRC field itself */
    Dem_NvData.Crc = Dem_Internal_CalcCrc(
        (const uint8*)&Dem_NvData,
        (uint16)(sizeof(Dem_NvData) - sizeof(Dem_NvData.Crc)));

    (void)j;
}

/* =========================================================================
 * Internal: unpack NvData → runtime state
 * ========================================================================= */

static void Dem_Nvm_Unpack(void)
{
    uint8 i;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        Dem_PrimaryMemory[i] = Dem_NvData.PrimaryMemory[i];
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_EventRuntime[i].UdsStatus = Dem_NvData.EventUdsStatus[i];
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if (Dem_EventCfg[i].DebounceType == DEM_DEBOUNCE_COUNTER) {
            Dem_EventRuntime[i].Debounce.Counter = Dem_NvData.DebounceCounters[i];
        }
    }

    Dem_TimestampCounter = Dem_NvData.Timestamp;
}

/* =========================================================================
 * Internal: validate NvData CRC
 * ========================================================================= */

static uint8 Dem_Nvm_IsValid(void)
{
    uint16 calcCrc = Dem_Internal_CalcCrc(
        (const uint8*)&Dem_NvData,
        (uint16)(sizeof(Dem_NvData) - sizeof(Dem_NvData.Crc)));
    return (calcCrc == Dem_NvData.Crc) ? TRUE : FALSE;
}

/* =========================================================================
 * §7.11.5  Dem_Nvm_Init — called from Dem_Init() after NvM read
 * ========================================================================= */

Std_ReturnType Dem_Nvm_Init(void)
{
    Std_ReturnType ret;

    ret = NVM_READ_BLOCK(DEM_NVM_BLOCK_ID, &Dem_NvData);

    if ((ret == E_OK) && Dem_Nvm_IsValid()) {
        /* Restore persisted data */
        Dem_Nvm_Unpack();
    } else {
        /* First run or corrupted block: use defaults from PreInit */
        (void)Dem_NvM_InitBlockCallback();
    }

    return E_OK;
}

/* =========================================================================
 * §7.11.5  Dem_Nvm_Shutdown — called from Dem_Shutdown()
 * ========================================================================= */

void Dem_Nvm_Shutdown(void)
{
    Dem_Nvm_Pack();
    (void)NVM_WRITE_BLOCK(DEM_NVM_BLOCK_ID, &Dem_NvData);
}

/* =========================================================================
 * §7.11.5  Dem_NvM_InitBlockCallback
 *          Called by NvM on first use (default data initialisation).
 * ========================================================================= */

Std_ReturnType Dem_NvM_InitBlockCallback(void)
{
    uint8 i;

    /* Write default (zeroed) content to NvData */
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        Dem_NvData.PrimaryMemory[i].EventId = DEM_EVENT_INVALID;
    }
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_NvData.EventUdsStatus[i]    = DEM_UDS_STATUS_DEFAULT;
        Dem_NvData.DebounceCounters[i]  = 0;
    }
    Dem_NvData.Timestamp = 0U;
    Dem_NvData.Crc       = Dem_Internal_CalcCrc(
        (const uint8*)&Dem_NvData,
        (uint16)(sizeof(Dem_NvData) - sizeof(Dem_NvData.Crc)));

    return E_OK;
}

/* =========================================================================
 * §7.11.5  Dem_NvM_JobFinishedCallback
 *          Called by NvM when an async read/write job finishes.
 * ========================================================================= */

Std_ReturnType Dem_NvM_JobFinishedCallback(uint8 ServiceId, Std_ReturnType JobResult)
{
    (void)ServiceId;

    if (JobResult != E_OK) {
        /* NvM write failed — data not persistent.
         * In production: set a DEM internal fault flag or report to Det. */
        return E_NOT_OK;
    }

    return E_OK;
}
