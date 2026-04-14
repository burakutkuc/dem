/**
 * @file    Dem.c
 * @brief   AUTOSAR CP R23-11 - Diagnostic Event Manager core implementation.
 *
 *  Implements:
 *    - §8.3.2  Lifecycle (PreInit, Init, Shutdown)
 *    - §8.3.3  BSW/SWC interface (SetEventStatus, Get/Set status, ClearDTC, ...)
 *    - §8.5    Dem_MainFunction (debounce tick, aging, healing)
 *    - §7.7.1  UDS status byte transitions
 */

#include "Dem_Internal.h"
#include "Dem_Cbk.h"

/* Forward declarations of helpers defined at the bottom of this file */
static void Dem_Internal_ProcessQualifiedStatus(Dem_EventIdType EventId,
                                                 Dem_EventStatusType qualStatus);

/* =========================================================================
 * Global state variables
 * ========================================================================= */
Dem_StateType           Dem_State                              = DEM_UNINITIALIZED;
Dem_EventRuntimeType    Dem_EventRuntime[DEM_MAX_NUMBER_OF_EVENTS];
Dem_MemoryEntryType     Dem_PrimaryMemory[DEM_MAX_EVENT_MEMORY_ENTRIES];
Dem_ClientStateType     Dem_ClientState;
Dem_NvDataType          Dem_NvData;
uint8                   Dem_EnableConditions[DEM_MAX_ENABLE_CONDITIONS];
uint8                   Dem_StorageConditions[DEM_MAX_STORAGE_CONDITIONS];
uint8                   Dem_DTCSettingDisabled                 = FALSE;
uint32                  Dem_TimestampCounter                   = 0U;

/* =========================================================================
 * Forward declarations (implemented in other .c files)
 * ========================================================================= */
extern Dem_EventStatusType  Dem_Debounce_ProcessStatus(Dem_EventIdType, Dem_EventStatusType);
extern void                 Dem_Debounce_Reset(Dem_EventIdType, Dem_DebounceResetStatusType);
extern sint8                Dem_Debounce_GetFDC(Dem_EventIdType);
extern Dem_DebouncingStateType Dem_Debounce_GetState(Dem_EventIdType);
extern uint8                Dem_Memory_StoreEvent(Dem_EventIdType);
extern void                 Dem_Memory_ClearEntry(Dem_EventIdType);
extern void                 Dem_Memory_ClearAll(void);
extern boolean              Dem_Memory_IsOverflow(void);
extern uint8                Dem_Memory_GetNumberOfEntries(void);
extern void                 Dem_Memory_UpdateAging(void);
extern Std_ReturnType       Dem_Nvm_Init(void);
extern void                 Dem_Nvm_Shutdown(void);

/* =========================================================================
 * §7.7.1  UDS Status Byte helpers
 * ========================================================================= */

/** All status bytes are supported by this implementation. */
#define DEM_UDS_STATUS_AVAILABILITY_MASK    (0xFFU)

/**
 * @brief  Update UDS status byte on FAILED qualification (§7.7.1.3).
 *   TF=1, TFTOC=1, TFSLC=1, TNCTOC=0, TNCLC=0
 */
void Dem_Internal_UpdateUdsStatusFailed(Dem_EventIdType EventId)
{
    Dem_UdsStatusByteType oldSt = Dem_EventRuntime[EventId - 1U].UdsStatus;
    Dem_MonitorStatusType oldMon = Dem_EventRuntime[EventId - 1U].MonitorStatus;
    Dem_UdsStatusByteType *st = &Dem_EventRuntime[EventId - 1U].UdsStatus;

    *st |=  (DEM_UDS_STATUS_TF | DEM_UDS_STATUS_TFTOC |
             DEM_UDS_STATUS_TFSLC);
    *st &=  (Dem_UdsStatusByteType)(~(DEM_UDS_STATUS_TNCTOC | DEM_UDS_STATUS_TNCLC));

    /* Monitor status */
    Dem_EventRuntime[EventId - 1U].MonitorStatus |= DEM_MONITOR_STATUS_TF;
    Dem_EventRuntime[EventId - 1U].MonitorStatus &=
        (Dem_MonitorStatusType)(~DEM_MONITOR_STATUS_TNCTOC);

#if (DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED == STD_ON)
    if (oldSt != *st) {
        DemTriggerOnEventUdsStatus(EventId, oldSt, *st);
    }
#endif
#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
    if (oldSt != *st) {
        DemGeneralTriggerOnEventUdsStatus(EventId, oldSt, *st);
    }
#endif
#if (DEM_CB_ENABLE_MONITOR_STATUS_CHANGED == STD_ON)
    if (oldMon != Dem_EventRuntime[EventId - 1U].MonitorStatus) {
        DemTriggerOnMonitorStatus(EventId, oldMon, Dem_EventRuntime[EventId - 1U].MonitorStatus);
    }
#endif
#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
    if (oldMon != Dem_EventRuntime[EventId - 1U].MonitorStatus) {
        DemGeneralTriggerOnMonitorStatus(EventId, oldMon, Dem_EventRuntime[EventId - 1U].MonitorStatus);
    }
#endif
}

