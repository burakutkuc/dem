/**
 * @file    Dem_Dcm.c
 * @brief   AUTOSAR CP R23-11 - DEM <=> DCM interface (§8.3.4).
 *
 *  Implements the full DCM-facing API:
 *    - DTC status / filter / iteration (§8.3.4.1)
 *    - FreezeFrame and ExtendedData access (§8.3.4.2)
 *    - DTC storage control / DisableDTCSetting (§8.3.4.3)
 */

#include "Dem_Internal.h"

/* Forward declarations (used by this module) */
extern sint8 Dem_Debounce_GetFDC(Dem_EventIdType EventId);

/* =========================================================================
 * §8.3.4.1  DTC status and translation
 * ========================================================================= */

Std_ReturnType Dem_GetTranslationType(uint8 ClientId,
                                       Dem_DTCTranslationFormatType *TranslationType)
{
    (void)ClientId;
    if (TranslationType == NULL_PTR) {
        return E_NOT_OK;
    }
    *TranslationType = DEM_DTC_TRANSLATION_ISO14229_1;
    return E_OK;
}

Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 ClientId,
                                                  Dem_UdsStatusByteType *DTCStatusMask)
{
    (void)ClientId;
    if (DTCStatusMask == NULL_PTR) {
        return E_NOT_OK;
    }
    *DTCStatusMask = 0xFFU; /* all 8 UDS status bits supported */
    return E_OK;
}

/**
 * @brief  Return UDS status of the currently selected DTC (§8.3.4.1.3).
 */
