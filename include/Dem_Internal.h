/**
 * @file    Dem_Internal.h
 * @brief   AUTOSAR CP R23-11 - DEM internal data structures.
 *          NOT part of the public API. Include only from Dem source files.
 */

#ifndef DEM_INTERNAL_H
#define DEM_INTERNAL_H

#include "Dem_Types.h"
#include "Dem_Cfg.h"

/* =========================================================================
 * Module state
 * ========================================================================= */
typedef uint8 Dem_StateType;
#define DEM_UNINITIALIZED   (0U)
#define DEM_PREINIT         (1U)
#define DEM_INITIALIZED     (2U)

/* =========================================================================
 * Debounce runtime state (per event)
 * ========================================================================= */
typedef struct {
    sint8   Counter;        /* counter-based: current debounce counter       */
    uint16  TimeCounter;    /* time-based: elapsed ms in current direction   */
    uint8   Direction;      /* 0=pass, 1=fail; used by time-based            */
} Dem_DebounceRuntimeType;

/* =========================================================================
 * Freeze frame storage (per memory entry)
 * ========================================================================= */
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
typedef struct {
    uint8  Data[DEM_FREEZE_FRAME_SIZE];
    uint8  RecordNumber;
    uint8  Valid;           /* TRUE if data has been stored                  */
} Dem_FreezeFrameType;
#endif

/* =========================================================================
 * Extended data storage (per memory entry)
 * ========================================================================= */
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
typedef struct {
    uint8  Data[DEM_EXTENDED_DATA_SIZE];
    uint8  RecordNumber;
    uint8  Valid;
} Dem_ExtendedDataType;
#endif

/* =========================================================================
 * Primary memory entry
 * ========================================================================= */
typedef struct {
    Dem_EventIdType       EventId;           /* 0 = slot free                */
    Dem_UdsStatusByteType UdsStatus;
    uint8                 OccurrenceCounter;
    uint8                 AgingCounter;
    uint8                 FailedCycleCounter;
    uint8                 ConsecutiveFailed; /* counts towards ConfirmThresh  */
    uint32                Timestamp;         /* monotonic counter             */
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
    Dem_FreezeFrameType   FreezeFrame;
#endif
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
    Dem_ExtendedDataType  ExtendedData;
#endif
} Dem_MemoryEntryType;

/* =========================================================================
 * Per-event runtime state
 * ========================================================================= */
typedef struct {
    Dem_UdsStatusByteType UdsStatus;         /* current status byte           */
    Dem_MonitorStatusType MonitorStatus;
    Dem_DebounceRuntimeType Debounce;
    uint8                 Available;         /* TRUE = event is available     */
    uint8                 Suppressed;        /* TRUE = DTC suppressed         */
    uint8                 ConfirmCounter;    /* counts FAILEDs for CDTC       */
    uint8                 HealingCounter;    /* counts PASSEDs for WIR clear  */
} Dem_EventRuntimeType;

/* =========================================================================
 * DCM client state (filter + selection)
 * ========================================================================= */
typedef struct {
    /* DTC selection */
    uint32              SelectedDTC;
    Dem_DTCFormatType   SelectedFormat;
    Dem_DTCOriginType   SelectedOrigin;
    uint8               SelectionValid;
    uint8               ClearPending;

    /* DTC filter (for GetNextFilteredDTC) */
    uint8               FilterActive;
    Dem_UdsStatusByteType FilterStatusMask;
    Dem_DTCKindType     FilterDTCKind;
    Dem_DTCFormatType   FilterFormat;
    Dem_DTCOriginType   FilterOrigin;
    Dem_FilterWithSeverityType FilterWithSeverity;
    Dem_DTCSeverityType FilterSeverity;
    Dem_FilterForFDCType FilterForFDC;
    uint8               FilterIndex;        /* current iterator position      */

    /* Freeze frame filter (for GetNextFilteredRecord) */
    uint8               FFFilterActive;
    uint8               FFFilterRecord;
    uint8               FFFilterIndex;

    /* Record update lock */
    uint8               RecordUpdateDisabled;
} Dem_ClientStateType;

/* =========================================================================
 * NvM-persistent data block
 * ========================================================================= */
typedef struct {
    Dem_MemoryEntryType   PrimaryMemory[DEM_MAX_EVENT_MEMORY_ENTRIES];
    Dem_UdsStatusByteType EventUdsStatus[DEM_MAX_NUMBER_OF_EVENTS];
    sint8                 DebounceCounters[DEM_MAX_NUMBER_OF_EVENTS];
    uint32                Timestamp;        /* monotonic write counter         */
    uint16                Crc;             /* simple XOR CRC for validity     */
} Dem_NvDataType;

/* =========================================================================
 * Global variables (declared here, defined in Dem.c)
 * ========================================================================= */
extern Dem_StateType          Dem_State;
extern Dem_EventRuntimeType   Dem_EventRuntime[DEM_MAX_NUMBER_OF_EVENTS];
extern Dem_MemoryEntryType    Dem_PrimaryMemory[DEM_MAX_EVENT_MEMORY_ENTRIES];
extern Dem_ClientStateType    Dem_ClientState;
extern Dem_NvDataType         Dem_NvData;
extern uint8                  Dem_EnableConditions[DEM_MAX_ENABLE_CONDITIONS];
extern uint8                  Dem_StorageConditions[DEM_MAX_STORAGE_CONDITIONS];
extern uint8                  Dem_DTCSettingDisabled;
extern uint32                 Dem_TimestampCounter;

/* =========================================================================
 * Internal helper prototypes (used across source files)
 * ========================================================================= */
uint8   Dem_Internal_EnableConditionsMet(Dem_EventIdType EventId);
uint8   Dem_Internal_StorageConditionsMet(Dem_EventIdType EventId);
void    Dem_Internal_UpdateUdsStatusFailed(Dem_EventIdType EventId);
void    Dem_Internal_UpdateUdsStatusPassed(Dem_EventIdType EventId);
void    Dem_Internal_UpdateUdsStatusConfirmed(Dem_EventIdType EventId);
void    Dem_Internal_OperationCycleEnd(void);
uint16  Dem_Internal_CalcCrc(const uint8 *data, uint16 length);

#endif /* DEM_INTERNAL_H */