/**
 * @brief  Update UDS status byte on PASSED qualification (§7.7.1.3).
 *   TF=0, TNCTOC=0
 */
void Dem_Internal_UpdateUdsStatusPassed(Dem_EventIdType EventId)
{
    Dem_UdsStatusByteType oldSt = Dem_EventRuntime[EventId - 1U].UdsStatus;
    Dem_MonitorStatusType oldMon = Dem_EventRuntime[EventId - 1U].MonitorStatus;
    Dem_UdsStatusByteType *st = &Dem_EventRuntime[EventId - 1U].UdsStatus;

    *st &= (Dem_UdsStatusByteType)(~(DEM_UDS_STATUS_TF | DEM_UDS_STATUS_TNCTOC));

    Dem_EventRuntime[EventId - 1U].MonitorStatus &=
        (Dem_MonitorStatusType)(~(DEM_MONITOR_STATUS_TF | DEM_MONITOR_STATUS_TNCTOC));

#if (DEM_CB_ENABLE_EVENT_UDS_STATUS_CHANGED == STD_ON)
    if (oldSt != *st) {
        DemTriggerOnEventUdsStatus(EventId, oldSt, *st);
    }
#endif
#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
    if (oldSt != *st) {
        DemGeneralTriggerOnEventUdsStatus(EventId, oldSt, *st);
    }
#endif
#if (DEM_CB_ENABLE_MONITOR_STATUS_CHANGED == STD_ON)
    if (oldMon != Dem_EventRuntime[EventId - 1U].MonitorStatus) {
        DemTriggerOnMonitorStatus(EventId, oldMon, Dem_EventRuntime[EventId - 1U].MonitorStatus);
    }
#endif
#if (DEM_CB_ENABLE_GENERAL_INTERFACE == STD_ON)
    if (oldMon != Dem_EventRuntime[EventId - 1U].MonitorStatus) {
        DemGeneralTriggerOnMonitorStatus(EventId, oldMon, Dem_EventRuntime[EventId - 1U].MonitorStatus);
    }
#endif
}

/**
 * @brief  Set PDTC and CDTC when confirmation threshold is reached (§7.7.4).
 */
void Dem_Internal_UpdateUdsStatusConfirmed(Dem_EventIdType EventId)
{
    Dem_EventRuntime[EventId - 1U].UdsStatus |=
        (DEM_UDS_STATUS_PDTC | DEM_UDS_STATUS_CDTC);
}

/**
 * @brief  Operation cycle end transitions (§7.7.1.3):
 *   TFTOC=0, TNCTOC=1
 */
void Dem_Internal_OperationCycleEnd(void)
{
    uint8 i;
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_UdsStatusByteType *st = &Dem_EventRuntime[i].UdsStatus;
        *st &= (Dem_UdsStatusByteType)(~DEM_UDS_STATUS_TFTOC);
        *st |= DEM_UDS_STATUS_TNCTOC;
    }
    Dem_Memory_UpdateAging();
}

/* =========================================================================
 * Enable / Storage condition checks
 * ========================================================================= */

