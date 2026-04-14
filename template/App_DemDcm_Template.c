/**
 * @file    App_DemDcm_Template.c
 * @brief   DCM upper-layer template — shows how a UDS diagnostic server
 *          uses the DEM API to implement UDS services.
 *
 *  This file is NOT part of the DEM module itself.
 *  Covered UDS services and their DEM call sequences:
 *
 *    UDS $19 02  ReadDTCInformationByStatusMask  → SetDTCFilter / GetNext loop
 *    UDS $19 04  FaultDetectionCounter           → GetNextFilteredDTCAndFDC
 *    UDS $19 06  FreezeFrame data                → SelectFreezeFrameData / GetNext
 *    UDS $19 06  Extended data record            → SelectExtendedDataRecord / GetNext
 *    UDS $19 09  DTC by occurrence time          → GetDTCByOccurrenceTime
 *    UDS $14     ClearDiagnosticInformation      → SelectDTC / ClearDTC (async)
 *    UDS $85     ControlDTCSetting               → Disable/EnableDTCSetting
 *
 *  Each function includes a flow comment showing DEM internal behaviour.
 */

#include "Dem.h"
#include "Dem_Cbk.h"
#include <string.h>   /* memset */

/* Single client ID — DEM_MULTIPLE_CLIENTS is STD_OFF */
#define APP_DCM_CLIENT_ID   (0U)

/* =========================================================================
 * UDS $19 02 — ReadDTCInformationByStatusMask
 * ========================================================================= */

/**
 * @brief  Build a response buffer containing all DTCs matching statusMask.
 *
 *  DEM internal flow:
 *    Dem_SetDTCFilter()
 *      → DEM stores filter params in Dem_ClientState
 *      → resets FilterIndex to 0
 *
 *    Dem_GetNumberOfFilteredDTC()
 *      → DEM iterates Dem_EventRuntime[0..14]:
 *          for each event: (UdsStatus & statusMask) != 0 → count++
 *      → resets FilterIndex to 0 again for subsequent GetNext calls
 *
 *    Dem_GetNextFilteredDTC()  (called in loop until DEM_FILTERED_NO_MATCHING_DTC)
 *      → DEM advances FilterIndex, returns next matching DTC + status byte
 *
 * @param  statusMask  UDS status byte bitmask (e.g. 0x09 = TF | CDTC)
 * @param  respBuf     output buffer (filled with DTC[3] + status[1] per entry)
 * @param  respLen     output: number of bytes written
 */
void App_Dcm_Service19_02(uint8 statusMask, uint8 *respBuf, uint16 *respLen)
{
    uint32                dtc       = 0U;
    Dem_UdsStatusByteType dtcStatus = 0U;
    uint16                count     = 0U;

    if ((respBuf == NULL_PTR) || (respLen == NULL_PTR)) { return; }
    *respLen = 0U;

    /* 1. Set filter */
    Dem_SetDTCFilter(APP_DCM_CLIENT_ID,
                     statusMask,
                     DEM_DTC_KIND_ALL_DTCS,
                     DEM_DTC_FORMAT_UDS,
                     DEM_DTC_ORIGIN_PRIMARY_MEMORY,
                     DEM_FILTER_WITH_SEVERITY_NO,
                     DEM_SEVERITY_NO_SEVERITY,
                     DEM_FILTER_FOR_FDC_NO);

    /* 2. How many match? (optional; DCM may pre-calculate response length) */
    Dem_GetNumberOfFilteredDTC(APP_DCM_CLIENT_ID, &count);
    (void)count;

    /* 3. Iterate and fill response */
    while (Dem_GetNextFilteredDTC(APP_DCM_CLIENT_ID, &dtc, &dtcStatus) == E_OK) {
        respBuf[(*respLen)++] = (uint8)(dtc >> 16U);
        respBuf[(*respLen)++] = (uint8)(dtc >>  8U);
        respBuf[(*respLen)++] = (uint8)(dtc);
        respBuf[(*respLen)++] = dtcStatus;
    }
}

/* =========================================================================
 * UDS $19 04 — FaultDetectionCounter per filtered DTC
 * ========================================================================= */

/**
 * @brief  Return DTC + FDC pairs for all DTCs matching statusMask.
 *
 *  DEM internal flow:
 *    Dem_GetNextFilteredDTCAndFDC()
 *      → calls GetNextFilteredDTC internally (same FilterIndex walk)
 *      → then calls Dem_Debounce_GetFDC(EventId) to get the normalised
 *        fault detection counter [-128 .. +127]
 */
