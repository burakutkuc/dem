/**
 * @file    Dem_Types.h
 * @brief   AUTOSAR CP R23-11 - Diagnostic Event Manager - Type Definitions (§8.2)
 */

#ifndef DEM_TYPES_H
#define DEM_TYPES_H

/* -------------------------------------------------------------------------
 * Platform / AUTOSAR base types (normally provided by Std_Types.h)
 * ------------------------------------------------------------------------- */
#ifndef PLATFORM_TYPES_H
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef signed char         sint8;
typedef signed short        sint16;
typedef signed int          sint32;
typedef unsigned char       boolean;
#define TRUE                (1U)
#define FALSE               (0U)
#define NULL_PTR            ((void*)0)
#endif

typedef uint8               Std_ReturnType;
#define E_OK                (0U)
#define E_NOT_OK            (1U)
#define STD_ON              (1U)
#define STD_OFF             (0U)

/* DEM-specific return codes */
#define DEM_PENDING         (2U)
#define DEM_CLEAR_BUSY      (3U)
#define DEM_CLEAR_MEMORY_ERROR (4U)
#define DEM_CLEAR_FAILED    (5U)
#define DEM_FILTERED_OK     (0U)
#define DEM_FILTERED_NO_MATCHING_DTC    (6U)
#define DEM_FILTERED_PENDING            (7U)
#define DEM_NO_SUCH_ELEMENT (8U)
#define DEM_BUFFER_TOO_SMALL (9U)
#define DEM_BUSY            (10U)

/* -------------------------------------------------------------------------
 * §8.2.1.1  Dem_ComponentIdType
 * ------------------------------------------------------------------------- */
typedef uint16  Dem_ComponentIdType;

/* -------------------------------------------------------------------------
 * §8.2.1.2  Dem_ConfigType
 * ------------------------------------------------------------------------- */
typedef struct Dem_ConfigType_tag  Dem_ConfigType;

/* -------------------------------------------------------------------------
 * §8.2.1.3  Dem_EventIdType
 * EventId is 1-based; 0 is DEM_EVENT_INVALID
 * ------------------------------------------------------------------------- */
typedef uint16  Dem_EventIdType;
#define DEM_EVENT_INVALID   (0U)

/* -------------------------------------------------------------------------
 * §8.2.1.4  Dem_EventStatusType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_EventStatusType;
#define DEM_EVENT_STATUS_PASSED       (0x00U)
#define DEM_EVENT_STATUS_FAILED       (0x01U)
#define DEM_EVENT_STATUS_PREPASSED    (0x02U)
#define DEM_EVENT_STATUS_PREFAILED    (0x03U)

/* -------------------------------------------------------------------------
 * §8.2.1.5  Dem_DebouncingStateType  (bitmask)
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DebouncingStateType;
#define DEM_DEBOUNCESTATUS_FREQLY_DETECTED    (0x01U)
#define DEM_DEBOUNCESTATUS_FINALLY_DEFECTIVE  (0x02U)
#define DEM_DEBOUNCESTATUS_TEMPORARILY_DEFECTIVE (0x04U)
#define DEM_DEBOUNCESTATUS_TEST_COMPLETE      (0x08U)

/* -------------------------------------------------------------------------
 * §8.2.1.6  Dem_DebounceResetStatusType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DebounceResetStatusType;
#define DEM_DEBOUNCE_STATUS_FREEZE    (0x00U)
#define DEM_DEBOUNCE_STATUS_RESET     (0x01U)

/* -------------------------------------------------------------------------
 * §8.2.1.7  Dem_UdsStatusByteType  (bitmask, §7.7.1)
 * Bit 0: TestFailed (TF)
 * Bit 1: TestFailedThisOperationCycle (TFTOC)
 * Bit 2: PendingDTC (PDTC)
 * Bit 3: ConfirmedDTC (CDTC)
 * Bit 4: TestNotCompletedSinceLastClear (TNCLC)
 * Bit 5: TestFailedSinceLastClear (TFSLC)
 * Bit 6: TestNotCompletedThisOperationCycle (TNCTOC)
 * Bit 7: WarningIndicatorRequested (WIR)
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_UdsStatusByteType;
#define DEM_UDS_STATUS_TF       (0x01U)
#define DEM_UDS_STATUS_TFTOC    (0x02U)
#define DEM_UDS_STATUS_PDTC     (0x04U)
#define DEM_UDS_STATUS_CDTC     (0x08U)
#define DEM_UDS_STATUS_TNCLC    (0x10U)
#define DEM_UDS_STATUS_TFSLC    (0x20U)
#define DEM_UDS_STATUS_TNCTOC   (0x40U)
#define DEM_UDS_STATUS_WIR      (0x80U)
#define DEM_UDS_STATUS_DEFAULT  (DEM_UDS_STATUS_TNCLC | DEM_UDS_STATUS_TNCTOC)

/* -------------------------------------------------------------------------
 * §8.2.1.8  Dem_IndicatorStatusType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_IndicatorStatusType;
#define DEM_INDICATOR_OFF           (0x00U)
#define DEM_INDICATOR_CONTINUOUS    (0x01U)
#define DEM_INDICATOR_BLINKING      (0x02U)
#define DEM_INDICATOR_BLINK_CONT    (0x03U)

/* -------------------------------------------------------------------------
 * §8.2.1.9  Dem_MonitorDataType
 * ------------------------------------------------------------------------- */
typedef uint32  Dem_MonitorDataType;
#define DEM_MONITOR_DATA_DEFAULT    (0x00000000UL)