uint8 Dem_Internal_EnableConditionsMet(Dem_EventIdType EventId)
{
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
    uint8 mask = Dem_EventCfg[EventId - 1U].EnableCondMask;
    uint8 i;
    for (i = 0U; i < DEM_MAX_ENABLE_CONDITIONS; i++) {
        if ((mask & (1U << i)) != 0U) {
            if (Dem_EnableConditions[i] == FALSE) {
                return FALSE;
            }
        }
    }
    return TRUE;
#else
    (void)EventId;
    return TRUE;
#endif
}

uint8 Dem_Internal_StorageConditionsMet(Dem_EventIdType EventId)
{
#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
    uint8 mask = Dem_EventCfg[EventId - 1U].StorageCondMask;
    uint8 i;
    for (i = 0U; i < DEM_MAX_STORAGE_CONDITIONS; i++) {
        if ((mask & (1U << i)) != 0U) {
            if (Dem_StorageConditions[i] == FALSE) {
                return FALSE;
            }
        }
    }
    return TRUE;
#else
    (void)EventId;
    return TRUE;
#endif
}

/* =========================================================================
 * Simple XOR CRC for NvM validity check
 * ========================================================================= */

uint16 Dem_Internal_CalcCrc(const uint8 *data, uint16 length)
{
    uint16 crc = 0xFFFFU;
    uint16 i;
    for (i = 0U; i < length; i++) {
        crc ^= (uint16)data[i];
    }
    return crc;
}

/* =========================================================================
 * §8.3.1  Dem_GetVersionInfo
 * ========================================================================= */

void Dem_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo != NULL_PTR) {
        versioninfo->vendorID         = DEM_VENDOR_ID;
        versioninfo->moduleID         = DEM_MODULE_ID;
        versioninfo->sw_major_version = DEM_SW_MAJOR_VERSION;
        versioninfo->sw_minor_version = DEM_SW_MINOR_VERSION;
        versioninfo->sw_patch_version = DEM_SW_PATCH_VERSION;
    }
}

/* =========================================================================
 * §8.3.2  Lifecycle
 * ========================================================================= */

/**
 * @brief  Pre-initialise the DEM (§8.3.2.1).
 *         Call before NvM read is complete; only RAM structures are zeroed.
 */
void Dem_PreInit(void)
{
    uint8 i;
    uint8 j;

    /* Zero all runtime event state */
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_EventRuntime[i].UdsStatus      = DEM_UDS_STATUS_DEFAULT;
        Dem_EventRuntime[i].MonitorStatus  = DEM_MONITOR_STATUS_TNCTOC;
        Dem_EventRuntime[i].Debounce.Counter     = 0;
        Dem_EventRuntime[i].Debounce.TimeCounter = 0U;
        Dem_EventRuntime[i].Debounce.Direction   = 0U;
        Dem_EventRuntime[i].Available      = TRUE;
        Dem_EventRuntime[i].Suppressed     = FALSE;
        Dem_EventRuntime[i].ConfirmCounter = 0U;
        Dem_EventRuntime[i].HealingCounter = 0U;
    }

    /* Zero all primary memory slots */
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        Dem_PrimaryMemory[i].EventId = DEM_EVENT_INVALID;
        Dem_PrimaryMemory[i].UdsStatus          = 0U;
        Dem_PrimaryMemory[i].OccurrenceCounter  = 0U;
        Dem_PrimaryMemory[i].AgingCounter       = 0U;
        Dem_PrimaryMemory[i].FailedCycleCounter = 0U;
        Dem_PrimaryMemory[i].ConsecutiveFailed  = 0U;
        Dem_PrimaryMemory[i].Timestamp          = 0U;
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
        Dem_PrimaryMemory[i].FreezeFrame.Valid = FALSE;
        for (j = 0U; j < DEM_FREEZE_FRAME_SIZE; j++) {
            Dem_PrimaryMemory[i].FreezeFrame.Data[j] = 0U;
        }
#endif
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
        Dem_PrimaryMemory[i].ExtendedData.Valid = FALSE;
        for (j = 0U; j < DEM_EXTENDED_DATA_SIZE; j++) {
            Dem_PrimaryMemory[i].ExtendedData.Data[j] = 0U;
        }
