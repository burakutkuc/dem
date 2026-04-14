/**
 * @file    Dem_Cfg.h
 * @brief   AUTOSAR CP R23-11 - DEM compile-time configuration.
 *          15 events, feature flags, operation cycles, conditions.
 *
 *  To enable / disable a feature change the matching flag below:
 *      STD_ON  = feature compiled in
 *      STD_OFF = feature excluded from build
 */

#ifndef DEM_CFG_H
#define DEM_CFG_H

#include "Dem_Types.h"

/* =========================================================================
 * Module version
 * ========================================================================= */
#define DEM_VENDOR_ID           (0x0001U)
#define DEM_MODULE_ID           (0x0013U)
#define DEM_SW_MAJOR_VERSION    (1U)
#define DEM_SW_MINOR_VERSION    (0U)
#define DEM_SW_PATCH_VERSION    (0U)

/* =========================================================================
 * Memory sizes
 * ========================================================================= */
#define DEM_MAX_NUMBER_OF_EVENTS        (15U)
#define DEM_MAX_EVENT_MEMORY_ENTRIES    (10U)
#define DEM_FREEZE_FRAME_SIZE           (16U)   /* bytes per entry            */
#define DEM_EXTENDED_DATA_SIZE          (8U)    /* bytes per entry            */
#define DEM_MAX_ENABLE_CONDITIONS       (4U)
#define DEM_MAX_STORAGE_CONDITIONS      (4U)
#define DEM_NVM_BLOCK_ID                (5U)

/* =========================================================================
 * Feature flags  (STD_ON / STD_OFF)
 * ========================================================================= */
#define DEM_OBD_SUPPORT                 STD_OFF
#define DEM_J1939_SUPPORT               STD_OFF
#define DEM_FREEZE_FRAME_SUPPORT        STD_ON
#define DEM_EXTENDED_DATA_SUPPORT       STD_ON
#define DEM_COMPONENT_SUPPORT           STD_OFF
#define DEM_SUPPRESSION_SUPPORT         STD_ON
#define DEM_WIR_SUPPORT                 STD_ON
#define DEM_PRESTORAGE_SUPPORT          STD_OFF
#define DEM_ENABLE_CONDITION_SUPPORT    STD_ON
#define DEM_STORAGE_CONDITION_SUPPORT   STD_ON
#define DEM_IUMPR_SUPPORT               STD_OFF
#define DEM_DTR_SUPPORT                 STD_OFF
#define DEM_MULTIPLE_CLIENTS            STD_OFF
#define DEM_CLEAR_ALLOWED_CB            STD_OFF
#define DEM_DEV_ERROR_DETECT            STD_ON

/* =========================================================================
 * Critical section macros (bare-metal IRQ-safe integration)
 *
 * Override these macros in your project build (e.g. via compiler defines or
 * a wrapper header) to disable/restore interrupts around critical sections.
 * Default is no-op.
 * ========================================================================= */
#ifndef DEM_ENTER_CRITICAL
#define DEM_ENTER_CRITICAL()  do { } while (0)
#endif
#ifndef DEM_EXIT_CRITICAL
#define DEM_EXIT_CRITICAL()   do { } while (0)
#endif

/* =========================================================================
 * Operation cycle IDs
 * ========================================================================= */
#define DEM_OPCYCLE_IGNITION            (0U)
#define DEM_OPCYCLE_OBD_DCY             (1U)
#define DEM_NUM_OPERATION_CYCLES        (2U)

/* =========================================================================
 * Enable condition IDs  (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
 * ========================================================================= */
#define DEM_ENABLECOND_VOLTAGE_OK       (0U)
#define DEM_ENABLECOND_COMM_OK          (1U)
#define DEM_ENABLECOND_TEMP_VALID       (2U)
#define DEM_ENABLECOND_ALWAYS           (3U)

/* =========================================================================
 * Storage condition IDs  (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
 * ========================================================================= */
#define DEM_STORAGECOND_ENGINE_RUNNING  (0U)
#define DEM_STORAGECOND_SPEED_VALID     (1U)
#define DEM_STORAGECOND_INIT_DONE       (2U)
#define DEM_STORAGECOND_ALWAYS          (3U)

/* =========================================================================
 * DTC group
 * ========================================================================= */
#define DEM_DTC_GROUP_ALL_DTCS          (0x00FFFFFFuL)
#define DEM_DTC_GROUP_EMISSION_REL      (0x00FFFF33uL)

/* =========================================================================
 * Event ID symbolic names (1-based index into Dem_EventCfg[])
 * ========================================================================= */
#define DEM_EVENT_VOLTAGE_LOW           (1U)
#define DEM_EVENT_VOLTAGE_HIGH          (2U)
#define DEM_EVENT_TEMP_SENSOR_1         (3U)
#define DEM_EVENT_TEMP_SENSOR_2         (4U)
#define DEM_EVENT_SPEED_SENSOR          (5U)
#define DEM_EVENT_PRESSURE_SENSOR       (6U)
#define DEM_EVENT_COMM_TIMEOUT          (7U)
#define DEM_EVENT_CHECKSUM_ERROR        (8U)
#define DEM_EVENT_ACTUATOR_1            (9U)
#define DEM_EVENT_ACTUATOR_2            (10U)
#define DEM_EVENT_ACTUATOR_3            (11U)
#define DEM_EVENT_NVM_ERROR             (12U)
#define DEM_EVENT_WATCHDOG              (13U)
#define DEM_EVENT_OVERCURRENT           (14U)
#define DEM_EVENT_UNDERVOLTAGE_ECU      (15U)

/* =========================================================================
 * Debounce type enumeration (stored in Dem_EventCfgType)
 * ========================================================================= */
typedef uint8 Dem_DebounceTypeType;
#define DEM_DEBOUNCE_COUNTER            (0U)
#define DEM_DEBOUNCE_TIME               (1U)
#define DEM_DEBOUNCE_MONITOR_INTERNAL   (2U)

/* =========================================================================
 * Event configuration structure
 * ========================================================================= */
typedef struct {
    uint32               Dtc;                 /* 3-byte UDS DTC value          */
    uint8                Priority;            /* 1 = highest, 255 = lowest     */
    Dem_DebounceTypeType DebounceType;
    sint8                CounterThreshFail;   /* counter-based: FAILED thresh  */
    sint8                CounterThreshPass;   /* counter-based: PASSED thresh  */
    uint16               TimeThreshFailMs;    /* time-based: ms to FAILED      */
    uint16               TimeThreshPassMs;    /* time-based: ms to PASSED      */
    uint8                ConfirmThreshold;    /* # consecutive FAILEDs → CDTC  */
    uint8                AgingThreshold;      /* # passed cycles → age out     */
    uint8                EnableCondMask;      /* bitmask of enable conditions  */
    uint8                StorageCondMask;     /* bitmask of storage conditions */
} Dem_EventCfgType;

/* =========================================================================
 * Event configuration table (defined in Dem_Cfg.c)
 * EventId = array_index + 1
 * ========================================================================= */
extern const Dem_EventCfgType Dem_EventCfg[DEM_MAX_NUMBER_OF_EVENTS];

#endif /* DEM_CFG_H */