void App_Dcm_Service19_04(uint8 statusMask, uint8 *respBuf, uint16 *respLen)
{
    uint32 dtc = 0U;
    sint8  fdc = 0;

    if ((respBuf == NULL_PTR) || (respLen == NULL_PTR)) { return; }
    *respLen = 0U;

    Dem_SetDTCFilter(APP_DCM_CLIENT_ID,
                     statusMask,
                     DEM_DTC_KIND_ALL_DTCS,
                     DEM_DTC_FORMAT_UDS,
                     DEM_DTC_ORIGIN_PRIMARY_MEMORY,
                     DEM_FILTER_WITH_SEVERITY_NO,
                     DEM_SEVERITY_NO_SEVERITY,
                     DEM_FILTER_FOR_FDC_YES);

    while (Dem_GetNextFilteredDTCAndFDC(APP_DCM_CLIENT_ID, &dtc, &fdc) == E_OK) {
        respBuf[(*respLen)++] = (uint8)(dtc >> 16U);
        respBuf[(*respLen)++] = (uint8)(dtc >>  8U);
        respBuf[(*respLen)++] = (uint8)(dtc);
        respBuf[(*respLen)++] = (uint8)fdc;  /* signed cast intentional */
    }
}

/* =========================================================================
 * UDS $19 06 — FreezeFrame data for a specific DTC
 * ========================================================================= */

/**
 * @brief  Read freeze frame data for a specific DTC and record number.
 *
 *  DEM internal flow:
 *    Dem_SelectDTC()
 *      → stores DTC + format + origin in Dem_ClientState.SelectionValid = TRUE
 *
 *    Dem_SelectFreezeFrameData()
 *      → stores requested record number in Dem_ClientState.FFFilterRecord
 *
 *    Dem_DisableDTCRecordUpdate()
 *      → sets Dem_ClientState.RecordUpdateDisabled = TRUE
 *      → while set, Dem_Memory_StoreEvent() does NOT overwrite this entry
 *        (prevents data changing mid-read)
 *
 *    Dem_GetSizeOfFreezeFrameSelection()
 *      → scans Dem_PrimaryMemory[] for the selected DTC
 *      → returns DEM_FREEZE_FRAME_SIZE (16) if FreezeFrame.Valid == TRUE
 *
 *    Dem_GetNextFreezeFrameData()
 *      → copies FreezeFrame.Data[16] into DestBuffer
 *
 *    Dem_EnableDTCRecordUpdate()
 *      → clears RecordUpdateDisabled; normal storage resumes
 *
 * @param  dtc        3-byte UDS DTC value
 * @param  record     freeze frame record number (0xFF = all)
 * @param  respBuf    output buffer
 * @param  respLen    output: bytes written
 */
void App_Dcm_Service19_06_FreezeFrame(uint32 dtc, uint8 record,
                                       uint8 *respBuf, uint16 *respLen)
{
    uint16 ffSize = sizeof(uint8) * 32U; /* upper bound for safety */
    Std_ReturnType ret;

    if ((respBuf == NULL_PTR) || (respLen == NULL_PTR)) { return; }
    *respLen = 0U;

    /* Step 1: Select the DTC */
    Dem_SelectDTC(APP_DCM_CLIENT_ID, dtc,
                  DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY);

    /* Step 2: Select the record */
    Dem_SelectFreezeFrameData(APP_DCM_CLIENT_ID, record);

    /* Step 3: Lock record against updates during read */
    Dem_DisableDTCRecordUpdate(APP_DCM_CLIENT_ID);

    /* Step 4: Determine how many bytes will be returned */
    ret = Dem_GetSizeOfFreezeFrameSelection(APP_DCM_CLIENT_ID, &ffSize);
    if ((ret == E_OK) && (ffSize > 0U)) {
        /* Step 5: Copy data */
        ret = Dem_GetNextFreezeFrameData(APP_DCM_CLIENT_ID, respBuf, &ffSize);
        if (ret == E_OK) {
            *respLen = ffSize;
        }
    }

    /* Step 6: Unlock */
    Dem_EnableDTCRecordUpdate(APP_DCM_CLIENT_ID);
}