#endif
    }

    /* Conditions all fulfilled by default */
    for (i = 0U; i < DEM_MAX_ENABLE_CONDITIONS; i++) {
        Dem_EnableConditions[i] = TRUE;
    }
    for (i = 0U; i < DEM_MAX_STORAGE_CONDITIONS; i++) {
        Dem_StorageConditions[i] = TRUE;
    }

    Dem_DTCSettingDisabled = FALSE;
    Dem_TimestampCounter   = 0U;
    Dem_State              = DEM_PREINIT;

    (void)j;
}

/**
 * @brief  Full initialisation (§8.3.2.2). Restores NvM data.
 *         Call after NvM read has completed.
 */
void Dem_Init(const Dem_ConfigType *configPtr)
{
    (void)configPtr;

    if (Dem_State < DEM_PREINIT) {
        Dem_PreInit(); /* safety */
    }

    /* Restore NvM data */
    (void)Dem_Nvm_Init();

#if (DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT == STD_ON)
    /* Notify upper layer to initialise each monitor */
    {
        uint8 i;
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            InitMonitorForEvent((Dem_EventIdType)(i + 1U), DEM_INIT_MONITOR_RESTART);
        }
    }
#endif

    Dem_State = DEM_INITIALIZED;
}

/**
 * @brief  Shutdown: persist data to NvM (§8.3.2.3).
 */
void Dem_Shutdown(void)
{
    Dem_Nvm_Shutdown();
    Dem_State = DEM_UNINITIALIZED;
}

/* =========================================================================
 * §8.3.3  BSW / SW-C Interface — SetEventStatus
 * ========================================================================= */

/**
 * @brief  Report a diagnostic monitor status to the DEM (§8.3.3.29).
 *
 *  Flow:
 *   1. Validate parameters and module state.
 *   2. Check enable conditions; if not met, ignore.
 *   3. Run debounce algorithm.
 *   4. If qualified (FAILED / PASSED): update UDS status byte, handle
 *      confirmation, store/update memory entry.
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId,
                                   Dem_EventStatusType EventStatus)
{
    Dem_EventStatusType qualStatus;

    if (Dem_State != DEM_INITIALIZED) {
        return E_NOT_OK;
    }
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (!Dem_EventRuntime[EventId - 1U].Available) {
        return E_NOT_OK;
    }
    if (!Dem_Internal_EnableConditionsMet(EventId)) {
        return E_NOT_OK;
    }

    /* Debounce */
    qualStatus = Dem_Debounce_ProcessStatus(EventId, EventStatus);

    /* Process the qualified result */
    Dem_Internal_ProcessQualifiedStatus(EventId, qualStatus);

    return E_OK;
}

/**
 * @brief  Same as Dem_SetEventStatus but with optional monitor data (§8.3.3.30).
 */
Std_ReturnType Dem_SetEventStatusWithMonitorData(Dem_EventIdType EventId,
                                                  Dem_EventStatusType EventStatus,
                                                  Dem_MonitorDataType MonitorData)
{
    (void)MonitorData; /* stored in extended data by application-specific callback */
    return Dem_SetEventStatus(EventId, EventStatus);
}

/**
 * @brief  Internal: apply qualified status → UDS bits → memory.
 */
static void Dem_Internal_ProcessQualifiedStatus(Dem_EventIdType EventId,
                                                 Dem_EventStatusType qualStatus)
{
    const Dem_EventCfgType *cfg = &Dem_EventCfg[EventId - 1U];
    Dem_EventRuntimeType   *rt  = &Dem_EventRuntime[EventId - 1U];

    if (qualStatus == DEM_EVENT_STATUS_FAILED) {
        Dem_Internal_UpdateUdsStatusFailed(EventId);

        /* Confirmation counter */
        if (rt->ConfirmCounter < cfg->ConfirmThreshold) {
            rt->ConfirmCounter++;
        }
        if (rt->ConfirmCounter >= cfg->ConfirmThreshold) {
            Dem_Internal_UpdateUdsStatusConfirmed(EventId);
        }

        rt->HealingCounter = 0U;

        /* Store / update memory */
        if ((rt->UdsStatus & DEM_UDS_STATUS_CDTC) != 0U) {
            Dem_Memory_StoreEvent(EventId);
        }
    }
    else if (qualStatus == DEM_EVENT_STATUS_PASSED) {
        Dem_Internal_UpdateUdsStatusPassed(EventId);
        rt->ConfirmCounter = 0U;

#if (DEM_WIR_SUPPORT == STD_ON)
        /* Healing counter for WIR clearing */
        if (rt->HealingCounter < cfg->AgingThreshold) {
            rt->HealingCounter++;
        }
        if (rt->HealingCounter >= cfg->AgingThreshold) {
            rt->UdsStatus &= (Dem_UdsStatusByteType)(~DEM_UDS_STATUS_WIR);
        }
#endif
    }
    /* PREFAILED / PREPASSED: debounce in progress; no UDS changes */
}