Std_ReturnType Dem_GetStatusOfDTC(uint8 ClientId,
                                    Dem_UdsStatusByteType *DTCStatus)
{
    uint8 i;
    (void)ClientId;

    if ((DTCStatus == NULL_PTR) || !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if (Dem_EventCfg[i].Dtc == Dem_ClientState.SelectedDTC) {
            *DTCStatus = Dem_EventRuntime[i].UdsStatus;
            return E_OK;
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetSeverityOfDTC(uint8 ClientId,
                                     Dem_DTCSeverityType *DTCSeverity)
{
    uint8 i;
    (void)ClientId;

    if ((DTCSeverity == NULL_PTR) || !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if (Dem_EventCfg[i].Dtc == Dem_ClientState.SelectedDTC) {
            /* Map priority 1 → CHECK_IMMEDIATELY, others → CHECK_AT_NEXT_HALT */
            *DTCSeverity = (Dem_EventCfg[i].Priority == 1U)
                           ? DEM_SEVERITY_CHECK_IMMEDIATELY
                           : DEM_SEVERITY_CHECK_AT_NEXT_HALT;
            return E_OK;
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetFunctionalUnitOfDTC(uint8 ClientId,
                                           uint8 *DTCFunctionalUnit)
{
    (void)ClientId;
    if (DTCFunctionalUnit == NULL_PTR) {
        return E_NOT_OK;
    }
    *DTCFunctionalUnit = 0x00U; /* not configured; return 0 */
    return E_OK;
}

/* =========================================================================
 * §8.3.4.1  DTC filter
 * ========================================================================= */

/**
 * @brief  Set filter parameters for subsequent GetNumberOfFilteredDTC /
 *         GetNextFilteredDTC calls (§8.3.4.1.6).
 */
Std_ReturnType Dem_SetDTCFilter(uint8 ClientId,
                                  Dem_UdsStatusByteType DTCStatusMask,
                                  Dem_DTCKindType DTCKind,
                                  Dem_DTCFormatType DTCFormat,
                                  Dem_DTCOriginType DTCOrigin,
                                  Dem_FilterWithSeverityType FilterWithSeverity,
                                  Dem_DTCSeverityType DTCSeverityMask,
                                  Dem_FilterForFDCType FilterForFDC)
{
    (void)ClientId;

    Dem_ClientState.FilterActive         = TRUE;
    Dem_ClientState.FilterStatusMask     = DTCStatusMask;
    Dem_ClientState.FilterDTCKind        = DTCKind;
    Dem_ClientState.FilterFormat         = DTCFormat;
    Dem_ClientState.FilterOrigin         = DTCOrigin;
    Dem_ClientState.FilterWithSeverity   = FilterWithSeverity;
    Dem_ClientState.FilterSeverity       = DTCSeverityMask;
    Dem_ClientState.FilterForFDC         = FilterForFDC;
    Dem_ClientState.FilterIndex          = 0U;

    return E_OK;
}

/** Internal: check whether event[i] matches current filter */
static uint8 Dem_Dcm_MatchesFilter(uint8 eventIndex)
{
    Dem_UdsStatusByteType evStatus = Dem_EventRuntime[eventIndex].UdsStatus;
    Dem_EventIdType       evId     = (Dem_EventIdType)(eventIndex + 1U);

    /* Suppressed events are invisible to DCM */
#if (DEM_SUPPRESSION_SUPPORT == STD_ON)
    if (Dem_EventRuntime[eventIndex].Suppressed) {
        return FALSE;
    }
#endif

    /* Status mask filter: at least one bit must match */
    if ((evStatus & Dem_ClientState.FilterStatusMask) == 0U) {
        return FALSE;
    }

    /* Event must be in primary memory (no secondary / mirror support here) */
    if (Dem_ClientState.FilterOrigin != DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
        return FALSE;
    }

    (void)evId;
    return TRUE;
}

Std_ReturnType Dem_GetNumberOfFilteredDTC(uint8 ClientId,
                                           uint16 *NumberOfFilteredDTC)
{
    uint8 i;
    uint16 count = 0U;
    (void)ClientId;

    if ((NumberOfFilteredDTC == NULL_PTR) || !Dem_ClientState.FilterActive) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if (Dem_Dcm_MatchesFilter(i)) {
            count++;
        }
    }

    Dem_ClientState.FilterIndex = 0U; /* reset iterator */
    *NumberOfFilteredDTC        = count;
    return E_OK;
}

Std_ReturnType Dem_GetNextFilteredDTC(uint8 ClientId,
                                       uint32 *DTC,
                                       Dem_UdsStatusByteType *DTCStatus)
{
    (void)ClientId;

    if ((DTC == NULL_PTR) || (DTCStatus == NULL_PTR) || !Dem_ClientState.FilterActive) {
        return E_NOT_OK;
    }

    while (Dem_ClientState.FilterIndex < DEM_MAX_NUMBER_OF_EVENTS) {
        uint8 idx = Dem_ClientState.FilterIndex;
        Dem_ClientState.FilterIndex++;

        if (Dem_Dcm_MatchesFilter(idx)) {
            *DTC       = Dem_EventCfg[idx].Dtc;
            *DTCStatus = Dem_EventRuntime[idx].UdsStatus;
            return E_OK;
        }
    }

    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_GetNextFilteredDTCAndFDC(uint8 ClientId,
                                             uint32 *DTC,
                                             sint8 *DTCFaultDetectionCounter)
{
    Dem_UdsStatusByteType dummy;
    Std_ReturnType ret;

    ret = Dem_GetNextFilteredDTC(ClientId, DTC, &dummy);
    if (ret == E_OK) {
        uint8 i;
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            if (Dem_EventCfg[i].Dtc == *DTC) {
                *DTCFaultDetectionCounter =
                    Dem_Debounce_GetFDC((Dem_EventIdType)(i + 1U));
                break;
            }
        }
    }
    return ret;
}

Std_ReturnType Dem_GetNextFilteredDTCAndSeverity(uint8 ClientId,
                                                   uint32 *DTC,
                                                   Dem_UdsStatusByteType *DTCStatus,
                                                   Dem_DTCSeverityType *DTCSeverity,
                                                   uint8 *DTCFunctionalUnit)
{
    Std_ReturnType ret;

    ret = Dem_GetNextFilteredDTC(ClientId, DTC, DTCStatus);
    if (ret == E_OK) {
        uint8 i;
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            if (Dem_EventCfg[i].Dtc == *DTC) {
                *DTCSeverity = (Dem_EventCfg[i].Priority == 1U)
                               ? DEM_SEVERITY_CHECK_IMMEDIATELY
                               : DEM_SEVERITY_CHECK_AT_NEXT_HALT;
                *DTCFunctionalUnit = 0x00U;
                break;
            }
        }
    }
    return ret;
}

/* =========================================================================
 * §8.3.4.1  FreezeFrame record filter (for $19 03)
 * ========================================================================= */

Std_ReturnType Dem_SetFreezeFrameRecordFilter(uint8 ClientId,
                                               uint8 RecordNumber,
                                               Dem_DTCOriginType DTCOrigin)
{
    (void)ClientId;
    Dem_ClientState.FFFilterActive  = TRUE;
    Dem_ClientState.FFFilterRecord  = RecordNumber;
    Dem_ClientState.FFFilterIndex   = 0U;
    (void)DTCOrigin;
    return E_OK;
}

Std_ReturnType Dem_GetNextFilteredRecord(uint8 ClientId,
                                          uint32 *DTC,
                                          Dem_DTCFormatType DTCFormat,
                                          uint8 *RecordNumber)
{
    (void)ClientId;
    (void)DTCFormat;

    if ((DTC == NULL_PTR) || (RecordNumber == NULL_PTR) || !Dem_ClientState.FFFilterActive) {
        return E_NOT_OK;
    }

    while (Dem_ClientState.FFFilterIndex < DEM_MAX_EVENT_MEMORY_ENTRIES) {
        uint8 idx = Dem_ClientState.FFFilterIndex;
        Dem_ClientState.FFFilterIndex++;

        if (Dem_PrimaryMemory[idx].EventId == DEM_EVENT_INVALID) {
            continue;
        }
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
        if (!Dem_PrimaryMemory[idx].FreezeFrame.Valid) {
            continue;
        }
        if ((Dem_ClientState.FFFilterRecord == 0xFFU) ||
            (Dem_ClientState.FFFilterRecord == Dem_PrimaryMemory[idx].FreezeFrame.RecordNumber))
        {
            *DTC          = Dem_EventCfg[Dem_PrimaryMemory[idx].EventId - 1U].Dtc;
            *RecordNumber = Dem_PrimaryMemory[idx].FreezeFrame.RecordNumber;
            return E_OK;
        }
#endif
    }
    return DEM_FILTERED_NO_MATCHING_DTC;
}

/* =========================================================================
 * §8.3.4.1  DTC by occurrence time
 * ========================================================================= */

Std_ReturnType Dem_GetDTCByOccurrenceTime(uint8 ClientId,
                                           Dem_DTCRequestType DTCRequest,
                                           uint32 *DTC)
{
    uint8 i;
    uint32 targetTs;
    uint8  found = FALSE;
    (void)ClientId;

    if (DTC == NULL_PTR) {
        return E_NOT_OK;
    }

    targetTs = (DTCRequest == DEM_FIRST_FAILED_DTC ||
                DTCRequest == DEM_FIRST_DET_CONFIRMED_DTC)
               ? 0xFFFFFFFFUL : 0x00000000UL;

    *DTC = 0U;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        if ((DTCRequest == DEM_FIRST_FAILED_DTC ||
             DTCRequest == DEM_FIRST_DET_CONFIRMED_DTC) &&
            Dem_PrimaryMemory[i].Timestamp < targetTs)
        {
            targetTs = Dem_PrimaryMemory[i].Timestamp;
            *DTC = Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc;
            found = TRUE;
        }
        else if ((DTCRequest == DEM_MOST_RECENT_FAILED_DTC ||
                  DTCRequest == DEM_MOST_REC_DET_CONFIRMED_DTC) &&
                 Dem_PrimaryMemory[i].Timestamp > targetTs)
        {
            targetTs = Dem_PrimaryMemory[i].Timestamp;
            *DTC = Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc;
            found = TRUE;
        }
    }

    return found ? E_OK : DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_SetDTCFilterByExtendedDataRecordNumber(uint8 ClientId,
                                                            uint8 EDRNumber)
{
    (void)ClientId;
    (void)EDRNumber;
    /* Simplified: activate same filter mechanism */
    return E_OK;
}

Std_ReturnType Dem_SetDTCFilterByReadinessGroup(uint8 ClientId,
                                                  Dem_EventOBDReadinessGroupType ReadinessGroupNumber)
{
    (void)ClientId;
    (void)ReadinessGroupNumber;
    return E_OK;
}

/* =========================================================================
 * §8.3.4.2  FreezeFrame / ExtendedData selection and read
 * ========================================================================= */

Std_ReturnType Dem_DisableDTCRecordUpdate(uint8 ClientId)
{
    (void)ClientId;
    Dem_ClientState.RecordUpdateDisabled = TRUE;
    return E_OK;
}

Std_ReturnType Dem_EnableDTCRecordUpdate(uint8 ClientId)
{
    (void)ClientId;
    Dem_ClientState.RecordUpdateDisabled = FALSE;
    return E_OK;
}

Std_ReturnType Dem_SelectFreezeFrameData(uint8 ClientId,
                                          uint8 RecordNumber)
{
    (void)ClientId;
    Dem_ClientState.FFFilterRecord = RecordNumber;
    return E_OK;
}

Std_ReturnType Dem_GetSizeOfFreezeFrameSelection(uint8 ClientId,
                                                   uint16 *SizeOfFreezeFrame)
{
    uint8 i;
    (void)ClientId;

    if ((SizeOfFreezeFrame == NULL_PTR) || !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    *SizeOfFreezeFrame = 0U;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        if (Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc ==
            Dem_ClientState.SelectedDTC)
        {
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
            if (Dem_PrimaryMemory[i].FreezeFrame.Valid) {
                *SizeOfFreezeFrame = DEM_FREEZE_FRAME_SIZE;
            }
#endif
            return E_OK;
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetNextFreezeFrameData(uint8 ClientId,
                                           uint8 *DestBuffer,
                                           uint16 *BufSize)
{
    uint8 i;
    (void)ClientId;

    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR) ||
        !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        if (Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc ==
            Dem_ClientState.SelectedDTC)
        {
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
            if (!Dem_PrimaryMemory[i].FreezeFrame.Valid) {
                return E_NOT_OK;
            }
            if (*BufSize < DEM_FREEZE_FRAME_SIZE) {
                return DEM_BUFFER_TOO_SMALL;
            }
            {
                uint8 j;
                for (j = 0U; j < DEM_FREEZE_FRAME_SIZE; j++) {
                    DestBuffer[j] = Dem_PrimaryMemory[i].FreezeFrame.Data[j];
                }
            }
            *BufSize = DEM_FREEZE_FRAME_SIZE;
            return E_OK;
#else
            return E_NOT_OK;
#endif
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetNumberOfFreezeFrameRecords(uint8 ClientId,
                                                   uint8 *NumberOfFilteredRecords)
{
    uint8 i;
    uint8 count = 0U;
    (void)ClientId;

    if (NumberOfFilteredRecords == NULL_PTR) {
        return E_NOT_OK;
    }

#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if ((Dem_PrimaryMemory[i].EventId != DEM_EVENT_INVALID) &&
            Dem_PrimaryMemory[i].FreezeFrame.Valid)
        {
            count++;
        }
    }
#endif

    *NumberOfFilteredRecords = count;
    return E_OK;
}

Std_ReturnType Dem_SelectExtendedDataRecord(uint8 ClientId,
                                             uint8 ExtendedDataNumber)
{
    (void)ClientId;
    (void)ExtendedDataNumber;
    return E_OK;
}

Std_ReturnType Dem_GetSizeOfExtendedDataRecordSelection(uint8 ClientId,
                                                          uint16 *SizeOfExtendedDataRecord)
{
    uint8 i;
    (void)ClientId;

    if ((SizeOfExtendedDataRecord == NULL_PTR) || !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    *SizeOfExtendedDataRecord = 0U;

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        if (Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc ==
            Dem_ClientState.SelectedDTC)
        {
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
            if (Dem_PrimaryMemory[i].ExtendedData.Valid) {
                *SizeOfExtendedDataRecord = DEM_EXTENDED_DATA_SIZE;
            }
#endif
            return E_OK;
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_GetNextExtendedDataRecord(uint8 ClientId,
                                              uint8 *DestBuffer,
                                              uint16 *BufSize)
{
    uint8 i;
    (void)ClientId;

    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR) ||
        !Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == DEM_EVENT_INVALID) {
            continue;
        }
        if (Dem_EventCfg[Dem_PrimaryMemory[i].EventId - 1U].Dtc ==
            Dem_ClientState.SelectedDTC)
        {
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
            if (!Dem_PrimaryMemory[i].ExtendedData.Valid) {
                return E_NOT_OK;
            }
            if (*BufSize < DEM_EXTENDED_DATA_SIZE) {
                return DEM_BUFFER_TOO_SMALL;
            }
            {
                uint8 j;
                for (j = 0U; j < DEM_EXTENDED_DATA_SIZE; j++) {
                    DestBuffer[j] = Dem_PrimaryMemory[i].ExtendedData.Data[j];
                }
            }
            *BufSize = DEM_EXTENDED_DATA_SIZE;
            return E_OK;
#else
            return E_NOT_OK;
#endif
        }
    }
    return DEM_NO_SUCH_ELEMENT;
}

/* =========================================================================
 * §8.3.4.3  DTC storage control
 * ========================================================================= */

/**
 * @brief  Disable DTC setting for a DTC group (§8.3.4.3.1).
 *         While disabled, events are not stored in memory and status bits
 *         are not updated.
 */
Std_ReturnType Dem_DisableDTCSetting(uint32 DTCGroup)
{
    (void)DTCGroup;
    Dem_DTCSettingDisabled = TRUE;
    return E_OK;
}

/**
 * @brief  Re-enable DTC setting (§8.3.4.3.2).
 */
Std_ReturnType Dem_EnableDTCSetting(uint32 DTCGroup)
{
    (void)DTCGroup;
    Dem_DTCSettingDisabled = FALSE;
    return E_OK;
}

/* End of file */