/* =========================================================================
 * UDS $19 06 — Extended Data Record for a specific DTC
 * ========================================================================= */

/**
 * @brief  Read extended data record for a specific DTC.
 *
 *  DEM internal flow (same lock pattern as FreezeFrame):
 *    Dem_SelectDTC()          → SelectionValid = TRUE
 *    Dem_SelectExtendedDataRecord() → record number stored
 *    Dem_DisableDTCRecordUpdate()   → write lock
 *    Dem_GetSizeOfExtendedDataRecordSelection() → scans PrimaryMemory
 *    Dem_GetNextExtendedDataRecord() → copies ExtendedData.Data[8]
 *    Dem_EnableDTCRecordUpdate()    → unlock
 */
void App_Dcm_Service19_06_ExtData(uint32 dtc, uint8 record,
                                   uint8 *respBuf, uint16 *respLen)
{
    uint16 edSize = sizeof(uint8) * 16U;
    Std_ReturnType ret;

    if ((respBuf == NULL_PTR) || (respLen == NULL_PTR)) { return; }
    *respLen = 0U;

    Dem_SelectDTC(APP_DCM_CLIENT_ID, dtc,
                  DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY);

    Dem_SelectExtendedDataRecord(APP_DCM_CLIENT_ID, record);

    Dem_DisableDTCRecordUpdate(APP_DCM_CLIENT_ID);

    ret = Dem_GetSizeOfExtendedDataRecordSelection(APP_DCM_CLIENT_ID, &edSize);
    if ((ret == E_OK) && (edSize > 0U)) {
        ret = Dem_GetNextExtendedDataRecord(APP_DCM_CLIENT_ID, respBuf, &edSize);
        if (ret == E_OK) {
            *respLen = edSize;
        }
    }

    Dem_EnableDTCRecordUpdate(APP_DCM_CLIENT_ID);
}

/* =========================================================================
 * UDS $19 09 — DTC by occurrence time
 * ========================================================================= */

/**
 * @brief  Return the first or most recent confirmed DTC.
 *
 *  DEM internal flow:
 *    Dem_GetDTCByOccurrenceTime()
 *      → scans Dem_PrimaryMemory[]:
 *          FIRST_FAILED_DTC    → entry with smallest Timestamp
 *          MOST_RECENT_FAILED  → entry with largest Timestamp
 *      → returns DTC value or DEM_NO_SUCH_ELEMENT
 */
Std_ReturnType App_Dcm_Service19_09(Dem_DTCRequestType requestType,
                                     uint32 *dtcOut)
{
    if (dtcOut == NULL_PTR) { return E_NOT_OK; }
    *dtcOut = 0U;
    return Dem_GetDTCByOccurrenceTime(APP_DCM_CLIENT_ID, requestType, dtcOut);
}

/* =========================================================================
 * UDS $14 — ClearDiagnosticInformation  (asynchronous pattern)
 * ========================================================================= */

/**
 * @brief  Clear the selected DTC group.  Call repeatedly until E_OK or error.
 *
 *  DEM internal flow:
 *    --- First call ---
 *    Dem_SelectDTC()
 *      → DTC stored in Dem_ClientState.SelectedDTC
 *      → SelectionValid = TRUE
 *
 *    Dem_ClearDTC()
 *      → if SelectedDTC == 0x00FFFFFF (all DTCs):
 *           Dem_Memory_ClearAll():
 *             for each event: EventId=0, UdsStatus=0x50, counters=0
 *             debounce reset for all events
 *      → else (single DTC):
 *           Dem_Memory_ClearEntry(EventId):
 *             PrimaryMemory slot wiped
 *             UdsStatus reset to 0x50 (TNCLC | TNCTOC)
 *             debounce counter zeroed
 *      → Dem_TimestampCounter++ (monotonic)
 *      → ClearPending = FALSE
 *      → Returns E_OK (synchronous in this implementation)
 *
 *    --- Asynchronous (when NvM write is pending, DEM_PENDING returned) ---
 *    Subsequent calls:
 *    Dem_GetDTCSelectionResultForClearDTC()
 *      → returns DEM_PENDING while NvM write is in progress
 *      → returns E_OK when complete
 *
 * @param  groupDtc  0x00FFFFFF = all DTCs; specific DTC value = single clear
 * @return E_OK (done), DEM_PENDING (in progress), E_NOT_OK (error)
 */