/* =========================================================================
 * §8.3.3  Get event status / debounce info
 * ========================================================================= */

Std_ReturnType Dem_GetEventUdsStatus(Dem_EventIdType EventId,
                                      Dem_UdsStatusByteType *UDSStatusByte)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (UDSStatusByte == NULL_PTR) {
        return E_NOT_OK;
    }
    *UDSStatusByte = Dem_EventRuntime[EventId - 1U].UdsStatus;
    return E_OK;
}

Std_ReturnType Dem_GetMonitorStatus(Dem_EventIdType EventId,
                                     Dem_MonitorStatusType *MonitorStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (MonitorStatus == NULL_PTR) {
        return E_NOT_OK;
    }
    *MonitorStatus = Dem_EventRuntime[EventId - 1U].MonitorStatus;
    return E_OK;
}

Std_ReturnType Dem_GetDebouncingOfEvent(Dem_EventIdType EventId,
                                         Dem_DebouncingStateType *DebouncingState)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (DebouncingState == NULL_PTR) {
        return E_NOT_OK;
    }
    *DebouncingState = Dem_Debounce_GetState(EventId);
    return E_OK;
}

Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType EventId,
                                             sint8 *FaultDetectionCounter)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (FaultDetectionCounter == NULL_PTR) {
        return E_NOT_OK;
    }
    *FaultDetectionCounter = Dem_Debounce_GetFDC(EventId);
    return E_OK;
}

/* =========================================================================
 * §8.3.3  Reset functions
 * ========================================================================= */

Std_ReturnType Dem_ResetEventDebounceStatus(Dem_EventIdType EventId,
                                             Dem_DebounceResetStatusType DebounceResetStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_Debounce_Reset(EventId, DebounceResetStatus);
    return E_OK;
}

Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType EventId)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    /* Clear TF; other bits remain */
    Dem_EventRuntime[EventId - 1U].UdsStatus &=
        (Dem_UdsStatusByteType)(~DEM_UDS_STATUS_TF);
    Dem_Debounce_Reset(EventId, DEM_DEBOUNCE_STATUS_RESET);
    return E_OK;
}

Std_ReturnType Dem_ResetMonitorStatus(Dem_EventIdType EventId)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_EventRuntime[EventId - 1U].MonitorStatus =
        (Dem_MonitorStatusType)(DEM_MONITOR_STATUS_TNCTOC);
    return E_OK;
}

/* =========================================================================
 * §8.3.3  Operation cycle (§7.6)
 * ========================================================================= */

Std_ReturnType Dem_RestartOperationCycle(Dem_OperationCycleIdType OperationCycleId)
{
    if (Dem_State == DEM_UNINITIALIZED) {
        return E_NOT_OK;
    }
    if (OperationCycleId >= DEM_NUM_OPERATION_CYCLES) {
        return E_NOT_OK;
    }
    Dem_Internal_OperationCycleEnd();

#if (DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT == STD_ON)
    /* Operation cycle restart implies re-init of monitor status */
    {
        uint8 i;
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            InitMonitorForEvent((Dem_EventIdType)(i + 1U), DEM_INIT_MONITOR_RESTART);
        }
    }
#endif

    return E_OK;
}

/* =========================================================================
 * §8.3.3  DTC selection and clearing
 * ========================================================================= */