/* -------------------------------------------------------------------------
 * §8.2.1.10 Dem_MonitorStatusType  (bitmask)
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_MonitorStatusType;
#define DEM_MONITOR_STATUS_TF           (0x01U)
#define DEM_MONITOR_STATUS_TNCTOC       (0x02U)

/* -------------------------------------------------------------------------
 * §8.2.1.11 Dem_DTCKindType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DTCKindType;
#define DEM_DTC_KIND_ALL_DTCS           (0x01U)
#define DEM_DTC_KIND_EMISSION_REL_DTCS  (0x02U)

/* -------------------------------------------------------------------------
 * §8.2.1.12 Dem_DTCFormatType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DTCFormatType;
#define DEM_DTC_FORMAT_OBD              (0x00U)
#define DEM_DTC_FORMAT_UDS              (0x01U)
#define DEM_DTC_FORMAT_J1939            (0x02U)

/* -------------------------------------------------------------------------
 * §8.2.1.13 Dem_DTCOriginType
 * ------------------------------------------------------------------------- */
typedef uint16  Dem_DTCOriginType;
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY   (0x0001U)
#define DEM_DTC_ORIGIN_MIRROR_MEMORY    (0x0002U)
#define DEM_DTC_ORIGIN_PERMANENT_MEMORY (0x0003U)
#define DEM_DTC_ORIGIN_SECONDARY_MEMORY (0x0004U)

/* -------------------------------------------------------------------------
 * §8.2.1.14 Dem_DTCRequestType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DTCRequestType;
#define DEM_FIRST_FAILED_DTC            (0x01U)
#define DEM_MOST_RECENT_FAILED_DTC      (0x02U)
#define DEM_FIRST_DET_CONFIRMED_DTC     (0x03U)
#define DEM_MOST_REC_DET_CONFIRMED_DTC  (0x04U)

/* -------------------------------------------------------------------------
 * §8.2.1.16 Dem_DTCSeverityType  (bitmask)
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_DTCSeverityType;
#define DEM_SEVERITY_NO_SEVERITY        (0x00U)
#define DEM_SEVERITY_MAINTENANCE_ONLY   (0x20U)
#define DEM_SEVERITY_CHECK_AT_NEXT_HALT (0x40U)
#define DEM_SEVERITY_CHECK_IMMEDIATELY  (0x80U)

/* -------------------------------------------------------------------------
 * §8.2.1.17 Dem_RatioIdType
 * ------------------------------------------------------------------------- */
typedef uint16  Dem_RatioIdType;

/* -------------------------------------------------------------------------
 * §8.2.1.19 Dem_InitMonitorReasonType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_InitMonitorReasonType;
#define DEM_INIT_MONITOR_CLEAR          (0x01U)
#define DEM_INIT_MONITOR_RESTART        (0x02U)
#define DEM_INIT_MONITOR_REENABLED      (0x03U)
#define DEM_INIT_MONITOR_STORAGE_REENABLED (0x04U)

/* -------------------------------------------------------------------------
 * §8.2.1.20 Dem_IumprDenomCondIdType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_IumprDenomCondIdType;

/* -------------------------------------------------------------------------
 * §8.2.1.21 Dem_IumprDenomCondStatusType
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_IumprDenomCondStatusType;
#define DEM_IUMPR_DEN_STATUS_NOT_REACHED (0x00U)
#define DEM_IUMPR_DEN_STATUS_REACHED     (0x01U)

/* -------------------------------------------------------------------------
 * DTC filter-related (used by DCM interface)
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_FilterForFDCType;
#define DEM_FILTER_FOR_FDC_YES          (0x01U)
#define DEM_FILTER_FOR_FDC_NO           (0x00U)

typedef uint8   Dem_FilterWithSeverityType;
#define DEM_FILTER_WITH_SEVERITY_YES    (0x01U)
#define DEM_FILTER_WITH_SEVERITY_NO     (0x00U)

/* -------------------------------------------------------------------------
 * Operation Cycle type
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_OperationCycleIdType;
typedef uint8   Dem_OperationCycleStateType;
#define DEM_CYCLE_STATE_START           (0x00U)
#define DEM_CYCLE_STATE_END             (0x01U)

/* -------------------------------------------------------------------------
 * Version info
 * ------------------------------------------------------------------------- */
typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

/* -------------------------------------------------------------------------
 * J1939 types
 * ------------------------------------------------------------------------- */
typedef uint8   Dem_J1939DcmDTCStatusFilterType;
#define DEM_J1939DTC_ACTIVE                     (0x00U)
#define DEM_J1939DTC_PREVIOUSLY_ACTIVE          (0x01U)
#define DEM_J1939DTC_PENDING                    (0x02U)
#define DEM_J1939DTC_PERMANENT                  (0x03U)
#define DEM_J1939DTC_CURRENTLY_ACTIVE           (0x04U)

typedef uint8   Dem_J1939DcmSetClearFilterType;
typedef uint8   Dem_J1939DcmSetFreezeFrameFilterType;

typedef struct {
    uint8 LampStatus;
    uint8 FlashLampStatus;
} Dem_J1939DcmLampStatusType;

typedef struct {
    uint8 OBDReadinessStatus[4];
} Dem_J1939DcmDiagnosticReadiness1Type;

typedef struct {
    uint8 OBDReadinessStatus[4];
} Dem_J1939DcmDiagnosticReadiness2Type;

typedef struct {
    uint8 OBDReadinessStatus[4];
} Dem_J1939DcmDiagnosticReadiness3Type;

typedef uint8   Dem_EventOBDReadinessGroupType;
typedef uint8   Dem_DTRControlType;
#define DEM_DTR_UPDATE_ALWAYS           (0x00U)
#define DEM_DTR_UPDATE_NEVER            (0x01U)

#endif /* DEM_TYPES_H */