Std_ReturnType App_Dcm_Service14(uint32 groupDtc)
{
    Std_ReturnType selResult;
    Std_ReturnType ret;

    /* Check if a previous clear is still pending */
    selResult = Dem_GetDTCSelectionResult(APP_DCM_CLIENT_ID);
    if (selResult == DEM_PENDING) {
        /* Poll for completion */
        return Dem_GetDTCSelectionResultForClearDTC(APP_DCM_CLIENT_ID);
    }

    /* New clear request: select + initiate */
    Dem_SelectDTC(APP_DCM_CLIENT_ID,
                  groupDtc,
                  DEM_DTC_FORMAT_UDS,
                  DEM_DTC_ORIGIN_PRIMARY_MEMORY);

    ret = Dem_ClearDTC(APP_DCM_CLIENT_ID);

    /*
     * ret == E_OK      → all DTCs cleared; send positive response
     * ret == DEM_PENDING → call this function again next DCM cycle
     * ret == E_NOT_OK  → DTC not found or conditions not met; send NRC 0x22
     */
    return ret;
}

/* =========================================================================
 * UDS $85 — ControlDTCSetting
 * ========================================================================= */

/**
 * @brief  Enable or disable DTC recording.
 *
 *  DEM internal flow:
 *    Dem_DisableDTCSetting()
 *      → Dem_DTCSettingDisabled = TRUE
 *      → Dem_Memory_StoreEvent() returns FALSE immediately (no writes)
 *      → UDS status bits are still updated by SetEventStatus; only
 *        memory storage is blocked.
 *
 *    Dem_EnableDTCSetting()
 *      → Dem_DTCSettingDisabled = FALSE
 *      → Normal storage resumes.
 *
 * @param  subFunc  0x01 = disable, 0x02 = enable
 */
void App_Dcm_Service85(uint8 subFunc)
{
    if (subFunc == 0x01U) {
        /*
         * $85 01: Stop recording DTCs.
         * Useful during ECU reprogramming or active tests to prevent
         * spurious faults from being logged.
         */
        Dem_DisableDTCSetting(DEM_DTC_GROUP_ALL_DTCS);
    } else if (subFunc == 0x02U) {
        /*
         * $85 02: Resume normal DTC recording.
         */
        Dem_EnableDTCSetting(DEM_DTC_GROUP_ALL_DTCS);
    }
}

/* =========================================================================
 * Additional: read current UDS status of a specific DTC ($19 01)
 * ========================================================================= */

/**
 * @brief  Return UDS status byte of a specific DTC (UDS $19 01 / $09 02).
 *
 *  DEM internal flow:
 *    Dem_SelectDTC() → stores DTC in ClientState
 *    Dem_GetStatusOfDTC() → scans EventCfg[] for matching DTC,
 *                           returns Dem_EventRuntime[].UdsStatus
 */
Std_ReturnType App_Dcm_GetDtcStatus(uint32 dtc, uint8 *statusOut)
{
    Dem_UdsStatusByteType udsStatus = 0U;
    Std_ReturnType ret;

    if (statusOut == NULL_PTR) { return E_NOT_OK; }

    Dem_SelectDTC(APP_DCM_CLIENT_ID, dtc,
                  DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY);

    ret = Dem_GetStatusOfDTC(APP_DCM_CLIENT_ID, &udsStatus);
    if (ret == E_OK) {
        *statusOut = udsStatus;
    }
    return ret;
}

/* =========================================================================
 * Additional: check memory overflow and count stored DTCs
 * ========================================================================= */

/**
 * @brief  Utility: how many DTCs are in primary memory right now?
 *
 *  DEM internal:
 *    Scans Dem_PrimaryMemory[0..9], counts entries with EventId != 0.
 */
uint8 App_Dcm_GetStoredDtcCount(void)
{
    uint8 count = 0U;
    Dem_GetNumberOfEventMemoryEntries(DEM_DTC_ORIGIN_PRIMARY_MEMORY, &count);
    return count;
}

/**
 * @brief  Utility: is primary memory full?
 *
 *  DEM internal:
 *    Dem_Memory_FindFreeSlot() returns 0xFF → overflow = TRUE.
 */
boolean App_Dcm_IsMemoryFull(void)
{
    boolean overflow = FALSE;
    Dem_GetEventMemoryOverflow(DEM_DTC_ORIGIN_PRIMARY_MEMORY, &overflow);
    return overflow;
}