Std_ReturnType Dem_SelectDTC(uint8 ClientId,
                               uint32 DTC,
                               Dem_DTCFormatType DTCFormat,
                               Dem_DTCOriginType DTCOrigin)
{
    (void)ClientId;
    Dem_ClientState.SelectedDTC      = DTC;
    Dem_ClientState.SelectedFormat   = DTCFormat;
    Dem_ClientState.SelectedOrigin   = DTCOrigin;
    Dem_ClientState.SelectionValid   = TRUE;
    Dem_ClientState.ClearPending     = FALSE;
    return E_OK;
}

/**
 * @brief  Clear the selected DTC or DTC group (§7.7.2.2, §8.3.3.1).
 *
 *  Returns E_OK when done, DEM_PENDING while NvM write is ongoing.
 */
Std_ReturnType Dem_ClearDTC(uint8 ClientId)
{
    uint8 i;
    (void)ClientId;

    if (!Dem_ClientState.SelectionValid) {
        return E_NOT_OK;
    }

    Dem_ClientState.ClearPending = TRUE;

    if (Dem_ClientState.SelectedDTC == DEM_DTC_GROUP_ALL_DTCS) {
        /* Clear all */
#if (DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED == STD_ON)
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            if (ClearEventAllowed((Dem_EventIdType)(i + 1U)) != E_OK) {
                Dem_ClientState.ClearPending = FALSE;
                return E_NOT_OK;
            }
        }
#endif
        Dem_Memory_ClearAll();
#if (DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION == STD_ON)
        ClearDtcNotification(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
    } else {
        /* Clear single DTC: find matching EventId */
        for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
            if (Dem_EventCfg[i].Dtc == Dem_ClientState.SelectedDTC) {
#if (DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED == STD_ON)
                if (ClearEventAllowed((Dem_EventIdType)(i + 1U)) != E_OK) {
                    Dem_ClientState.ClearPending = FALSE;
                    return E_NOT_OK;
                }
#endif
                Dem_Memory_ClearEntry((Dem_EventIdType)(i + 1U));
#if (DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION == STD_ON)
                ClearDtcNotification(Dem_ClientState.SelectedDTC, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
                break;
            }
        }
    }

    Dem_TimestampCounter++;
    Dem_ClientState.ClearPending    = FALSE;
    Dem_ClientState.SelectionValid  = FALSE;
    return E_OK;
}

Std_ReturnType Dem_GetDTCSelectionResult(uint8 ClientId)
{
    (void)ClientId;
    return Dem_ClientState.ClearPending ? DEM_PENDING : E_OK;
}

Std_ReturnType Dem_GetDTCSelectionResultForClearDTC(uint8 ClientId)
{
    (void)ClientId;
    return Dem_ClientState.ClearPending ? DEM_PENDING : E_OK;
}

/* =========================================================================
 * §8.3.3  DTC of event
 * ========================================================================= */

Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType EventId,
                                  Dem_DTCFormatType DTCFormat,
                                  uint32 *DTCOfEvent)
{
    (void)DTCFormat;
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (DTCOfEvent == NULL_PTR) {
        return E_NOT_OK;
    }
    *DTCOfEvent = Dem_EventCfg[EventId - 1U].Dtc;
    return E_OK;
}

/* =========================================================================
 * §8.3.3  Memory overflow / entry count
 * ========================================================================= */

Std_ReturnType Dem_GetEventMemoryOverflow(Dem_DTCOriginType DTCOrigin,
                                           boolean *isOverflow)
{
    (void)DTCOrigin;
    if (isOverflow == NULL_PTR) {
        return E_NOT_OK;
    }
    *isOverflow = Dem_Memory_IsOverflow();
    return E_OK;
}

Std_ReturnType Dem_GetNumberOfEventMemoryEntries(Dem_DTCOriginType DTCOrigin,
                                                   uint8 *NumberOfEventMemoryEntries)
{
    (void)DTCOrigin;
    if (NumberOfEventMemoryEntries == NULL_PTR) {
        return E_NOT_OK;
    }
    *NumberOfEventMemoryEntries = Dem_Memory_GetNumberOfEntries();
    return E_OK;
}

/* =========================================================================
 * §8.3.3  Event availability
 * ========================================================================= */

Std_ReturnType Dem_GetEventAvailable(Dem_EventIdType EventId,
                                      boolean *AvailableStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (AvailableStatus == NULL_PTR) {
        return E_NOT_OK;
    }
    *AvailableStatus = (boolean)Dem_EventRuntime[EventId - 1U].Available;
    return E_OK;
}

Std_ReturnType Dem_SetEventAvailable(Dem_EventIdType EventId,
                                      boolean AvailableStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_EventRuntime[EventId - 1U].Available = (uint8)AvailableStatus;
    return E_OK;
}

Std_ReturnType Dem_SetEventConfirmationThresholdCounter(Dem_EventIdType EventId,
                                                         uint8 ThresholdCounter)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_EventRuntime[EventId - 1U].ConfirmCounter = ThresholdCounter;
    return E_OK;
}

/* =========================================================================
 * §8.3.3  Enable / Storage conditions
 * ========================================================================= */

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID,
                                       boolean ConditionFulfilled)
{
    if (EnableConditionID >= DEM_MAX_ENABLE_CONDITIONS) {
        return E_NOT_OK;
    }
    Dem_EnableConditions[EnableConditionID] = (uint8)ConditionFulfilled;
    return E_OK;
}
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetStorageCondition(uint8 StorageConditionID,
                                        boolean ConditionFulfilled)
{
    if (StorageConditionID >= DEM_MAX_STORAGE_CONDITIONS) {
        return E_NOT_OK;
    }
    Dem_StorageConditions[StorageConditionID] = (uint8)ConditionFulfilled;
    return E_OK;
}
#endif

/* =========================================================================
 * §8.3.3  DTC Suppression
 * ========================================================================= */

#if (DEM_SUPPRESSION_SUPPORT == STD_ON)
Std_ReturnType Dem_GetDTCSuppression(Dem_EventIdType EventId,
                                      boolean *SuppressionStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (SuppressionStatus == NULL_PTR) {
        return E_NOT_OK;
    }
    *SuppressionStatus = (boolean)Dem_EventRuntime[EventId - 1U].Suppressed;
    return E_OK;
}

Std_ReturnType Dem_SetDTCSuppression(Dem_EventIdType EventId,
                                      boolean SuppressionStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_EventRuntime[EventId - 1U].Suppressed = (uint8)SuppressionStatus;
    return E_OK;
}
#endif

/* =========================================================================
 * §8.3.3  Warning Indicator Requested (WIR)
 * ========================================================================= */

#if (DEM_WIR_SUPPORT == STD_ON)
Std_ReturnType Dem_SetWIRStatus(Dem_EventIdType EventId, boolean WIRStatus)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if (WIRStatus) {
        Dem_EventRuntime[EventId - 1U].UdsStatus |= DEM_UDS_STATUS_WIR;
    } else {
        Dem_EventRuntime[EventId - 1U].UdsStatus &=
            (Dem_UdsStatusByteType)(~DEM_UDS_STATUS_WIR);
    }
    return E_OK;
}

Std_ReturnType Dem_GetIndicatorStatus(uint8 IndicatorId,
                                       Dem_IndicatorStatusType *IndicatorStatus)
{
    uint8 i;
    (void)IndicatorId;
    if (IndicatorStatus == NULL_PTR) {
        return E_NOT_OK;
    }
    *IndicatorStatus = DEM_INDICATOR_OFF;
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        if ((Dem_EventRuntime[i].UdsStatus & DEM_UDS_STATUS_WIR) != 0U) {
            *IndicatorStatus = DEM_INDICATOR_CONTINUOUS;
            break;
        }
    }
    return E_OK;
}
#endif

/* =========================================================================
 * §8.3.3  FreezeFrame / ExtendedData direct access
 * ========================================================================= */

#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
Std_ReturnType Dem_GetEventFreezeFrameDataEx(Dem_EventIdType EventId,
                                              uint8 RecordNumber,
                                              uint16 DataIdentifier,
                                              uint8 *DestBuffer,
                                              uint16 *BufSize)
{
    uint8 i;
    (void)RecordNumber;
    (void)DataIdentifier;

    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) {
        return E_NOT_OK;
    }

    /* Find memory entry for this event */
    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == EventId) {
            if (!Dem_PrimaryMemory[i].FreezeFrame.Valid) {
                return E_NOT_OK;
            }
            if (*BufSize < DEM_FREEZE_FRAME_SIZE) {
                return DEM_BUFFER_TOO_SMALL;
            }
            uint8 j;
            for (j = 0U; j < DEM_FREEZE_FRAME_SIZE; j++) {
                DestBuffer[j] = Dem_PrimaryMemory[i].FreezeFrame.Data[j];
            }
            *BufSize = DEM_FREEZE_FRAME_SIZE;
            return E_OK;
        }
    }
    return E_NOT_OK;
}
#endif

#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
Std_ReturnType Dem_GetEventExtendedDataRecordEx(Dem_EventIdType EventId,
                                                 uint8 RecordNumber,
                                                 uint8 *DestBuffer,
                                                 uint16 *BufSize)
{
    uint8 i;
    (void)RecordNumber;

    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) {
        return E_NOT_OK;
    }

    for (i = 0U; i < DEM_MAX_EVENT_MEMORY_ENTRIES; i++) {
        if (Dem_PrimaryMemory[i].EventId == EventId) {
            if (!Dem_PrimaryMemory[i].ExtendedData.Valid) {
                return E_NOT_OK;
            }
            if (*BufSize < DEM_EXTENDED_DATA_SIZE) {
                return DEM_BUFFER_TOO_SMALL;
            }
            uint8 j;
            for (j = 0U; j < DEM_EXTENDED_DATA_SIZE; j++) {
                DestBuffer[j] = Dem_PrimaryMemory[i].ExtendedData.Data[j];
            }
            *BufSize = DEM_EXTENDED_DATA_SIZE;
            return E_OK;
        }
    }
    return E_NOT_OK;
}
#endif

#if (DEM_PRESTORAGE_SUPPORT == STD_ON)
Std_ReturnType Dem_PrestoreFreezeFrame(Dem_EventIdType EventId)
{
    (void)EventId;
    return E_OK; /* placeholder */
}
Std_ReturnType Dem_ClearPrestoredFreezeFrame(Dem_EventIdType EventId)
{
    (void)EventId;
    return E_OK; /* placeholder */
}
#endif

#if (DEM_COMPONENT_SUPPORT == STD_ON)
Std_ReturnType Dem_GetComponentFailed(Dem_ComponentIdType ComponentId,
                                       boolean *ComponentFailed)
{
    (void)ComponentId;
    if (ComponentFailed != NULL_PTR) { *ComponentFailed = FALSE; }
    return E_OK;
}
Std_ReturnType Dem_SetComponentAvailable(Dem_ComponentIdType ComponentId,
                                          boolean AvailableStatus)
{
    (void)ComponentId;
    (void)AvailableStatus;
    return E_OK;
}
#endif

/* =========================================================================
 * §8.5  Dem_MainFunction — periodic task
 * ========================================================================= */

/**
 * @brief  Periodic DEM task (call every DEM_MAIN_PERIOD_MS ms).
 *         - Advances time-based debounce counters.
 *         - Increments the monotonic timestamp.
 */
void Dem_MainFunction(void)
{
    uint8 i;

    if (Dem_State != DEM_INITIALIZED) {
        return;
    }

    Dem_TimestampCounter++;

    /* Re-evaluate time-based debounce for events that had a recent input.
     * For simplicity each event with time-based debounce is ticked here.
     * Real implementations would only tick if a new status was received
     * within the current main-function cycle. */
    for (i = 0U; i < DEM_MAX_NUMBER_OF_EVENTS; i++) {
        Dem_EventIdType evId = (Dem_EventIdType)(i + 1U);
        if (Dem_EventCfg[i].DebounceType == DEM_DEBOUNCE_TIME) {
            if (Dem_EventRuntime[i].Debounce.TimeCounter > 0U) {
                Dem_EventStatusType dir =
                    (Dem_EventRuntime[i].Debounce.Direction == 1U)
                    ? DEM_EVENT_STATUS_PREFAILED
                    : DEM_EVENT_STATUS_PREPASSED;
                Dem_EventStatusType qual = Dem_Debounce_ProcessStatus(evId, dir);
                Dem_Internal_ProcessQualifiedStatus(evId, qual);
            }
        }
    }
}
